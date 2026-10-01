#!/usr/bin/env python3
"""Translate Codex and Claude Code lifecycle events into light states."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent


def state_for(payload: dict[str, object]) -> str | None:
    event = payload.get("hook_event_name")
    if event == "SessionStart":
        return "green"
    if event == "UserPromptSubmit":
        return "yellow"
    if event == "PermissionRequest":
        return "red"
    if event == "Notification" and payload.get("notification_type") in {
        "permission_prompt",
        "elicitation_dialog",
        "elicitation_url_dialog",
        "agent_needs_input",
    }:
        return "red"
    if event == "Elicitation":
        return "red"
    if event == "ElicitationResult":
        return "yellow"
    if event == "PreToolUse" and payload.get("tool_name") in {
        "request_user_input",
        "AskUserQuestion",
    }:
        return "red"
    if event == "PostToolUse":
        return "yellow"
    if event == "Stop":
        return "green"
    if event in {"Interrupt", "PostToolUseFailure", "StopFailure"}:
        return "red"
    if event == "SessionEnd":
        return "off"
    return None


def main() -> int:
    try:
        payload = json.load(sys.stdin)
        if not isinstance(payload, dict):
            return 0
        state = state_for(payload)
        if state:
            subprocess.run(
                [str(ROOT / "light.sh"), state],
                check=False,
                stdin=subprocess.DEVNULL,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=2,
            )
    except (json.JSONDecodeError, OSError, subprocess.SubprocessError):
        # A light that is absent or offline must never interfere with Codex.
        pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
