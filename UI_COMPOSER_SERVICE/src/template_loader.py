import yaml
import os
import logging
from typing import Dict, Any
from src.models import Template, SourceConfig

logger = logging.getLogger(__name__)

class TemplateLoader:
    def __init__(self, templates_dir: str = "templates"):
        self.templates_dir = templates_dir
        self._cache: Dict[str, Template] = {}
        logger.info(f"TemplateLoader initialized with dir: {os.path.abspath(templates_dir)}")
    
    def load(self, name: str) -> Template:
        if name in self._cache:
            logger.info(f"Loading from cache: {name}")
            return self._cache[name]
        
        path = os.path.join(self.templates_dir, f"{name}.yaml")
        abs_path = os.path.abspath(path)
        logger.info(f"Loading template from: {abs_path}")
        
        if not os.path.exists(abs_path):
            logger.error(f"Template file not found: {abs_path}")
            raise FileNotFoundError(f"Template file not found: {abs_path}")
        
        with open(abs_path, 'r') as f:
            data = yaml.safe_load(f)
        
        logger.info(f"Template data loaded: {list(data.keys())}")
        
        sources = []
        for s in data.get('sources', []):
            sources.append(SourceConfig(**s))
        
        template = Template(
            name=data['name'],
            endpoint=data['endpoint'],
            title=data['title'],
            description=data.get('description'),
            sources=sources,
            ui=data.get('ui', {})
        )
        self._cache[name] = template
        logger.info(f"Template loaded: {name}, sources: {len(sources)}")
        return template
    
    def reload_all(self):
        self._cache.clear()
    
    def list_templates(self):
        templates = []
        for f in os.listdir(self.templates_dir):
            if f.endswith(".yaml"):
                templates.append(f.replace(".yaml", ""))
        return templates
