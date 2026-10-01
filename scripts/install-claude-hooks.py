#!/usr/bin/env python3
"""Install the traffic-light hooks in the current user's Claude settings."""

from __future__ import annotations

import json
import os
import shutil
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
CLAUDE_HOME = Path(os.environ.get("CLAUDE_CONFIG_DIR", Path.home() / ".claude")).expanduser()
DESTINATION = CLAUDE_HOME / "settings.json"
MARKERS = (
    "traffic-light-hook.py",
    "codex-light-hook.py",
    "/claude-traffic-light/light.sh",
    "/codex-traffic-light/light.sh",
)


def handler(*, async_: bool = True, timeout: int = 3) -> dict[str, object]:
    command = f'python3 "{ROOT / "scripts" / "traffic-light-hook.py"}"'
    result: dict[str, object] = {
        "type": "command",
        "command": command,
        "timeout": timeout,
    }
    if async_:
        result["async"] = True
    return result


def group(matcher: str | None = None, *, async_: bool = True) -> dict[str, object]:
    result: dict[str, object] = {"hooks": [handler(async_=async_)]}
    if matcher is not None:
        result["matcher"] = matcher
    return result


TRAFFIC_LIGHT_HOOKS: dict[str, list[dict[str, object]]] = {
    "SessionStart": [group("startup|resume|clear|compact|fork")],
    "UserPromptSubmit": [group()],
    "PreToolUse": [group("AskUserQuestion")],
    "PermissionRequest": [group()],
    "Notification": [
        group("permission_prompt|elicitation_dialog|elicitation_url_dialog|agent_needs_input")
    ],
    "Elicitation": [group()],
    "ElicitationResult": [group()],
    "PostToolUse": [group()],
    "PostToolUseFailure": [group()],
    "Stop": [group()],
    "StopFailure": [group()],
    "SessionEnd": [group(async_=False)],
}


def without_traffic_light_handlers(item: object) -> object | None:
    """Remove only this project's handlers, preserving others in the group."""
    if not isinstance(item, dict) or not isinstance(item.get("hooks"), list):
        return item
    remaining = [
        hook
        for hook in item["hooks"]
        if not (
            isinstance(hook, dict)
            and any(marker in str(hook.get("command", "")) for marker in MARKERS)
        )
    ]
    if not remaining:
        return None
    return {**item, "hooks": remaining}


def load_existing() -> dict[str, object]:
    if not DESTINATION.exists():
        return {"$schema": "https://json.schemastore.org/claude-code-settings.json", "hooks": {}}
    with DESTINATION.open(encoding="utf-8") as handle:
        payload = json.load(handle)
    if not isinstance(payload, dict):
        raise ValueError(f"{DESTINATION} must contain a JSON object")
    if not isinstance(payload.get("hooks", {}), dict):
        raise ValueError(f"{DESTINATION} field 'hooks' must be an object")
    return payload


def main() -> int:
    payload = load_existing()
    hooks = payload.setdefault("hooks", {})
    assert isinstance(hooks, dict)

    for event, additions in TRAFFIC_LIGHT_HOOKS.items():
        current = hooks.get(event, [])
        if not isinstance(current, list):
            raise ValueError(f"{DESTINATION} hooks.{event} must be an array")
        preserved = [without_traffic_light_handlers(item) for item in current]
        hooks[event] = [item for item in preserved if item is not None] + additions

    DESTINATION.parent.mkdir(parents=True, exist_ok=True)
    if DESTINATION.exists():
        shutil.copy2(DESTINATION, DESTINATION.with_suffix(".json.bak.traffic-light"))

    fd, temporary = tempfile.mkstemp(prefix="settings.", suffix=".json", dir=DESTINATION.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            json.dump(payload, handle, indent=2)
            handle.write("\n")
        os.replace(temporary, DESTINATION)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)

    print(f"Installed Claude traffic-light hooks in {DESTINATION}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
