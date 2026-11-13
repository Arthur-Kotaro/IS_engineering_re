# app/services/typology_loader.py
import json
import logging
from pathlib import Path
from typing import Optional

logger = logging.getLogger(__name__)

# Файлы типологий лежат в корне MG_service
_TYPOLOGY_FILE = Path(__file__).parent.parent.parent / "typology.json"
_POWER_UNIT_TYPOLOGY_FILE = Path(__file__).parent.parent.parent / "power_unit_typology.json"


def _load_file(path: Path) -> list[dict]:
    """Читает файл, содержащий несколько JSON-объектов подряд (как typology.json)."""
    if not path.exists():
        logger.warning(f"Typology file not found: {path}")
        return []
    text = path.read_text(encoding="utf-8")
    decoder = json.JSONDecoder()
    idx = 0
    items = []
    n = len(text)
    while idx < n:
        # пропустить пробелы/переводы строк
        while idx < n and text[idx] in " \t\r\n":
            idx += 1
        if idx >= n:
            break
        obj, end = decoder.raw_decode(text, idx)
        items.append(obj)
        idx = end
    return items


def _find_typology(items: list[dict], name: str) -> Optional[dict]:
    for item in items:
        if item.get("typology_name") == name:
            return item
    return None


def load_typology(project_type: str, typology_name: str) -> Optional[dict]:
    """
    Возвращает typology dict:
      {
        "typology_name": "...",
        "milestones": [ {m_name, m_id, m_week, abbreviation, tooltip}, ... ]
      }
    Для project_type == "power_unit" — читает power_unit_typology.json.
    Для project_type == "vehicle" — читает typology.json.
    """
    if project_type == "power_unit":
        items = _load_file(_POWER_UNIT_TYPOLOGY_FILE)
        # в power_unit_typology.json typology_name = "child"
        return _find_typology(items, typology_name) or _find_typology(items, "child")

    items = _load_file(_TYPOLOGY_FILE)
    return _find_typology(items, typology_name)


def build_milestones(typology: dict) -> list[dict]:
    """
    Преобразует milestones из typology dict в список для вставки в mg_milestones.
    Возвращает: [{abbreviation, full_name, tooltip, week_offset, sort_order}, ...]
    """
    milestones = typology.get("milestones", [])
    result = []
    for idx, m in enumerate(milestones):
        result.append({
            "abbreviation": m.get("abbreviation") or m.get("m_name") or "",
            "full_name": m.get("m_name") or "",
            "tooltip": m.get("tooltip") or None,
            "week_offset": int(m.get("m_week", 0)),
            "sort_order": idx,
        })
    return result
