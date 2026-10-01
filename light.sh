#!/bin/sh
# Usage: light.sh red|yellow|green|off
# Sends a color over USB serial. A missing board is intentionally a no-op.

case "${1:-}" in
  red|yellow|green|off) STATE="$1" ;;
  *)
    printf 'usage: %s red|yellow|green|off\n' "$0" >&2
    exit 2
    ;;
esac

PORT="${TRAFFIC_LIGHT_PORT:-${CODEX_LIGHT_PORT:-${CLAUDE_LIGHT_PORT:-}}}"
if [ -z "$PORT" ]; then
  for CANDIDATE in /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial*; do
    if [ -c "$CANDIDATE" ]; then
      PORT="$CANDIDATE"
      break
    fi
  done
fi

[ -n "$PORT" ] || exit 0
[ -c "$PORT" ] || exit 0

# macOS resets port settings when the port closes, so set them on the open
# handle. -hupcl stops macOS from toggling reset and rebooting the board.
(
  exec 3<>"$PORT"
  stty 115200 -hupcl clocal raw -echo <&3
  printf '%s\n' "$STATE" >&3
) >/dev/null 2>&1

exit 0
