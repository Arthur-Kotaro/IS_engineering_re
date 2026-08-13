from typing import List, Dict, Any, Tuple
from app.schemas.validation import ValidationError


def validate_part(part: Dict[str, Any], line_number: int) -> List[Dict[str, Any]]:
    """Валидация одной строки"""
    errors = []

    # Проверка на пустой part
    if not part:
        errors.append({
            "line": line_number,
            "field": "_",
            "error": "Empty part data",
            "severity": "error"
        })
        return errors

    # 1. Обязательные поля
    required_fields = ["part_number", "part_name", "source_type", "quantity"]
    for field in required_fields:
        if not part.get(field):
            errors.append({
                "line": line_number,
                "field": field,
                "error": f"Поле '{field}' обязательно для заполнения",
                "severity": "error"
            })

    # 2. Применяемость
    applies_to = part.get("applies_to_prototypes")
    if not applies_to or not isinstance(applies_to, list) or len(applies_to) == 0:
        errors.append({
            "line": line_number,
            "field": "applies_to_prototypes",
            "error": "Не указана применяемость к прототипам (хотя бы один SVxxxx)",
            "severity": "error"
        })

    # 3. Валидация в зависимости от источника
    source_type = part.get("source_type")

    if source_type in ["PROTOTYPE", "REFINING"]:
        cd_status = part.get("cd_status")
        if not cd_status:
            errors.append({
                "line": line_number,
                "field": "cd_status",
                "error": "Для КИ, производимых УПП, необходимо указать статус КД ('OK' или '15КН')",
                "severity": "error"
            })
        elif cd_status not in ["OK", "15КН"]:
            errors.append({
                "line": line_number,
                "field": "cd_status",
                "error": f"Недопустимый статус КД: '{cd_status}'. Допустимо: 'OK', '15КН'",
                "severity": "error"
            })
        elif cd_status == "15КН" and not part.get("cd_ready_week"):
            errors.append({
                "line": line_number,
                "field": "cd_ready_week",
                "error": "Для КД со статусом '15КН' необходимо указать номер календарной недели",
                "severity": "warning"
            })

    if source_type == "PURCHASE":
        if not part.get("supplier"):
            errors.append({
                "line": line_number,
                "field": "supplier",
                "error": "Для покупных КИ необходимо указать поставщика",
                "severity": "error"
            })
        if not part.get("supplier_code"):
            errors.append({
                "line": line_number,
                "field": "supplier_code",
                "error": "Для покупных КИ необходимо указать код поставщика",
                "severity": "error"
            })

    return errors


def validate_pjp(parts: List[Dict[str, Any]]) -> Tuple[bool, List[Dict[str, Any]], List[Dict[str, Any]]]:
    """Валидация всего PJP"""
    all_errors = []
    all_warnings = []
    
    for idx, part in enumerate(parts, start=1):
        errors = validate_part(part, idx)
        for e in errors:
            if e["severity"] == "error":
                all_errors.append(e)
            else:
                all_warnings.append(e)

    is_valid = len(all_errors) == 0
    return is_valid, all_errors, all_warnings
