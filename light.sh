#!/bin/sh
# Usage: light.sh red|yellow|green|off
# Sends over USB if the board is plugged in, otherwise over WiFi.
# Fire-and-forget so a missing or offline light never slows Claude down.
PORT="${CLAUDE_LIGHT_PORT:-$(ls /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial* 2>/dev/null | head -1)}"
if [ -n "$PORT" ]; then
  # macOS resets port settings when the port closes, so set them on the open handle.
  # -hupcl stops macOS from toggling the reset line, which would reboot the board.
  (
    exec 3<>"$PORT"
    stty 115200 -hupcl clocal raw -echo <&3
    printf '%s\n' "$1" >&3
  ) >/dev/null 2>&1 &
else
  HOST="${CLAUDE_LIGHT_HOST:-claude-light.local}"
  curl -s -m 2 "http://$HOST/set?state=$1" >/dev/null 2>&1 &
fi
exit 0
