# src/main.py
import os
import logging
import yaml
from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException, Header, Request, Query
from typing import Optional, Dict, Any

from src.template_loader import TemplateLoader
from src.workflow_loader import WorkflowLoader
from src.data_aggregator import DataAggregator
from src.ui_builder import UIBuilder
from src.session_service import SessionService
from src.config import settings

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

template_loader = TemplateLoader()
workflow_loader = WorkflowLoader()
data_aggregator = None
ui_builder = UIBuilder()
session_service = SessionService()

with open(settings.SERVICES_CONFIG_PATH) as f:
    services = yaml.safe_load(f).get("services", {})


@asynccontextmanager
async def lifespan(app: FastAPI):
    global data_aggregator
    data_aggregator = DataAggregator(services)
    workflow_loader.load_all()
    yield
    await data_aggregator.close()


app = FastAPI(title="UI Composer Service", version="2.0.0", lifespan=lifespan)


def _get_user_id(request: Request) -> str:
    uid = request.headers.get("X-User-ID")
    if not uid:
        raise HTTPException(401, "Missing X-User-ID")
    return uid


def _get_token(request: Request) -> Optional[str]:
    auth = request.headers.get("Authorization")
    if not auth:
        return None
    return auth.removeprefix("Bearer ").strip()


def _get_impersonated_by(request: Request) -> Optional[str]:
    return request.headers.get("X-Impersonated-By")


@app.get("/health")
async def health():
    return {"status": "ok", "redis": await session_service.ping()}


@app.get("/pages")
async def list_pages():
    return {"pages": template_loader.list_templates()}


@app.get("/page/{full_path:path}")
async def get_page(full_path: str, request: Request):
    user_id = _get_user_id(request)
    token = _get_token(request)
    impersonated_by = _get_impersonated_by(request)
    query_params = dict(request.query_params)

    endpoint = f"/page/{full_path}"
    workflow = workflow_loader.find_by_endpoint(endpoint)

    if not workflow:
        # Fallback: рендер шаблона без workflow
        try:
            template = template_loader.load(full_path)
        except FileNotFoundError:
            raise HTTPException(404, f"Page '{full_path}' not found")

        data = await data_aggregator.fetch_all(
            template.sources, user_id, token, query_params, impersonated_by, None
        )
        ui = ui_builder.build(template, data)
        return ui

    session = await session_service.create(
        workflow_name=workflow.name,
        initial_step=workflow.initial_step,
        user_id=int(user_id),
    )

    step = workflow.steps[session.current_step]
    template = template_loader.load(step.template)

    data = await data_aggregator.fetch_all(
        template.sources, user_id, token, query_params, impersonated_by, session.data
    )
    ui = ui_builder.build(template, data)

    return {
        "title": step.title or template.title,
        "widgets": ui["widgets"],
        "context": {
            "session_id": session.session_id,
            "current_step": session.current_step,
            "workflow_name": workflow.name,
        },
    }


@app.post("/workflow/event")
async def workflow_event(request: Request):
    user_id = _get_user_id(request)
    token = _get_token(request)
    impersonated_by = _get_impersonated_by(request)

    body = await request.json()
    session_id = body.get("session_id")
    widget_id = body.get("widget_id")
    event_type = body.get("event_type", "click")
    event_data = body.get("data", {}) or {}

    if not session_id:
        raise HTTPException(400, "session_id required")

    session = await session_service.get(session_id)
    if not session:
        raise HTTPException(404, "Session not found or expired")

    workflow = workflow_loader.get(session.workflow_name)
    if not workflow:
        raise HTTPException(404, "Workflow not found")

    step = workflow.steps.get(session.current_step)
    if not step:
        raise HTTPException(404, "Step not found")

    handler = None
    for ev in step.on_event:
        if ev.widget == widget_id and ev.event_type == event_type:
            handler = ev
            break

    if not handler:
        raise HTTPException(400, f"No handler for widget={widget_id} event={event_type}")

    service_result = None
    if handler.service:
        svc = handler.service
        endpoint = svc.get("endpoint", "")
        if "{{" in endpoint:
            for k, v in (session.data or {}).items():
                endpoint = endpoint.replace("{{" + k + "}}", str(v))
            for k, v in event_data.items():
                endpoint = endpoint.replace("{{" + k + "}}", str(v))
        service_name = svc.get("name")
        service_url = services.get(service_name, {}).get("url")
        if not service_url:
            raise HTTPException(500, f"Service {service_name} not configured")

        method = svc.get("method", "GET")
        params = {}
        for k, v in (svc.get("params") or {}).items():
            if isinstance(v, str) and v.startswith("{{") and v.endswith("}}"):
                key = v[2:-2].strip()
                if key in event_data:
                    params[k] = event_data[key]
                elif key in (session.data or {}):
                    params[k] = session.data[key]
            else:
                params[k] = v

        headers = {"X-User-ID": user_id}
        if impersonated_by:
            headers["X-Impersonated-By"] = impersonated_by
        if token:
            headers["Authorization"] = f"Bearer {token}"

        import httpx
        async with httpx.AsyncClient(timeout=10.0) as client:
            url = f"{service_url}{endpoint}"
            try:
                if method.upper() == "GET":
                    resp = await client.get(url, headers=headers, params=params or None)
                else:
                    resp = await client.request(method.upper(), url, headers=headers, json=params or None)
                if resp.status_code < 400:
                    service_result = resp.json()
                else:
                    raise HTTPException(resp.status_code, f"Service error: {resp.text[:200]}")
            except httpx.RequestError as e:
                raise HTTPException(502, f"Service unavailable: {e}")

    on_success = handler.on_success or {}
    next_step = on_success.get("next_step")
    data_updates = on_success.get("data") or {}

    if service_result is not None:
        for key, value in data_updates.items():
            if isinstance(value, str) and value == "{{result}}":
                session.data[key] = service_result
            elif isinstance(value, str) and value.startswith("{{") and value.endswith("}}"):
                session.data[key] = event_data.get(value[2:-2].strip())
            else:
                session.data[key] = value

    for k, v in event_data.items():
        if k not in (session.data or {}):
            session.data[k] = v

    if next_step:
        session.history.append(session.current_step)
        session.current_step = next_step

    await session_service.save(session)

    if not next_step:
        return {
            "title": step.title or "",
            "widgets": [],
            "context": {
                "session_id": session.session_id,
                "current_step": session.current_step,
                "workflow_name": session.workflow_name,
                "result": service_result,
            },
        }

    next_step_obj = workflow.steps[session.current_step]
    template = template_loader.load(next_step_obj.template)

    data = await data_aggregator.fetch_all(
        template.sources, user_id, token, None, impersonated_by, session.data
    )
    ui = ui_builder.build(template, data)

    return {
        "title": next_step_obj.title or template.title,
        "widgets": ui["widgets"],
        "context": {
            "session_id": session.session_id,
            "current_step": session.current_step,
            "workflow_name": session.workflow_name,
        },
    }


@app.post("/workflow/back")
async def workflow_back(request: Request):
    user_id = _get_user_id(request)
    token = _get_token(request)
    impersonated_by = _get_impersonated_by(request)

    body = await request.json()
    session_id = body.get("session_id")
    if not session_id:
        raise HTTPException(400, "session_id required")

    session = await session_service.get(session_id)
    if not session or not session.history:
        raise HTTPException(404, "No previous step")

    session.current_step = session.history.pop()
    await session_service.save(session)

    workflow = workflow_loader.get(session.workflow_name)
    step = workflow.steps[session.current_step]
    template = template_loader.load(step.template)

    data = await data_aggregator.fetch_all(
        template.sources, user_id, token, None, impersonated_by, session.data
    )
    ui = ui_builder.build(template, data)

    return {
        "title": step.title or template.title,
        "widgets": ui["widgets"],
        "context": {
            "session_id": session.session_id,
            "current_step": session.current_step,
            "workflow_name": session.workflow_name,
        },
    }


@app.post("/workflow/close")
async def workflow_close(request: Request):
    body = await request.json()
    session_id = body.get("session_id")
    if session_id:
        await session_service.close(session_id)
    return {"message": "Session closed"}
