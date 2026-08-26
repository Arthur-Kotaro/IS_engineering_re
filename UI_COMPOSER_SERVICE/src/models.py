from dataclasses import dataclass, field
from typing import Dict, List, Any, Optional

@dataclass
class SourceConfig:
    id: str
    service: str
    endpoint: str
    method: str = "GET"
    params: Optional[Dict[str, str]] = None

@dataclass
class Template:
    name: str
    endpoint: str
    title: str
    sources: List[SourceConfig]
    ui: Dict[str, Any]
    description: Optional[str] = None

@dataclass
class PageResponse:
    title: str
    widgets: List[Dict[str, Any]]
