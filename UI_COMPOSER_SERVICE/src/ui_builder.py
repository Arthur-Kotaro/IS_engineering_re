import re
import logging
from typing import Dict, Any, List
from src.models import Template

logger = logging.getLogger(__name__)

class UIBuilder:
    def __init__(self):
        self._placeholder_pattern = re.compile(r'\{\{\s*([^}]+)\s*\}\}')
    
    def build(self, template: Template, data: Dict[str, Any]) -> Dict[str, Any]:
        logger.info(f"Building UI with data keys: {list(data.keys())}")
        ui = template.ui.copy()
        
        if 'widgets' in ui:
            ui['widgets'] = self._render_widgets(ui['widgets'], data)
        
        return {
            "title": template.title,
            "widgets": ui.get('widgets', [])
        }
    
    def _render_widgets(self, widgets: List[Dict], data: Dict[str, Any]) -> List[Dict]:
        result = []
        for widget in widgets:
            rendered = self._render_widget(widget, data)
            if rendered:
                result.append(rendered)
        return result
    
    def _render_widget(self, widget: Dict, data: Dict[str, Any]) -> Dict:
        if 'visible' in widget:
            visible = self._resolve_placeholder(widget['visible'], data)
            if not visible:
                return None
        
        result = widget.copy()
        
        if 'data' in result and 'rows' in result['data']:
            rows = self._resolve_placeholder(result['data']['rows'], data)
            result['data']['rows'] = rows
        
        if 'fields' in result.get('data', {}):
            fields = result['data']['fields']
            for field in fields:
                if 'value' in field:
                    logger.info(f"Resolving field: {field['value']}")
                    resolved = self._resolve_placeholder(field['value'], data)
                    logger.info(f"Resolved to: {resolved}")
                    field['value'] = resolved
        
        if 'widgets' in result:
            result['widgets'] = self._render_widgets(result['widgets'], data)
        
        return result
    
    def _resolve_placeholder(self, value: Any, data: Dict[str, Any]) -> Any:
        if isinstance(value, str):
            matches = self._placeholder_pattern.findall(value)
            for match in matches:
                parts = match.strip().split('.')
                current = data
                for part in parts:
                    if isinstance(current, dict):
                        current = current.get(part)
                    else:
                        current = None
                        break
                if current is not None:
                    value = value.replace(f'{{{{{match}}}}}', str(current))
                    logger.info(f"Replaced {match} with {current}")
            return value
        elif isinstance(value, list):
            return [self._resolve_placeholder(item, data) for item in value]
        elif isinstance(value, dict):
            return {k: self._resolve_placeholder(v, data) for k, v in value.items()}
        return value
