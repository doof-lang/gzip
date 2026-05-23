#pragma once

#include "doof_runtime.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <zlib.h>

namespace doof_gzip {

inline void panicZlibFailure(const char* operation, int code, const z_stream& stream) {
    std::string message = std::string("gzip ") + operation + " failed";
    if (stream.msg != nullptr) {
        message += ": ";
        message += stream.msg;
    } else {
        message += " with zlib code ";
        message += std::to_string(code);
    }
    doof::panic(message);
}

inline int normalizeBlockSize(int32_t blockSize) {
    return blockSize > 0 ? blockSize : 65536;
}

class NativeGzipStream {
public:
    static std::shared_ptr<NativeGzipStream> constructor(
        const std::shared_ptr<std::vector<uint8_t>>& data,
        int32_t blockSize
    ) {
        return std::shared_ptr<NativeGzipStream>(new NativeGzipStream(data, normalizeBlockSize(blockSize)));
    }

    ~NativeGzipStream() {
        if (initialized_) {
            deflateEnd(&stream_);
        }
    }

    std::shared_ptr<std::vector<uint8_t>> next() {
        if (done_) {
            return nullptr;
        }

        auto output = std::make_shared<std::vector<uint8_t>>(static_cast<size_t>(blockSize_));
        stream_.next_out = output->data();
        stream_.avail_out = static_cast<uInt>(output->size());

        while (stream_.avail_out > 0) {
            if (!inputLoaded_) {
                stream_.next_in = const_cast<Bytef*>(inputData());
                stream_.avail_in = static_cast<uInt>(inputSize());
                inputLoaded_ = true;
            }

            const int result = deflate(&stream_, Z_FINISH);
            if (result == Z_STREAM_END) {
                done_ = true;
                break;
            }
            if (result != Z_OK) {
                panicZlibFailure("deflate", result, stream_);
            }
            if (stream_.avail_out == 0) {
                break;
            }
            if (stream_.avail_in == 0 && result == Z_OK) {
                continue;
            }
        }

        output->resize(output->size() - static_cast<size_t>(stream_.avail_out));
        if (output->empty()) {
            return nullptr;
        }
        return output;
    }

private:
    NativeGzipStream(const std::shared_ptr<std::vector<uint8_t>>& data, int blockSize)
        : data_(data ? data : std::make_shared<std::vector<uint8_t>>()), blockSize_(blockSize) {
        std::memset(&stream_, 0, sizeof(stream_));
        const int result = deflateInit2(
            &stream_,
            Z_DEFAULT_COMPRESSION,
            Z_DEFLATED,
            MAX_WBITS + 16,
            8,
            Z_DEFAULT_STRATEGY
        );
        if (result != Z_OK) {
            panicZlibFailure("initialize", result, stream_);
        }
        initialized_ = true;
    }

    Bytef* inputData() const {
        if (data_->empty()) {
            return nullptr;
        }
        return const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data_->data()));
    }

    size_t inputSize() const {
        const size_t maxChunk = static_cast<size_t>(std::numeric_limits<uInt>::max());
        return std::min(data_->size(), maxChunk);
    }

    std::shared_ptr<std::vector<uint8_t>> data_;
    int blockSize_;
    z_stream stream_;
    bool initialized_ = false;
    bool inputLoaded_ = false;
    bool done_ = false;
};

class NativeGzipEncoder {
public:
    static std::shared_ptr<NativeGzipEncoder> create() {
        return std::shared_ptr<NativeGzipEncoder>(new NativeGzipEncoder());
    }

    ~NativeGzipEncoder() {
        if (initialized_) {
            deflateEnd(&stream_);
        }
    }

    std::shared_ptr<std::vector<uint8_t>> update(const std::shared_ptr<std::vector<uint8_t>>& data) {
        if (finished_) {
            doof::panic("gzip encoder update after finish");
        }

        const auto safeData = data ? data : std::make_shared<std::vector<uint8_t>>();
        if (safeData->empty()) {
            return std::make_shared<std::vector<uint8_t>>();
        }

        stream_.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(safeData->data()));
        stream_.avail_in = static_cast<uInt>(std::min(
            safeData->size(),
            static_cast<size_t>(std::numeric_limits<uInt>::max())
        ));
        return drain(Z_NO_FLUSH);
    }

    std::shared_ptr<std::vector<uint8_t>> finish() {
        if (finished_) {
            return std::make_shared<std::vector<uint8_t>>();
        }

        finished_ = true;
        stream_.next_in = nullptr;
        stream_.avail_in = 0;
        return drain(Z_FINISH);
    }

private:
    NativeGzipEncoder() {
        std::memset(&stream_, 0, sizeof(stream_));
        const int result = deflateInit2(
            &stream_,
            Z_DEFAULT_COMPRESSION,
            Z_DEFLATED,
            MAX_WBITS + 16,
            8,
            Z_DEFAULT_STRATEGY
        );
        if (result != Z_OK) {
            panicZlibFailure("initialize", result, stream_);
        }
        initialized_ = true;
    }

    std::shared_ptr<std::vector<uint8_t>> drain(int flush) {
        auto output = std::make_shared<std::vector<uint8_t>>();
        std::array<uint8_t, 65536> buffer {};

        while (true) {
            stream_.next_out = buffer.data();
            stream_.avail_out = static_cast<uInt>(buffer.size());

            const int result = deflate(&stream_, flush);
            if (result == Z_STREAM_END) {
                const size_t produced = buffer.size() - static_cast<size_t>(stream_.avail_out);
                output->insert(output->end(), buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(produced));
                break;
            }
            if (result != Z_OK) {
                panicZlibFailure("deflate", result, stream_);
            }

            const size_t produced = buffer.size() - static_cast<size_t>(stream_.avail_out);
            output->insert(output->end(), buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(produced));

            if (stream_.avail_in == 0 && stream_.avail_out > 0) {
                break;
            }
        }

        return output;
    }

    z_stream stream_;
    bool initialized_ = false;
    bool finished_ = false;
};

inline std::shared_ptr<std::vector<uint8_t>> gzip(const std::shared_ptr<std::vector<uint8_t>>& data) {
    auto stream = NativeGzipStream::constructor(data, 65536);
    auto output = std::make_shared<std::vector<uint8_t>>();

    while (true) {
        auto chunk = stream->next();
        if (!chunk) {
            return output;
        }
        output->insert(output->end(), chunk->begin(), chunk->end());
    }
}

}  // namespace doof_gzip
