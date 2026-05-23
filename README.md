# std/gzip

Gzip compression utilities for byte arrays.

## Usage

```doof
import { GzipStream, gzip, gzipStream } from "std/gzip"

compressed := gzip(bytes)

for chunk of gzipStream(bytes, 16384) {
  writeChunk(chunk)
}

stream := GzipStream.create(bytes)
```

## Exports

### `gzip(data: readonly byte[]): readonly byte[]`

Compress a byte array and return the complete gzip payload as a byte array.

### `gzipStream(data: readonly byte[], blockSize: int = 65536): Stream<readonly byte[]>`

Return a stream that emits the gzip-compressed payload in byte-array blocks. `blockSize` controls the maximum size of each emitted block; non-positive values use the default block size.

### `GzipStream`

A `Stream<readonly byte[]>` implementation used by `gzipStream`.

```doof
stream := GzipStream.create(bytes, 16384)
```
