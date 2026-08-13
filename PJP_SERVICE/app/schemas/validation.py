from pydantic import BaseModel
from typing import List, Dict, Any


class ValidationError(BaseModel):
    line: int
    field: str
    error: str
    severity: str  # 'error' или 'warning'


class ValidationResponse(BaseModel):
    is_valid: bool
    errors: List[ValidationError] = []
    warnings: List[ValidationError] = []
