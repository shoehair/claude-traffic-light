#!/bin/sh
# Usage: light.sh red|yellow|green|off
# Fire-and-forget so a missing or offline light never slows Claude down.
HOST="${CLAUDE_LIGHT_HOST:-claude-light.local}"
curl -s -m 2 "http://$HOST/set?state=$1" >/dev/null 2>&1 &
exit 0
