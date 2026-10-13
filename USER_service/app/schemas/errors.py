# app/schemas/errors.py
from pydantic import BaseModel
from typing import List


class ValidationErrorItem(BaseModel):
    field: str
    code: str
    message: str


class ValidationError(Exception):
    """
    Бизнес-ошибка валидации, возвращаемая клиенту как 400
    с телом {"detail": "...", "errors": [{field, code, message}, ...]}
    """

    def __init__(self, detail: str, errors: List[ValidationErrorItem]):
        self.detail = detail
        self.errors = errors
        super().__init__(detail)
