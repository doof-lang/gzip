import class NativeGzipStream from "native_gzip.hpp" as doof_gzip::NativeGzipStream {
  static constructor(data: readonly byte[], blockSize: int): NativeGzipStream
  next(): readonly byte[] | null
}

export import function gzip(data: readonly byte[]): readonly byte[] from "native_gzip.hpp" as doof_gzip::gzip

function normalizeBlockSize(blockSize: int): int {
  if blockSize > 0 {
    return blockSize
  }

  return 65536
}

export class GzipStream implements Stream<readonly byte[]> {
  private native: NativeGzipStream
  private currentValue: readonly byte[] = []

  static constructor(data: readonly byte[], blockSize: int = 65536): GzipStream {
    return GzipStream {
      native: NativeGzipStream(data, normalizeBlockSize(blockSize)),
    }
  }

  next(): bool {
    chunk := this.native.next()
    if chunk == null {
      return false
    }

    this.currentValue = chunk!
    return true
  }

  value(): readonly byte[] => this.currentValue
}
