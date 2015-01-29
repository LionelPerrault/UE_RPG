// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameSystem.h"

#include "GSItem.h"
#include "GSEquipmentComponent.h"
#include "../Weapons/GSWeaponEquipmentComponent.h"
#include "../Weapons/GSWeaponRanged.h"
#include "../Components/GSActiveActionsComponent.h"
#include "Net/UnrealNetwork.h"

#include "GSItemWeaponRangedInfo.h"

UGSItemWeaponRangedInfo::UGSItemWeaponRangedInfo(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	LastAttachedSocket = NAME_None;
	CurrentHand = EGSWeaponHand::Invalid;
}
void UGSItemWeaponRangedInfo::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGSItemWeaponRangedInfo, RangedWeaponClass);
	DOREPLIFETIME(UGSItemWeaponRangedInfo, RangedWeapon);
}
void UGSItemWeaponRangedInfo::SetEquipingTime(float TimeIn)
{
	if (RangedWeapon)
		RangedWeapon->SetEquipingTime(TimeIn);
}
int32 UGSItemWeaponRangedInfo::GetCurrentMagainzeAmmo()
{
	if (RangedWeapon)
	{
		return RangedWeapon->GetRemainingMagazineAmmo();
	}
	return 0;
}
int32 UGSItemWeaponRangedInfo::GetCurrentTotalAmmo()
{
	if (RangedWeapon)
	{
		return RangedWeapon->GetRemaningAmmo();
	}
	return 0;
}
UTexture2D* UGSItemWeaponRangedInfo::GetImage()
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		return RangedWeaponClass.GetDefaultObject()->Icon;
	}
	return nullptr;
}

AActor* UGSItemWeaponRangedInfo::GetActorToAttach()
{
	return RangedWeapon;
}

const TArray<FGSWeaponSocketInfo>& UGSItemWeaponRangedInfo::GetPossibleWeaponSockets() const
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		return RangedWeaponClass.GetDefaultObject()->SocketList;
	}
	return socketArray;
}
const EGSWeaponWield UGSItemWeaponRangedInfo::GetWeaponWield() const
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		return RangedWeaponClass.GetDefaultObject()->WieldType;
	}
	return EGSWeaponWield::Invalid;
}
const EGSWeaponType UGSItemWeaponRangedInfo::GetWeaponType() const
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		return RangedWeaponClass.GetDefaultObject()->WeaponType;
	}
	return EGSWeaponType::Invalid;
}
FVector UGSItemWeaponRangedInfo::GetCrosshairStartLocation()
{
	if (RangedWeapon)
		return RangedWeapon->GetWeaponSocketLocation();
	return FVector::ZeroVector;
}

UAnimSequence* UGSItemWeaponRangedInfo::GetEquipedAnimation()
{

	return nullptr;
}

UAnimSequence* UGSItemWeaponRangedInfo::GetIdleAnimation()
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		switch (CurrentHand)
		{
		case EGSWeaponHand::Left:
			return RangedWeaponClass.GetDefaultObject()->LeftHandIdle;
		case EGSWeaponHand::Right:
			return RangedWeaponClass.GetDefaultObject()->RightHandIdle;
		case EGSWeaponHand::BothHands:
			return RangedWeaponClass.GetDefaultObject()->BothHandsIdle;
		case EGSWeaponHand::Invalid:
			return nullptr;
		default:
			return nullptr;
		}
	}
	return nullptr;
}
UAnimSequence* UGSItemWeaponRangedInfo::GetMoveAnimation()
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		switch (CurrentHand)
		{
		case EGSWeaponHand::Left:
			return RangedWeaponClass.GetDefaultObject()->LeftHandMove;
		case EGSWeaponHand::Right:
			return RangedWeaponClass.GetDefaultObject()->RightHandMove;
		case EGSWeaponHand::BothHands:
			return RangedWeaponClass.GetDefaultObject()->BothHandsMove;
		case EGSWeaponHand::Invalid:
			return nullptr;
		default:
			return nullptr;
		}
	}
	return nullptr;
}
UAnimSequence* UGSItemWeaponRangedInfo::GetCombatAnimation()
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		switch (CurrentHand)
		{
		case EGSWeaponHand::Left:
			return RangedWeaponClass.GetDefaultObject()->LeftHandCombat;
		case EGSWeaponHand::Right:
			return RangedWeaponClass.GetDefaultObject()->RightHandCombat;
		case EGSWeaponHand::BothHands:
			return RangedWeaponClass.GetDefaultObject()->BothHandsCombat;
		case EGSWeaponHand::Invalid:
			return nullptr;
		default:
			return nullptr;
		}
	}
	return nullptr;
}

FVector UGSItemWeaponRangedInfo::GetCrosshairTraceStartLocation()
{
	if (!RangedWeapon)
		return FVector::ZeroVector;

	return RangedWeapon->GetWeaponSocketLocation();
}
const float UGSItemWeaponRangedInfo::GetCurrentHorizontalRecoil() const
{
	if (RangedWeapon)
		return RangedWeapon->CurrentHorizontalRecoil;
	return 0;
}
const float UGSItemWeaponRangedInfo::GetCurrentVerticalRecoil() const
{
	if (RangedWeapon)
		return RangedWeapon->CurrentVerticalRecoil;
	return 0;
}
UAimOffsetBlendSpace* UGSItemWeaponRangedInfo::GetEquipedAimBlendSpace()
{
	if (RangedWeaponClass.GetDefaultObject())
	{
		return RangedWeaponClass.GetDefaultObject()->AimOffsetBlend;
		//switch (CurrentHand)
		//{
		//case EGSWeaponHand::Left:
		//	return RangedWeaponClass.GetDefaultObject()->LeftHandAnimation;
		//case EGSWeaponHand::Right:
		//	return RangedWeaponClass.GetDefaultObject()->RightHandAnimation;
		//case EGSWeaponHand::BothHands:
		//	return RangedWeaponClass.GetDefaultObject()->BothHandsAnimation;
		//case EGSWeaponHand::Invalid:
		//	return nullptr;
		//default:
		//	return nullptr;
		//}
	}
	return nullptr;
}
bool UGSItemWeaponRangedInfo::OnItemAddedToSlot()
{
	if (RangedWeaponClass)
	{
		if (UGSWeaponEquipmentComponent* eqComp = Cast<UGSWeaponEquipmentComponent>(CurrentInventory))
		{
			RangedWeapon = GetWorld()->SpawnActor<AGSWeaponRanged>(RangedWeaponClass);
			RangedWeapon->SetOwner(OwningPawn);
			RangedWeapon->Instigator = OwningPawn;
			RangedWeapon->WeaponPC = OwiningPlayerController;

			//we will use wepon defaults if it zero.
			if (RemainingAmmoMagazine != 0)
				RangedWeapon->SetRemainingMagazineAmmo(RemainingAmmoMagazine);
			if (RemainingAmmoTotal != 0)
				RangedWeapon->SetRemaningAmmofloat(RemainingAmmoTotal);

			RangedWeapon->InitializeWeapon();
			eqComp->AttachActorTo(RangedWeapon, LastAttachedSocket, RangedWeapon->SocketList);
		}
	}
	return false;
}
bool UGSItemWeaponRangedInfo::OnItemRemovedFromSlot()
{
	if (RangedWeapon)
	{
		UGSWeaponEquipmentComponent* eqComp = Cast<UGSWeaponEquipmentComponent>(CurrentInventory);
		if (eqComp)
		{
			for (FGSEquipSocketInfo& socket : eqComp->AttachmentSockets)
			{
				if (socket.SocketName == LastAttachedSocket)
					socket.bIsSocketAvailable = true;
			}
		}
		if (!eqComp)
		{
			UGSWeaponEquipmentComponent* eqLastComp = Cast<UGSWeaponEquipmentComponent>(LastInventory);
			if (eqLastComp)
			{
				for (FGSEquipSocketInfo& socket : eqLastComp->AttachmentSockets)
				{
					if (socket.SocketName == LastAttachedSocket)
					{
						socket.bIsSocketAvailable = true;
						break;
					}
				}
			}
			LastAttachedSocket = NAME_Name;
			RemainingAmmoMagazine = RangedWeapon->GetRemainingMagazineAmmo();
			RemainingAmmoTotal = RangedWeapon->GetRemaningAmmo();
			RangedWeapon->Destroy();
		}
		return true;
	}
	return false;
}

bool UGSItemWeaponRangedInfo::InputPressed_Implementation()
{
	if (RangedWeapon)
	{
		RangedWeapon->InputPressed();
		return true;
	}
	return false;
}
bool UGSItemWeaponRangedInfo::InputReleased_Implementation()
{
	if (RangedWeapon)
	{
		RangedWeapon->InputReleased();
		return true;
	}
	return false;
}
void UGSItemWeaponRangedInfo::InputRealadWeapon()
{
	if (RangedWeapon)
	{
		RangedWeapon->InputReload();
	}
}