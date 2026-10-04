# src/data_aggregator.py
import httpx
import asyncio
import os
import logging
from typing import Dict, Any, List, Optional
from urllib.parse import urlencode
from src.models import SourceConfig

logger = logging.getLogger(__name__)


class DataAggregator:
    def __init__(self, services_config: Dict[str, Any]):
        self.services = services_config
        self.client = httpx.AsyncClient(timeout=10.0)
        self.internal_key = os.getenv("INTERNAL_API_KEY", "")

    async def fetch(
        self,
        source: SourceConfig,
        user_id: str,
        token: Optional[str] = None,
        query_params: Optional[Dict[str, str]] = None,
        impersonated_by: Optional[str] = None,
        session_data: Optional[Dict[str, Any]] = None,
    ) -> Any:
        if source.endpoint.startswith("session:"):
            key = source.endpoint[len("session:"):]
            if session_data and key in session_data:
                return session_data[key]
            return None

        service_config = self.services.get(source.service)
        if not service_config:
            return {"error": f"Service {source.service} not found"}
        service_url = service_config.get("url")
        if not service_url:
            return {"error": f"Service {source.service} has no url"}

        endpoint = source.endpoint.replace("__me__", str(user_id))
        url = f"{service_url}{endpoint}"

        headers = {"X-User-ID": str(user_id)}
        if impersonated_by:
            headers["X-Impersonated-By"] = str(impersonated_by)

        if source.endpoint.startswith("/internal"):
            if self.internal_key:
                headers["X-Internal-Key"] = self.internal_key
        elif token:
            headers["Authorization"] = f"Bearer {token}"

        params: Dict[str, Any] = {}
        if source.params:
            for key, value in source.params.items():
                if isinstance(value, str) and value.startswith("{{") and value.endswith("}}"):
                    placeholder = value[2:-2].strip()
                    if query_params and placeholder in query_params:
                        params[key] = query_params[placeholder]
                    elif session_data and placeholder in session_data:
                        params[key] = session_data[placeholder]
                else:
                    params[key] = value

        if query_params:
            for key, value in query_params.items():
                params.setdefault(key, value[0] if isinstance(value, list) else value)

        try:
            if source.method.upper() == "GET":
                if params:
                    url = f"{url}?{urlencode(params, encoding='utf-8')}"
                resp = await self.client.get(url, headers=headers)
            else:
                resp = await self.client.request(
                    source.method.upper(), url, headers=headers, json=params or None
                )
            resp.raise_for_status()
            return resp.json()
        except httpx.TimeoutException:
            return {"error": "Timeout", "service": source.service}
        except httpx.HTTPStatusError as e:
            return {"error": f"HTTP {e.response.status_code}", "service": source.service}
        except Exception as e:
            return {"error": str(e), "service": source.service}

    async def fetch_all(
        self,
        sources: List[SourceConfig],
        user_id: str,
        token: Optional[str] = None,
        query_params: Optional[Dict[str, str]] = None,
        impersonated_by: Optional[str] = None,
        session_data: Optional[Dict[str, Any]] = None,
    ) -> Dict[str, Any]:
        tasks = [
            self.fetch(s, user_id, token, query_params, impersonated_by, session_data)
            for s in sources
        ]
        results = await asyncio.gather(*tasks, return_exceptions=True)

        aggregated: Dict[str, Any] = {}
        for i, source in enumerate(sources):
            r = results[i]
            if isinstance(r, BaseException):
                aggregated[source.id] = {"error": str(r)}
            else:
                aggregated[source.id] = r
        return aggregated

    async def close(self):
        await self.client.aclose()
