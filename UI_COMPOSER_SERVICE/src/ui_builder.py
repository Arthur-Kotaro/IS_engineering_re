# src/ui_builder.py
import re
import copy
import logging
from typing import Dict, Any, List
from src.models import Template

logger = logging.getLogger(__name__)


class UIBuilder:
    def __init__(self):
        self._placeholder_pattern = re.compile(r"\{\{\s*([^}]+)\s*\}\}")

    def build(self, template: Template, data: Dict[str, Any]) -> Dict[str, Any]:
        ui = copy.deepcopy(template.ui)
        widgets = ui.get("widgets", [])
        rendered = self._render_widgets(widgets, data)
        return {
            "title": template.title,
            "widgets": rendered,
        }

    def _render_widgets(self, widgets: List[Dict], data: Dict[str, Any]) -> List[Dict]:
        result = []
        for widget in widgets:
            rendered = self._render_widget(widget, data)
            if rendered:
                result.append(rendered)
        return result

    def _render_widget(self, widget: Dict, data: Dict[str, Any]) -> Dict:
        w = copy.deepcopy(widget)

        if "visible" in w:
            visible = self._resolve_placeholder(w["visible"], data)
            if not self._is_truthy(visible):
                return None

        # Обработка источников для полей Form/Select/MultiSelect
        if w.get("type") == "Form" and "fields" in w and isinstance(w["fields"], list):
            for field in w["fields"]:
                src_name = field.get("source")
                if src_name and isinstance(data, dict) and src_name in data:
                    field["source_data"] = data[src_name]

        if "data" in w and isinstance(w["data"], dict):
            if "rows" in w["data"]:
                w["data"]["rows"] = self._resolve_placeholder(w["data"]["rows"], data)
            if "source" in w["data"]:
                source_key = w["data"]["source"]
                if isinstance(source_key, str) and source_key.startswith("{{") and source_key.endswith("}}"):
                    w["data"]["rows"] = self._resolve_placeholder(source_key, data)
            if "fields" in w["data"] and isinstance(w["data"]["fields"], list):
                for field in w["data"]["fields"]:
                    if "value" in field:
                        field["value"] = self._resolve_placeholder(field["value"], data)

        if "widgets" in w and isinstance(w["widgets"], list):
            w["widgets"] = self._render_widgets(w["widgets"], data)

        return w

    def _resolve_placeholder(self, value: Any, data: Dict[str, Any]) -> Any:
        if isinstance(value, str):
            return self._resolve_string(value, data)
        if isinstance(value, list):
            return [self._resolve_placeholder(x, data) for x in value]
        if isinstance(value, dict):
            return {k: self._resolve_placeholder(v, data) for k, v in value.items()}
        return value

    def _resolve_string(self, value: str, data: Dict[str, Any]) -> Any:
        matches = self._placeholder_pattern.findall(value)
        if not matches:
            return value

        if len(matches) == 1 and value.strip() == "{{" + matches[0] + "}}":
            return self._lookup(matches[0].strip(), data)

        def repl(m):
            key = m.group(1).strip()
            resolved = self._lookup(key, data)
            return "" if resolved is None else str(resolved)

        return self._placeholder_pattern.sub(repl, value)

    def _lookup(self, path: str, data: Dict[str, Any]) -> Any:
        parts = path.split(".")
        current = data
        for part in parts:
            if isinstance(current, dict):
                current = current.get(part)
            elif isinstance(current, list):
                try:
                    idx = int(part)
                    current = current[idx] if 0 <= idx < len(current) else None
                except (ValueError, IndexError):
                    current = None
            else:
                return None
            if current is None:
                return None
        return current

    def _is_truthy(self, value: Any) -> bool:
        if value is None:
            return False
        if isinstance(value, bool):
            return value
        if isinstance(value, str):
            return value.lower() not in ("", "false", "0", "none", "null")
        if isinstance(value, (int, float)):
            return value != 0
        if isinstance(value, (list, dict)):
            return len(value) > 0
        return bool(value)
