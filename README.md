# Claude Code + Codex traffic light

An ESP32 traffic light that shows the state of local Claude Code and Codex
chats:

- 🟡 **Yellow** – the agent is working
- 🔴 **Red** – the agent needs attention (approval, input, interruption, or error)
- 🟢 **Green** – the turn is complete
- ⚫ **Off** – the session ended

This is a USB-only adaptation of
[shoehair/claude-traffic-light](https://github.com/shoehair/claude-traffic-light).
It supports both [Claude Code hooks](https://code.claude.com/docs/en/hooks) and
[Codex hooks](https://learn.chatgpt.com/docs/hooks). Wi-Fi, credentials, the web
server, and mDNS have been removed.

## Parts and wiring

- ESP32-S3 dev board (ESP32-S3-DevKitC-1 style, for example N16R8)
- Open-Smart RYG traffic light module (R, Y, G, and GND pins)
- Four female-to-female jumper wires

| Light | ESP32 |
| --- | --- |
| R | 11 |
| Y | 12 |
| G | 13 |
| GND | GND |

All four are on the side of the board labeled `3V3`, `RST`, `4`, `5`, `6`, …,
with `GND` at the bottom. For an original ESP32 rather than an S3, change the
pins at the top of the `.ino` file (for example, 25, 26, and 27) and use the
matching fully qualified board name when compiling.

## Flash the board

Install `arduino-cli` and the ESP32 core if needed:

```sh
brew install arduino-cli
arduino-cli config add board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

Plug in the board, identify its port, and compile/upload:

```sh
arduino-cli board list
arduino-cli compile --upload \
  -p /dev/cu.usbserial-XXXX \
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc \
  firmware/codex_light
```

If the upload stalls at `Connecting…`, hold the board's **BOOT** button. On
boot, the light flashes red → yellow → green and then stays green.

## Install both integrations

Run both idempotent installers from this repository:

```sh
python3 scripts/install-user-hooks.py
python3 scripts/install-claude-hooks.py
```

The Codex installer merges handlers into `~/.codex/hooks.json`. The Claude
installer merges handlers into `~/.claude/settings.json`. Existing unrelated
settings and hooks are preserved, and each installer creates a backup before
updating an existing file:

- `~/.codex/hooks.json.bak.codex-traffic-light`
- `~/.claude/settings.json.bak.traffic-light`

Restart both clients after installing. In Codex, run `/hooks` and review/trust
the new non-managed hooks. In Claude Code, run `/hooks` to inspect the loaded
configuration.

The repository is also a valid Codex plugin. Its portable hook definition is
`hooks/hooks.json`; `hooks/claude-hooks.json` is the equivalent Claude plugin
hook definition.

## Test and configure

Set each color manually:

```sh
./light.sh red
./light.sh yellow
./light.sh green
./light.sh off
```

The script uses the first matching `/dev/cu.usbmodem*`,
`/dev/cu.usbserial*`, or `/dev/cu.wchusbserial*` device. If more than one USB
serial device is attached, set the exact port before starting either client:

```sh
export TRAFFIC_LIGHT_PORT=/dev/cu.usbmodem101
```

`CODEX_LIGHT_PORT` and `CLAUDE_LIGHT_PORT` remain supported as fallbacks. A
missing board is a silent no-op and never blocks either client. With several
active chats, the light shows the most recently received lifecycle event.

## State mapping

| Event | Light |
| --- | --- |
| Session starts/resumes | Green |
| User submits a prompt | Yellow |
| Permission or structured-input request | Red |
| Agent resumes after structured input | Yellow |
| Turn completes | Green |
| Tool failure or interruption | Red |
| Session ends | Off |
