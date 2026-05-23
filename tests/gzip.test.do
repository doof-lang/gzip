import { gzip, GzipStream } from "../index"
import { BlobBuilder } from "std/blob"

function assertBytes(actual: readonly byte[], expected: readonly byte[]): void {
  assert(actual.length == expected.length, "expected byte lengths to match")

  for index of 0..<actual.length {
    assert(actual[index] == expected[index], "expected bytes to match")
  }
}

function buildPayload(): readonly byte[] {
  builder := BlobBuilder()
  builder.writeString("hello gzip\n")
  builder.writeString("hello gzip\n")
  builder.writeString("hello gzip\n")
  return builder.build()
}

function collect(stream: Stream<readonly byte[]>): readonly byte[] {
  builder := BlobBuilder()
  for chunk of stream {
    builder.writeBytes(chunk)
  }
  return builder.build()
}

function readLittleEndianInt(data: readonly byte[], start: int): long {
  return long(data[start]) |
    (long(data[start + 1]) << 8) |
    (long(data[start + 2]) << 16) |
    (long(data[start + 3]) << 24)
}

export function testGzipProducesGzipContainer(): void {
  input := buildPayload()
  compressed := gzip(input)

  assert(compressed.length > 18, "expected gzip payload to contain header, body, and trailer")
  assert(compressed[0] == 31, "expected gzip magic byte 1")
  assert(compressed[1] == 139, "expected gzip magic byte 2")
  assert(compressed[2] == 8, "expected deflate compression method")
  assert(compressed[3] == 0, "expected no optional gzip flags")

  inputSize := readLittleEndianInt(compressed, compressed.length - 4)
  assert(inputSize == long(input.length), "expected gzip trailer to include the original input size")
}

export function testGzipStreamMatchesOneShotGzip(): void {
  input := buildPayload()
  streamed := collect(GzipStream(input, 5))

  assertBytes(streamed, gzip(input))
}

export function testGzipHandlesEmptyInput(): void {
  input: readonly byte[] := []
  compressed := gzip(input)
  streamed := collect(GzipStream(input, 3))

  assertBytes(streamed, compressed)
  assert(compressed.length > 0, "expected empty input to still produce a gzip container")
}
