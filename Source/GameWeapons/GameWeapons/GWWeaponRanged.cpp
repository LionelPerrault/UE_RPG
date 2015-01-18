// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameWeapons.h"
#include "States/GWWeaponState.h"
#include "Tracing/GWTraceBase.h"
#include "Tracing/GWTraceRangedWeapon.h"

#include "Ammo/GWAmmo.h"

#include "IGISkeletalMesh.h"

#include "Net/UnrealNetwork.h"

#include "GWWeaponRanged.h"

AGWWeaponRanged::AGWWeaponRanged(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bReplicates = true;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	PrimaryActorTick.bRunOnAnyThread = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_DuringPhysics;
	SetTickGroup(ETickingGroup::TG_DuringPhysics);


	bIsWeaponFiring = false;

	ReloadEndCount = 0;
}
void AGWWeaponRanged::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bTraceEveryTick && bIsWeaponFiring && TargetingMethod)
	{
		TargetingMethod->SingleLineTraceSetHitLocation();
		if (Role < ROLE_Authority || GetNetMode() == ENetMode::NM_Standalone)
			OnWeaponFiring(HitInfo.Origin, HitInfo.HitLocation);
	}
}
void AGWWeaponRanged::BeginPlay()
{
	if (TargetingMethod)
	{
		TargetingMethod->SetRange(Range);
		TargetingMethod->SetCurrentSpread(BaseSpread);
		TargetingMethod->Initialize();
	}
	CurrentState = ActiveState;
	RemaningAmmo = MaximumAmmo;
	RemainingMagazineAmmo = MagazineSize;
	if (TypeOfAmmo)
	{
		CurrentAmmo = ConstructObject<UGWAmmo>(TypeOfAmmo, this);
	}
}
void AGWWeaponRanged::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGWWeaponRanged, RemainingMagazineAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGWWeaponRanged, RemaningAmmo, COND_OwnerOnly);
	DOREPLIFETIME(AGWWeaponRanged, ReloadEndCount);
	DOREPLIFETIME(AGWWeaponRanged, ReloadBeginCount);
}

void AGWWeaponRanged::InitializeWeapon()
{
	Super::InitializeWeapon();
}
void AGWWeaponRanged::InputPressed()
{
	BeginFire();
}
void AGWWeaponRanged::InputReleased()
{
	EndFire();
}

void AGWWeaponRanged::InputReload()
{
	BeginReload();
}
void AGWWeaponRanged::FireWeapon(const FHitResult& TargetIn, float DamageIn, APawn* InstigatorIn)
{
	if (CurrentAmmo)
	{
		CurrentAmmo->ApplyDamage(TargetIn, DamageIn, InstigatorIn);
	}
}
void AGWWeaponRanged::ActionBegin()
{
	//check for ammo, subtract it
	//if no ammo end current state
	//and go to reload state.
	//GotoState(ReloadState);
	//or don't do anything abort firing, goto active state
	//and let player deicide if he want to reaload.
	if (!CheckIfHaveAmmo())
	{
		CurrentState->EndActionSequence();
		return;
	}
	SubtractAmmo();

	if (TargetingMethod)
		TargetingMethod->Execute();
	OnFireBegin();

	//fire Ammo somewhere around here.
	/*
		1. It should be automatic. Designer should not remember to put ammo.
		2. Even if, it should not be expected to call in in right place.
		3. Worst case scnario, ammo should be called as either first or last node.
	*/

	if (GetNetMode() == ENetMode::NM_Standalone)
		OnRep_HitInfo();
}
void AGWWeaponRanged::ActionEnd()
{

}

//activate states
void AGWWeaponRanged::BeginFire()
{
	if (Role < ROLE_Authority)
	{
		if (!CheckIfHaveAmmo())
			return;
		SubtractAmmo();
		
		bIsWeaponFiring = true;

		ServerBeginFire();
		if (TargetingMethod)
			TargetingMethod->Execute();
		OnRep_HitInfo();


	}
	else
	{
		if (!CheckIfHaveAmmo())
		{
			CurrentState->EndActionSequence();
			return;
		}
		bIsWeaponFiring = true;
		SubtractAmmo();
		CurrentState->BeginActionSequence();
	}
}
void AGWWeaponRanged::ServerBeginFire_Implementation()
{
	BeginFire();
}
bool AGWWeaponRanged::ServerBeginFire_Validate()
{
	return true;
}

void AGWWeaponRanged::EndFire()
{
	if (Role < ROLE_Authority)
	{
		bIsWeaponFiring = false;
		ServerEndFire();
	}
	else
	{
		bIsWeaponFiring = false;
		CurrentState->EndActionSequence();
		OnFireEnd();
	}
}
void AGWWeaponRanged::ServerEndFire_Implementation()
{
	EndFire();
}
bool AGWWeaponRanged::ServerEndFire_Validate()
{
	return true;
}

void AGWWeaponRanged::BeginReload()
{
	if (Role < ROLE_Authority)
	{
		OnRep_ReloadBeign();
		ServerBeginReload();
	}
	else
	{
		ReloadBeginCount++;
		GotoState(ReloadState);
		if (GetNetMode() == ENetMode::NM_Standalone)
			OnRep_ReloadBeign();
	}
}
void AGWWeaponRanged::ServerBeginReload_Implementation()
{
	BeginReload();
}
bool AGWWeaponRanged::ServerBeginReload_Validate()
{
	return true;
}

void AGWWeaponRanged::CalculateReloadAmmo()
{
	float NeededAmmo = MagazineSize - RemainingMagazineAmmo;
	if (NeededAmmo <= RemainingMagazineAmmo)
		return;

	RemaningAmmo -= NeededAmmo;
	RemainingMagazineAmmo += NeededAmmo;
}

void AGWWeaponRanged::EndReload()
{
	if (Role < ROLE_Authority)
	{
		if (!CheckIfCanReload())
			return;
		CalculateReloadAmmo();
		ServerEndReload();
	}
	else
	{
		/*
			Impelemtnsome reload mechanic..
			1. Where is ammo coming from ?
			2. How it is setup ?
			3. Does weapon have it's own magazine max ammo count, or is it stored externally ?
		*/
		if (!CheckIfCanReload())
			return;
		CalculateReloadAmmo();
		ReloadEndCount++;
		if (GetNetMode() == ENetMode::NM_Standalone)
			OnRep_ReloadEnd();
	}
}
void AGWWeaponRanged::ServerEndReload_Implementation()
{
	EndReload();
}
bool AGWWeaponRanged::ServerEndReload_Validate()
{
	return true;
}

void AGWWeaponRanged::OnRep_Shooting()
{
	//OnWeaponFiring();
}

void AGWWeaponRanged::OnRep_ReloadBeign()
{
	OnReloadBegin();
	if (ReloadAnimation)
	{
		IIGISkeletalMesh* skelMeshInt = Cast<IIGISkeletalMesh>(Instigator);
		if (!skelMeshInt)
			return;

		UAnimInstance* AnimInst = skelMeshInt->GetMasterSkeletalMesh()->GetAnimInstance();
		if (!AnimInst)
			return;
		float montageLenght = ReloadAnimation->CalculateSequenceLength();

		float multiplier = montageLenght / ReloadTime;

		AnimInst->Montage_Play(ReloadAnimation, multiplier);
	}
}

void AGWWeaponRanged::OnRep_ReloadEnd()
{
	OnReloadEnd();
}

void AGWWeaponRanged::OnRep_HitInfo()
{
	OnWeaponFiring(HitInfo.Origin, HitInfo.HitLocation);
}

bool AGWWeaponRanged::CheckIfHaveAmmo()
{
	if (RemainingMagazineAmmo > 0)
		return true;
	else
		return false;
}
void AGWWeaponRanged::SubtractAmmo()
{
	RemainingMagazineAmmo -= AmmoCost;
}
bool AGWWeaponRanged::CheckIfCanReload()
{
	if (RemainingMagazineAmmo >= MagazineSize)
		return false;
	else
		return true;
}