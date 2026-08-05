# std/gzip

Gzip compression and decompression utilities for byte arrays and byte streams.
This package owns the shared zlib native dependency and also exports raw
deflate/inflate and CRC-32 primitives for packages such as `std/archive`.

## Documentation

- [Guide and API reference](docs/API.md) covers one-shot compression, streaming compression, invalid input, and exported helpers.
- Tests can be run with `doof test gzip`.

## Usage

```doof
import { GzipStream, gunzip, gzip } from "std/gzip"

compressed := gzip(bytes)
original := try! gunzip(compressed)

compressedChunks := GzipStream(chunks)
```

## Exports

### `gzip(data: readonly byte[]): readonly byte[]`

Compress a byte array and return the complete gzip payload as a byte array.

### `gunzip(data: readonly byte[]): Result<readonly byte[], string>`

Decompress a complete gzip payload, returning a failure for invalid or
truncated input.

### `GzipStream(source: Stream<readonly byte[]>): Stream<readonly byte[]>`

Return a stream that incrementally compresses chunks from another byte stream.

### `deflateRaw(data: readonly byte[]): readonly byte[]`

Compress a byte array as raw deflate without a container.

### `inflateRaw(data: readonly byte[]): Result<readonly byte[], string>`

Decompress raw deflate data.

### `crc32(data: readonly byte[]): long`

Calculate the standard CRC-32 checksum used by ZIP and gzip.
