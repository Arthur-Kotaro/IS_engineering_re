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
        self.internal_key = os.getenv("INTERNAL_API_KEY", "k4x9pLm2Qw8Rt5Yv7Bn3Fd1Gs6Hj0CzX")
    
    async def fetch(
        self,
        source: SourceConfig,
        user_id: str,
        token: Optional[str] = None,
        query_params: Optional[Dict[str, str]] = None
    ) -> Dict[str, Any]:
        service_config = self.services.get(source.service)
        if not service_config:
            logger.error(f"Service {source.service} not found in config")
            return {"error": f"Service {source.service} not found"}
        
        service_url = service_config.get('url')
        if not service_url:
            logger.error(f"Service {source.service} has no url")
            return {"error": f"Service {source.service} has no url"}
        
        url = f"{service_url}{source.endpoint}"
        headers = {"X-User-ID": user_id}
        
        if source.endpoint.startswith("/internal"):
            headers["X-Internal-Key"] = self.internal_key
            logger.info(f"🔑 Using internal key for: {url}")
        elif token:
            headers["Authorization"] = f"Bearer {token}"
            logger.info(f"🔐 Using JWT for: {url}")
        
        # Собираем параметры
        params = {}
        
        # Сначала параметры из source.params
        if source.params:
            # Подставляем значения из query_params в шаблоны source.params
            for key, value in source.params.items():
                if isinstance(value, str) and value.startswith("{{") and value.endswith("}}"):
                    placeholder = value[2:-2].strip()
                    if query_params and placeholder in query_params:
                        params[key] = query_params[placeholder]
                    else:
                        params[key] = value
                else:
                    params[key] = value
        
        # Затем параметры из query_params
        if query_params:
            for key, value in query_params.items():
                if isinstance(value, list):
                    params[key] = value[0]
                else:
                    params[key] = value
        
        # URL-encode параметры
        if params:
            encoded_params = urlencode(params, encoding='utf-8')
            url_with_params = f"{url}?{encoded_params}"
            logger.info(f"📡 Fetching: {url_with_params}")
        else:
            url_with_params = url
            logger.info(f"📡 Fetching: {url}")
        
        try:
            response = await self.client.get(url_with_params, headers=headers)
            response.raise_for_status()
            logger.info(f"✅ Fetched: {url} -> status {response.status_code}")
            return response.json()
        except httpx.TimeoutException:
            logger.error(f"⏰ Timeout: {url}")
            return {"error": "Timeout", "service": source.service}
        except httpx.HTTPStatusError as e:
            logger.error(f"❌ HTTP error: {url} -> {e.response.status_code}")
            return {"error": f"HTTP {e.response.status_code}", "service": source.service}
        except Exception as e:
            logger.error(f"❌ Error: {url} -> {str(e)}")
            return {"error": str(e), "service": source.service}
    
    async def fetch_all(
        self,
        sources: List[SourceConfig],
        user_id: str,
        token: Optional[str] = None,
        query_params: Optional[Dict[str, str]] = None
    ) -> Dict[str, Any]:
        tasks = [self.fetch(source, user_id, token, query_params) for source in sources]
        results = await asyncio.gather(*tasks, return_exceptions=True)
        
        aggregated = {}
        for i, source in enumerate(sources):
            if isinstance(results[i], Exception):
                logger.error(f"Exception for {source.id}: {str(results[i])}")
                aggregated[source.id] = {"error": str(results[i])}
            else:
                aggregated[source.id] = results[i]
                logger.info(f"Data for {source.id}: {str(results[i])[:200]}...")
        
        return aggregated
    
    async def close(self):
        await self.client.aclose()
