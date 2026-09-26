import os
import logging
from fastapi import FastAPI, HTTPException, Header, Request, Query
from fastapi.middleware.cors import CORSMiddleware
from typing import Optional, Dict, Any
from urllib.parse import urlencode

from src.template_loader import TemplateLoader
from src.data_aggregator import DataAggregator
from src.ui_builder import UIBuilder
import yaml

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="UI Composer Service", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

with open("config/services.yaml", 'r') as f:
    config = yaml.safe_load(f)

services = config.get('services', {})
logger.info(f"Loaded services: {services}")

template_loader = TemplateLoader("templates")
data_aggregator = DataAggregator(services)
ui_builder = UIBuilder()

@app.on_event("shutdown")
async def shutdown():
    await data_aggregator.close()

@app.get("/health")
async def health():
    return {"status": "ok"}

@app.get("/pages")
async def list_pages():
    pages = []
    for f in os.listdir("templates"):
        if f.endswith(".yaml"):
            pages.append(f.replace(".yaml", ""))
    return {"pages": pages}

@app.get("/page/{full_path:path}")
async def get_page(
    full_path: str,
    request: Request,
    authorization: Optional[str] = Header(None),
    x_user_id: Optional[str] = Header(None)
):
    logger.info(f"=== get_page called with full_path: {full_path} ===")
    
    if not x_user_id:
        logger.warning(f"Missing X-User-ID header for {full_path}")
        raise HTTPException(status_code=401, detail="Missing X-User-ID header")
    
    if not authorization:
        logger.warning(f"Missing Authorization header for {full_path}")
        raise HTTPException(status_code=401, detail="Authorization header required")
    
    token = authorization.replace("Bearer ", "")
    
    # Получаем параметры запроса
    query_params = dict(request.query_params)
    logger.info(f"Query params: {query_params}")
    
    # Для поиска пользователей передаём параметры в data_aggregator
    if full_path == "hr/search-user" and "search_query" in query_params:
        # Передаём search_query как параметр для DataAggregator
        query_params_for_fetch = {"search_query": query_params.get("search_query")}
        logger.info(f"Search query: {query_params_for_fetch}")
    else:
        query_params_for_fetch = query_params
    
    try:
        logger.info(f"Attempting to load template: {full_path}")
        template = template_loader.load(full_path)
        logger.info(f"Template loaded successfully: {template.name}")
    except FileNotFoundError as e:
        logger.error(f"Template not found: {e}")
        raise HTTPException(status_code=404, detail=f"Page '{full_path}' not found")
    except Exception as e:
        logger.error(f"Template error: {e}")
        raise HTTPException(status_code=500, detail=f"Template error: {str(e)}")
    
    logger.info(f"Fetching data for: {full_path}, sources: {[s.id for s in template.sources]}, user_id: {x_user_id}")
    
    data = await data_aggregator.fetch_all(template.sources, x_user_id, token, query_params_for_fetch)
    logger.info(f"Data fetched: {list(data.keys())}")
    
    ui = ui_builder.build(template, data)
    logger.info(f"UI built: {ui['title']}")
    
    return ui
