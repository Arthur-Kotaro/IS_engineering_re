# app/services/delegation_service.py
from typing import Optional, List
from datetime import datetime, timezone
from fastapi import HTTPException, status
from app.repositories.delegation_repo import DelegationRepository
from app.repositories.history_repo import HistoryRepository
from app.services.validation_service import ValidationService
from app.services.external_service import ExternalService
from app.schemas.delegation import DelegationCreate, DelegationResponse, DelegationRevoke
from app.models.delegation import DelegationStatus, DelegationType


class DelegationService:
    def __init__(
        self,
        delegation_repo: DelegationRepository,
        history_repo: HistoryRepository,
        validation_service: ValidationService
    ):
        self.delegation_repo = delegation_repo
        self.history_repo = history_repo
        self.validation_service = validation_service
    
    async def create_delegation(
        self,
        data: DelegationCreate,
        created_by: int,
        token: Optional[str] = None,
        ip_address: Optional[str] = None,
        user_agent: Optional[str] = None
    ) -> DelegationResponse:
        """Создать новое делегирование"""
        
        # Валидация
        if data.delegation_type == DelegationType.DIRECT:
            await self.validation_service.validate_direct_delegation(
                data.delegator_id,
                data.delegate_id,
                data.starts_at,
                data.expires_at,
                token
            )
        elif data.delegation_type == DelegationType.REVERSE:
            await self.validation_service.validate_reverse_delegation(
                data.delegator_id,
                data.delegate_id,
                data.starts_at,
                data.expires_at,
                token
            )
        elif data.delegation_type == DelegationType.TEMPORARY:
            await self.validation_service.validate_temporary_delegation(
                data.delegator_id,
                data.delegate_id,
                data.main_delegate_id,
                data.starts_at,
                data.expires_at,
                token
            )
        
        # Получаем имена пользователей для кеша
        delegator_info = await ExternalService.get_user_info(data.delegator_id, token)
        delegate_info = await ExternalService.get_user_info(data.delegate_id, token)
        
        delegator_name = delegator_info.get("user_name") if delegator_info else f"User {data.delegator_id}"
        delegate_name = delegate_info.get("user_name") if delegate_info else f"User {data.delegate_id}"
        
        # Создаем делегирование
        delegation_data = {
            "delegator_id": data.delegator_id,
            "delegator_name": delegator_name,
            "delegate_id": data.delegate_id,
            "delegate_name": delegate_name,
            "delegation_type": data.delegation_type,
            "starts_at": data.starts_at,
            "expires_at": data.expires_at,
            "reason": data.reason,
            "created_by": created_by,
            "main_delegate_id": data.main_delegate_id
        }
        
        delegation = await self.delegation_repo.create(**delegation_data)
        
        # Логируем историю
        await self.history_repo.create(
            delegation_id=delegation.delegation_id,
            action="CREATED",
            user_id=created_by,
            details={
                "type": data.delegation_type,
                "delegator": data.delegator_id,
                "delegate": data.delegate_id,
                "starts_at": data.starts_at.isoformat(),
                "expires_at": data.expires_at.isoformat()
            },
            ip_address=ip_address,
            user_agent=user_agent
        )
        
        # ----- ОТПРАВКА УВЕДОМЛЕНИЙ -----
        
        type_names = {
            DelegationType.DIRECT: "прямое делегирование",
            DelegationType.REVERSE: "обратное делегирование",
            DelegationType.TEMPORARY: "временное делегирование"
        }
        
        # 1. Уведомление делегату (получателю полномочий)
        await ExternalService.send_notification(
            user_id=data.delegate_id,
            title=f"Вам делегированы полномочия",
            message=(
                f"Пользователь {delegator_name} делегировал вам полномочия.\n"
                f"Тип: {type_names.get(data.delegation_type, data.delegation_type)}\n"
                f"Действует до: {data.expires_at.strftime('%d.%m.%Y %H:%M')}"
            ),
            notification_type="delegation_created",
            reference_id=delegation.delegation_id,
            reference_type="delegation",
            data={
                "delegator_id": data.delegator_id,
                "delegator_name": delegator_name,
                "delegation_type": data.delegation_type,
                "expires_at": data.expires_at.isoformat()
            },
            token=token
        )
        
        # 2. Уведомление делегатору (создателю) о том, что делегирование создано
        await ExternalService.send_notification(
            user_id=data.delegator_id,
            title=f"Делегирование создано",
            message=(
                f"Вы делегировали полномочия пользователю {delegate_name}.\n"
                f"Тип: {type_names.get(data.delegation_type, data.delegation_type)}\n"
                f"Действует до: {data.expires_at.strftime('%d.%m.%Y %H:%M')}"
            ),
            notification_type="delegation_created",
            reference_id=delegation.delegation_id,
            reference_type="delegation",
            data={
                "delegate_id": data.delegate_id,
                "delegate_name": delegate_name,
                "delegation_type": data.delegation_type,
                "expires_at": data.expires_at.isoformat()
            },
            token=token
        )
        
        # 3. Если временное делегирование — уведомление основному сотруднику
        if data.delegation_type == DelegationType.TEMPORARY and data.main_delegate_id:
            main_info = await ExternalService.get_user_info(data.main_delegate_id, token)
            main_name = main_info.get("user_name") if main_info else f"User {data.main_delegate_id}"
            
            await ExternalService.send_notification(
                user_id=data.main_delegate_id,
                title=f"Временное замещение",
                message=(
                    f"Пользователь {delegate_name} временно замещает вас.\n"
                    f"Действует до: {data.expires_at.strftime('%d.%m.%Y %H:%M')}"
                ),
                notification_type="delegation_temporary",
                reference_id=delegation.delegation_id,
                reference_type="delegation",
                data={
                    "delegate_id": data.delegate_id,
                    "delegate_name": delegate_name,
                    "expires_at": data.expires_at.isoformat()
                },
                token=token
            )
        
        return DelegationResponse.model_validate(delegation)
    
    async def revoke_delegation(
        self,
        delegation_id: int,
        user_id: int,
        data: DelegationRevoke,
        is_super_admin: bool = False,
        ip_address: Optional[str] = None,
        user_agent: Optional[str] = None,
        token: Optional[str] = None
    ) -> DelegationResponse:
        """Отозвать делегирование"""
        
        # Проверяем права
        can_revoke = await self.validation_service.can_revoke(delegation_id, user_id, is_super_admin)
        if not can_revoke:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Only delegator or admin can revoke"
            )
        
        # Получаем делегирование до отзыва
        delegation_before = await self.delegation_repo.get_by_id(delegation_id)
        
        # Отзываем
        delegation = await self.delegation_repo.revoke(
            delegation_id,
            revoked_by=user_id,
            reason=data.reason
        )
        if not delegation:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Delegation not found"
            )
        
        # Логируем историю
        await self.history_repo.create(
            delegation_id=delegation.delegation_id,
            action="REVOKED",
            user_id=user_id,
            details={"reason": data.reason},
            ip_address=ip_address,
            user_agent=user_agent
        )
        
        # ----- ОТПРАВКА УВЕДОМЛЕНИЙ ПРИ ОТЗЫВЕ -----
        
        # Уведомление делегату
        await ExternalService.send_notification(
            user_id=delegation.delegate_id,
            title=f"Делегирование отозвано",
            message=(
                f"Пользователь {delegation.delegator_name} отозвал делегирование.\n"
                f"Причина: {data.reason or 'Не указана'}"
            ),
            notification_type="delegation_revoked",
            reference_id=delegation.delegation_id,
            reference_type="delegation",
            token=token
        )
        
        # Уведомление делегатору о подтверждении отзыва
        if delegation.delegator_id != user_id:
            await ExternalService.send_notification(
                user_id=delegation.delegator_id,
                title=f"Делегирование отозвано",
                message=(
                    f"Делегирование для {delegation.delegate_name} было отозвано.\n"
                    f"Причина: {data.reason or 'Не указана'}"
                ),
                notification_type="delegation_revoked",
                reference_id=delegation.delegation_id,
                reference_type="delegation",
                token=token
            )
        
        return DelegationResponse.model_validate(delegation)
    
    async def get_active_delegations(self, user_id: int) -> List[DelegationResponse]:
        """Получить активные делегирования для пользователя"""
        delegations = await self.delegation_repo.get_active_by_user(user_id)
        return [DelegationResponse.model_validate(d) for d in delegations]
    
    async def get_user_active_delegation(self, user_id: int) -> Optional[DelegationResponse]:
        """Получить активное делегирование для пользователя (как делегата)"""
        delegations = await self.delegation_repo.get_active_delegations_for_user(user_id)
        if delegations:
            return DelegationResponse.model_validate(delegations[0])
        return None
    
    async def get_delegations_by_delegator(self, delegator_id: int) -> List[DelegationResponse]:
        """Получить все делегирования созданные пользователем"""
        delegations = await self.delegation_repo.get_delegations_by_delegator(delegator_id)
        return [DelegationResponse.model_validate(d) for d in delegations]
    
    async def get_delegations_by_delegate(self, delegate_id: int) -> List[DelegationResponse]:
        """Получить все делегирования для пользователя (как делегата)"""
        delegations = await self.delegation_repo.get_delegations_by_delegate(delegate_id)
        return [DelegationResponse.model_validate(d) for d in delegations]
    
    async def check_delegation_for_user(self, user_id: int) -> dict:
        """Проверить, есть ли у пользователя активное делегирование"""
        delegation = await self.get_user_active_delegation(user_id)
        if delegation:
            return {
                "has_delegation": True,
                "delegation_id": delegation.delegation_id,
                "delegated_from": delegation.delegator_id,
                "delegation_type": delegation.delegation_type,
                "expires_at": delegation.expires_at
            }
        return {"has_delegation": False}
    
    async def expire_expired_delegations(self) -> int:
        """Автоматически истекать просроченные делегирования"""
        expired = await self.delegation_repo.get_expired()
        count = 0
        for delegation in expired:
            await self.delegation_repo.mark_expired(delegation.delegation_id)
            await self.history_repo.create(
                delegation_id=delegation.delegation_id,
                action="EXPIRED",
                user_id=0,  # Система
                details={"auto_expired": True}
            )
            
            # Уведомление об истечении (без токена, т.к. это системное)
            await ExternalService.send_notification(
                user_id=delegation.delegate_id,
                title=f"Срок делегирования истек",
                message=(
                    f"Срок делегирования от {delegation.delegator_name} истек."
                ),
                notification_type="delegation_expired",
                reference_id=delegation.delegation_id,
                reference_type="delegation"
            )
            
            count += 1
        return count
