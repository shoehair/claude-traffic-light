# Claude Code traffic light

An ESP32 traffic light that shows what Claude Code is doing:

- 🟡 **Yellow** – working
- 🔴 **Red** – waiting on you (permission prompt or a question)
- 🟢 **Green** – done
- ⚫ **Off** – session ended

Claude Code [hooks](https://docs.claude.com/en/docs/claude-code/hooks) call `light.sh`, which sends a request over WiFi to the ESP32 at `http://claude-light.local/set?state=<color>`.

## Parts

- ESP32-S3 dev board (ESP32-S3-DevKitC-1 style, e.g. N16R8)
- Open-Smart RYG traffic light module (R, Y, G, GND pins)
- 4 female-to-female jumper wires

## Wiring

| Light | ESP32   |
|-------|---------|
| R     | 11      |
| Y     | 12      |
| G     | 13      |
| GND   | GND     |

All four are on the same side of the board, which is labeled 3V3, RST, 4, 5, 6…, and GND is at the bottom of that side.
If you're using an original ESP32 instead of an S3, change the pins at the top of the `.ino` file (for example to 25, 26, 27) and use `--fqbn esp32:esp32:esp32`.

## Flashing

1. Install [arduino-cli](https://arduino.github.io/arduino-cli/) and the ESP32 core:
   ```bash
   arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
   arduino-cli core update-index && arduino-cli core install esp32:esp32
   ```
2. Copy `firmware/claude_light/secrets.h.example` to `secrets.h` in the same folder and fill in your WiFi details. The ESP32 needs a 2.4 GHz network.
3. Plug in the board, find its port with `arduino-cli board list`, then from the `firmware` folder run:
   ```bash
   arduino-cli compile --upload -p /dev/cu.usbserial-XXXX --fqbn esp32:esp32:esp32s3 claude_light
   ```
   If the upload stalls at "Connecting…", hold the BOOT button.

The lights cycle while the board connects to WiFi, then stay solid green once it's connected.

## Hooking up Claude Code

1. Clone this repo to `~/dev/claude-traffic-light`. If you put it somewhere else, update the paths in `hooks.json`.
2. Merge the `hooks` block from `hooks.json` into `~/.claude/settings.json`.
3. Test it with `./light.sh red`.

If `claude-light.local` doesn't resolve on your network, set `CLAUDE_LIGHT_HOST` to the board's IP address. You can see the IP in the serial monitor at 115200 baud.

If several sessions are open, the light shows whatever happened most recently in any of them.
