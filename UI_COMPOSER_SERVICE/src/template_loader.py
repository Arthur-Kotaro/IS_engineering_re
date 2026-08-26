import yaml
import os
from typing import Dict, Any
from src.models import Template, SourceConfig

class TemplateLoader:
    def __init__(self, templates_dir: str = "templates"):
        self.templates_dir = templates_dir
        self._cache: Dict[str, Template] = {}
    
    def load(self, name: str) -> Template:
        if name in self._cache:
            return self._cache[name]
        
        path = os.path.join(self.templates_dir, f"{name}.yaml")
        with open(path, 'r') as f:
            data = yaml.safe_load(f)
        
        sources = [SourceConfig(**s) for s in data.get('sources', [])]
        template = Template(
            name=data['name'],
            endpoint=data['endpoint'],
            title=data['title'],
            description=data.get('description'),
            sources=sources,
            ui=data.get('ui', {})
        )
        self._cache[name] = template
        return template
    
    def reload(self, name: str):
        if name in self._cache:
            del self._cache[name]
        return self.load(name)
