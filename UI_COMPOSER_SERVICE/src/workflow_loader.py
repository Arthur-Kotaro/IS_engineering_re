# src/workflow_loader.py
import os
import yaml
import logging
from typing import Dict, Optional
from src.models import Workflow, WorkflowStep, WorkflowEvent
from src.config import settings

logger = logging.getLogger(__name__)


class WorkflowValidationError(Exception):
    pass


class WorkflowLoader:
    def __init__(self, workflows_dir: Optional[str] = None):
        self.workflows_dir = workflows_dir or settings.WORKFLOWS_DIR
        self.by_endpoint: Dict[str, Workflow] = {}
        self.by_name: Dict[str, Workflow] = {}
        self._loaded = False

    def load_all(self):
        self.by_endpoint.clear()
        self.by_name.clear()

        if not os.path.isdir(self.workflows_dir):
            logger.warning(f"Workflows dir not found: {self.workflows_dir}")
            self._loaded = True
            return

        for root, _, files in os.walk(self.workflows_dir):
            for f in files:
                if not f.endswith(".yaml"):
                    continue
                path = os.path.join(root, f)
                try:
                    wf = self._load_file(path)
                    self._validate(wf, path)
                    self.by_endpoint[wf.initial_endpoint] = wf
                    self.by_name[wf.name] = wf
                    logger.info(f"Loaded workflow: {wf.name} ({wf.initial_endpoint})")
                except Exception as e:
                    logger.error(f"Failed to load workflow {path}: {e}")

        self._loaded = True

    def _load_file(self, path: str) -> Workflow:
        with open(path, "r") as f:
            data = yaml.safe_load(f)

        steps = {}
        for step_name, step_data in (data.get("steps") or {}).items():
            events = []
            for ev in (step_data.get("on_event") or []):
                events.append(WorkflowEvent(
                    widget=ev.get("widget"),
                    event_type=ev.get("event_type", "click"),
                    service=ev.get("service"),
                    validation=ev.get("validation"),
                    on_success=ev.get("on_success"),
                ))
            steps[step_name] = WorkflowStep(
                name=step_name,
                type=step_data.get("type", "navigation"),
                template=step_data.get("template"),
                title=step_data.get("title"),
                on_event=events,
            )

        return Workflow(
            name=data["name"],
            initial_endpoint=data["initial_endpoint"],
            initial_step=data["initial_step"],
            steps=steps,
        )

    def _validate(self, wf: Workflow, path: str):
        if not wf.name:
            raise WorkflowValidationError(f"{path}: missing name")
        if not wf.initial_endpoint:
            raise WorkflowValidationError(f"{path}: missing initial_endpoint")
        if wf.initial_step not in wf.steps:
            raise WorkflowValidationError(f"{path}: initial_step '{wf.initial_step}' not in steps")

        for step_name, step in wf.steps.items():
            if not step.template:
                raise WorkflowValidationError(f"{path}: step '{step_name}' has no template")
            for ev in step.on_event:
                if not ev.widget:
                    raise WorkflowValidationError(f"{path}: step '{step_name}' has event without widget")
                if ev.on_success and ev.on_success.get("next_step"):
                    ns = ev.on_success["next_step"]
                    if ns not in wf.steps:
                        raise WorkflowValidationError(
                            f"{path}: step '{step_name}' next_step '{ns}' not found"
                        )

    def find_by_endpoint(self, endpoint: str) -> Optional[Workflow]:
        return self.by_endpoint.get(endpoint)

    def get(self, name: str) -> Optional[Workflow]:
        return self.by_name.get(name)
