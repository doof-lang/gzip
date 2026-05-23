import class NativeGzipStream from "native_gzip.hpp" as doof_gzip::NativeGzipStream {
  static constructor(data: readonly byte[], blockSize: int): NativeGzipStream
  next(): readonly byte[] | null
}

import class NativeGzipEncoder from "native_gzip.hpp" as doof_gzip::NativeGzipEncoder {
  static create(): NativeGzipEncoder
  update(data: readonly byte[]): readonly byte[]
  finish(): readonly byte[]
}

export import function gzip(data: readonly byte[]): readonly byte[] from "native_gzip.hpp" as doof_gzip::gzip

function normalizeBlockSize(blockSize: int): int {
  if blockSize > 0 {
    return blockSize
  }

  return 65536
}

export class GzipStream implements Stream<readonly byte[]> {
  source: Stream<readonly byte[]>
  private native: NativeGzipEncoder
  private currentValue: readonly byte[] = []
  private sourceDone: bool = false
  private finished: bool = false

  static constructor(source: Stream<readonly byte[]>): GzipStream {
    return GzipStream {
      source,
      native: NativeGzipEncoder.create(),
    }
  }

  next(): bool {
    while true {
      if !this.sourceDone {
        if this.source.next() {
          compressed := this.native.update(this.source.value())
          if compressed.length > 0 {
            this.currentValue = compressed
            return true
          }
          continue
        }
        this.sourceDone = true
      }

      if this.finished {
        return false
      }

      this.finished = true
      finalChunk := this.native.finish()
      if finalChunk.length == 0 {
        return false
      }
      this.currentValue = finalChunk
      return true
    }
  }

  value(): readonly byte[] => this.currentValue
}
