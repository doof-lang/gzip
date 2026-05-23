# std/gzip

Gzip compression utilities for byte arrays and byte streams.

## Usage

```doof
import { GzipStream, gzip } from "std/gzip"

compressed := gzip(bytes)

compressedChunks := GzipStream(chunks)
```

## Exports

### `gzip(data: readonly byte[]): readonly byte[]`

Compress a byte array and return the complete gzip payload as a byte array.

### `GzipStream(source: Stream<readonly byte[]>): Stream<readonly byte[]>`

Return a stream that incrementally compresses chunks from another byte stream.
