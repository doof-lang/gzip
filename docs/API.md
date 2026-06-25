# std/gzip Guide

`std/gzip` compresses and decompresses gzip container payloads. Use the
one-shot helpers when the whole payload is already in memory, and use
`GzipStream` when you already have data as a `Stream<readonly byte[]>` and want
to compress it incrementally.

This module works with bytes only. Convert text with `std/blob` or another text
encoding helper before compression, and decode after decompression.

## Quick Start

```doof
import { GzipStream, gunzip, gzip } from "std/gzip"

compressed := gzip(bytes)
original := try! gunzip(compressed)

for chunk of GzipStream(sourceChunks) {
  // write each compressed chunk to a file, socket, or another stream sink
}
```

## One-Shot Compression

`gzip(data)` returns a complete gzip container, including the standard header,
compressed deflate body, and trailer. It is the simplest option for request
bodies, small files, test fixtures, and cache entries that already fit in
memory.

`gunzip(data)` expects a complete gzip payload. Invalid, truncated, or malformed
input returns `Failure<string>` instead of panicking.

## Streaming Compression

`GzipStream` implements `Stream<readonly byte[]>`. It pulls chunks from the
source stream, feeds them into the native gzip encoder, skips empty native
outputs, and emits a final trailer chunk when the source is exhausted.

The stream is compression-only. There is currently no streaming gunzip decoder;
use `gunzip` when you need to decompress a complete payload.

## API

### `gzip`

```doof
export import function gzip(data: readonly byte[]): readonly byte[]
```

Compress a complete byte array and return a complete gzip payload.

Defined in [index.do](../index.do).

### `gunzip`

```doof
export import function gunzip(data: readonly byte[]): Result<readonly byte[], string>
```

Decompress a complete gzip payload. Returns `Failure` for invalid or truncated
input.

Defined in [index.do](../index.do).

### `GzipStream`

```doof
export class GzipStream implements Stream<readonly byte[]>
```

Incrementally compress chunks from another byte stream. Construct with
`GzipStream(source)` and consume it with normal stream iteration.

Methods:

- `next(): bool` advances to the next non-empty compressed chunk.
- `value(): readonly byte[]` returns the current compressed chunk.

Defined in [index.do](../index.do).
