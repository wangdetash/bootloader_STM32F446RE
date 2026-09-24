# Provisional bootloader protocol

These choices are temporary and must match the host implementation. They were
chosen for this implementation, not taken from the command-reference PDF.

## UART roles

- `C_UART = &huart2`: binary host requests and responses, 115200 baud, 8-N-1.
- `D_UART = &huart3`: diagnostic text through `printmsg()`, 115200 baud, 8-N-1.

## Request format

`[length: 1 byte][command: 1 byte][arguments: optional][CRC: 4 bytes]`

`length` counts every byte after itself, including the CRC. Total request size
is 6 to 256 bytes. GET_VER requires exactly six bytes, with no arguments.

CRC uses the STM32F446 hardware CRC peripheral via `HAL_CRC_Accumulate`. It covers
the length, command, and arguments, excluding the four CRC bytes. Parameters:

- Polynomial: `0x04C11DB7`.
- Initial value: `0xFFFFFFFF`; reset the peripheral before each packet.
- No input/output reflection and no final XOR.
- Zero-extend each packet byte into a separate 32-bit word and accumulate
  that word MSB first (32 bits per packet byte). Do not pack four bytes
  into one word or cast the byte buffer to a word buffer.
- The received CRC is serialized little-endian.

Protocol constants and the provisional version `BL_VERSION = 0x10` are in
`Core/Inc/main.h`; the hardware-backed verifier is in `Core/Src/main.c`.
`bootloader_verify_crc(uint8_t *p_data, uint8_t len, uint32_t crc_host)`
accepts the data buffer, its length excluding the four CRC bytes, and the
decoded host CRC. It initializes `uwCRCvalue` to `0xFF`, then overwrites it
with each hardware result and returns `VERIFY_CRC_SUCCESS` (0) on a match
or `VERIFY_CRC_FAILURE` (1) otherwise. Null or empty input fails verification.
The local variable's initial value does not change the hardware seed of
`0xFFFFFFFF`. A packet helper validates framing and extracts the host CRC
before calling the verifier; the maximum covered length is 252 bytes.
This replaces the previous software CRC-32/ISO-HDLC contract: `zlib.crc32`
and the old GET_VER packet `05 51 d8 87 c2 20` are no longer compatible.

## Responses

- Valid GET_VER: `[0xA5][0x01][0x10]` (ACK, reply size, version).
- Invalid CRC or malformed recognized request: `[0x7F]` (NACK only).
- Valid CRC for other recognized commands: `[0xA5][0x00]`, plus a diagnostic
  saying that the handler is not implemented. ACK confirms receipt and CRC,
  not successful command execution. No version is returned by these handlers.
- Unknown command: existing invalid-command diagnostic on D_UART, no binary
  response. Zero-length input is skipped before command dispatch.

All 12 recognized commands reach a handler. Enable and disable protection share
`bootloader_handle_endis_rw_protect()`. Every handler verifies the CRC before ACK.
GET_VER then calls `get_bootloader_version()`, formats the version for
`printmsg()`, and sends the version byte with `bootloader_uart_write_data()`.

## Host example and hardware checks

```python
def bootloader_crc(data):
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= byte  # One zero-extended uint32_t per byte.
        for _ in range(32):
            crc = ((crc << 1) ^ (0x04C11DB7 if crc & 0x80000000 else 0)) & 0xFFFFFFFF
    return crc

request_without_crc = bytes([5, 0x51])
request = request_without_crc + bootloader_crc(request_without_crc).to_bytes(4, 'little')
assert request.hex(' ') == '05 51 e7 e9 ab 7c'
```

Send those six raw bytes on C_UART. Expected binary reply: `a5 01 10`.
Expected D_UART text: `Bootloader version: 0x10\r\n`.
Changing the last request byte to `7d` should produce NACK only.
A GET_VER request with extra arguments and a correctly recomputed CRC should
also produce NACK. Test minimum/maximum packet lengths and back-to-back packets
on hardware before relying on this transport.

Hardware behavior has not been verified. Receives and writes still use
`HAL_MAX_DELAY`; incomplete requests can block indefinitely. Other command
operations and command-specific argument checks remain unimplemented.
