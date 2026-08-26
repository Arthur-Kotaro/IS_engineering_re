import httpx
import asyncio
import logging
from typing import Dict, Any, List, Union
from src.models import SourceConfig

logger = logging.getLogger(__name__)

class DataAggregator:
    def __init__(self, services_config: Dict[str, Any]):
        self.services = services_config
        self.client = httpx.AsyncClient(timeout=10.0)
    
    async def fetch(self, source: SourceConfig, user_id: str) -> Dict[str, Any]:
        service_config = self.services.get(source.service)
        if not service_config:
            logger.error(f"Service {source.service} not found in config")
            return {"error": f"Service {source.service} not found"}
        
        if isinstance(service_config, dict):
            service_url = service_config.get('url')
        else:
            service_url = service_config
        
        if not service_url:
            logger.error(f"Service {source.service} has no url")
            return {"error": f"Service {source.service} has no url"}
        
        url = f"{service_url}{source.endpoint}"
        headers = {"X-User-ID": user_id}
        
        logger.info(f"Fetching: {url} with X-User-ID: {user_id}")
        
        try:
            response = await self.client.get(url, headers=headers)
            response.raise_for_status()
            logger.info(f"Fetched: {url} -> status {response.status_code}")
            return response.json()
        except httpx.TimeoutException:
            logger.error(f"Timeout: {url}")
            return {"error": "Timeout", "service": source.service}
        except httpx.HTTPStatusError as e:
            logger.error(f"HTTP error: {url} -> {e.response.status_code}")
            return {"error": f"HTTP {e.response.status_code}", "service": source.service}
        except Exception as e:
            logger.error(f"Error: {url} -> {str(e)}")
            return {"error": str(e), "service": source.service}
    
    async def fetch_all(self, sources: List[SourceConfig], user_id: str) -> Dict[str, Any]:
        tasks = [self.fetch(source, user_id) for source in sources]
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
