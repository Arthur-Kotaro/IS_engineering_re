# src/models.py
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
class WorkflowEvent:
    widget: str
    event_type: str
    service: Optional[Dict[str, Any]] = None
    validation: Optional[List[Dict[str, Any]]] = None
    on_success: Optional[Dict[str, Any]] = None


@dataclass
class WorkflowStep:
    name: str
    type: str
    template: str
    title: Optional[str] = None
    on_event: List[WorkflowEvent] = field(default_factory=list)


@dataclass
class Workflow:
    name: str
    initial_endpoint: str
    initial_step: str
    steps: Dict[str, WorkflowStep]


@dataclass
class WorkflowSession:
    session_id: str
    workflow_name: str
    current_step: str
    user_id: int
    data: Dict[str, Any] = field(default_factory=dict)
    history: List[str] = field(default_factory=list)
