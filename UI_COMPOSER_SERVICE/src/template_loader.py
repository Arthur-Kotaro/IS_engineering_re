# src/template_loader.py
import yaml
import os
import logging
from typing import Dict, List
from src.models import Template, SourceConfig
from src.config import settings

logger = logging.getLogger(__name__)


class TemplateLoader:
    def __init__(self, templates_dir: str = None):
        self.templates_dir = templates_dir or settings.TEMPLATES_DIR
        self._cache: Dict[str, Template] = {}

    def load(self, name: str) -> Template:
        if name in self._cache:
            return self._cache[name]

        path = os.path.join(self.templates_dir, f"{name}.yaml")
        if not os.path.exists(path):
            raise FileNotFoundError(f"Template not found: {path}")

        with open(path, "r") as f:
            data = yaml.safe_load(f)

        sources = [SourceConfig(**s) for s in data.get("sources", [])]
        template = Template(
            name=data["name"],
            endpoint=data["endpoint"],
            title=data["title"],
            description=data.get("description"),
            sources=sources,
            ui=data.get("ui", {}),
        )
        self._cache[name] = template
        return template

    def reload_all(self):
        self._cache.clear()

    def list_templates(self) -> List[str]:
        result = []
        for root, _, files in os.walk(self.templates_dir):
            for f in files:
                if f.endswith(".yaml"):
                    rel = os.path.relpath(os.path.join(root, f), self.templates_dir)
                    result.append(rel[:-5])
        return sorted(result)
