# app/api/v1/rules.py
from fastapi import APIRouter, Depends, HTTPException, status, Request
from typing import List
from app.database import get_db
from app.schemas.rule import DelegationRuleCreate, DelegationRuleResponse
from app.repositories.rule_repo import RuleRepository
from sqlalchemy.ext.asyncio import AsyncSession

router = APIRouter(prefix="/api/v1/delegation-rules", tags=["Delegation Rules"])


async def get_rule_repo(db: AsyncSession = Depends(get_db)) -> RuleRepository:
    return RuleRepository(db)


def _is_super_admin(request: Request) -> bool:
    role = request.headers.get("X-User-Role", "")
    return "super_admin" in role.split(",")


@router.get("/", response_model=List[DelegationRuleResponse])
async def get_all_rules(
    request: Request,
    repo: RuleRepository = Depends(get_rule_repo),
):
    if not _is_super_admin(request):
        raise HTTPException(403, "Only super admin")
    return await repo.get_all()


@router.get("/{role}", response_model=DelegationRuleResponse)
async def get_rule(
    role: str,
    request: Request,
    repo: RuleRepository = Depends(get_rule_repo),
):
    if not _is_super_admin(request):
        raise HTTPException(403, "Only super admin")
    rule = await repo.get_by_role(role)
    if not rule:
        raise HTTPException(404, f"Rule for '{role}' not found")
    return rule


@router.post("/", response_model=DelegationRuleResponse, status_code=201)
async def create_rule(
    data: DelegationRuleCreate,
    request: Request,
    repo: RuleRepository = Depends(get_rule_repo),
):
    if not _is_super_admin(request):
        raise HTTPException(403, "Only super admin")
    existing = await repo.get_by_role(data.role)
    if existing:
        raise HTTPException(400, f"Rule for '{data.role}' already exists")
    return await repo.create(**data.model_dump())


@router.put("/{role}", response_model=DelegationRuleResponse)
async def update_rule(
    role: str,
    data: DelegationRuleCreate,
    request: Request,
    repo: RuleRepository = Depends(get_rule_repo),
):
    if not _is_super_admin(request):
        raise HTTPException(403, "Only super admin")
    rule = await repo.get_by_role(role)
    if not rule:
        raise HTTPException(404, f"Rule for '{role}' not found")
    payload = data.model_dump()
    payload.pop("role", None)
    return await repo.update(rule.rule_id, **payload)
