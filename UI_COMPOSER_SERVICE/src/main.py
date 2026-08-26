import os
import yaml
import logging
from fastapi import FastAPI, HTTPException, Header
from fastapi.middleware.cors import CORSMiddleware
from typing import Optional

from src.template_loader import TemplateLoader
from src.data_aggregator import DataAggregator
from src.ui_builder import UIBuilder

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

@app.get("/page/{page_name}")
async def get_page(
    page_name: str,
    authorization: Optional[str] = Header(None),
    x_user_id: Optional[str] = Header(None)
):
    if not x_user_id:
        logger.warning(f"Missing X-User-ID header for {page_name}")
        raise HTTPException(status_code=401, detail="Missing X-User-ID header")
    
    if not authorization:
        logger.warning(f"Missing Authorization header for {page_name}")
        raise HTTPException(status_code=401, detail="Authorization header required")
    
    token = authorization.replace("Bearer ", "")
    
    try:
        template = template_loader.load(page_name)
        logger.info(f"Loaded template: {page_name}")
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail=f"Page '{page_name}' not found")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Template error: {str(e)}")
    
    logger.info(f"Fetching data for: {page_name}, sources: {[s.id for s in template.sources]}, user_id: {x_user_id}")
    data = await data_aggregator.fetch_all(template.sources, x_user_id)
    logger.info(f"Data fetched: {list(data.keys())}")
    
    ui = ui_builder.build(template, data)
    logger.info(f"UI built: {ui['title']}")
    
    return ui

@app.get("/pages")
async def list_pages():
    pages = []
    for f in os.listdir("templates"):
        if f.endswith(".yaml"):
            pages.append(f.replace(".yaml", ""))
    return {"pages": pages}
