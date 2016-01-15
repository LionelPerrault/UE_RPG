// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameAttributes.h"
#include "GAAttributeComponent.h"
#include "GAEffectInstanced.h"

UGAEffectInstanced::UGAEffectInstanced(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void UGAEffectInstanced::Initialize(const FGAEffectContext& ContextIn)
{
	Context = ContextIn;
}

FGAGameEffectHandle UGAEffectInstanced::ApplyEffect(TSubclassOf<class UGAGameEffectSpec> SpecIn)
{
	FGAGameEffectHandle handle = Context.InstigatorComp->MakeGameEffect(SpecIn, Context);

	Context.InstigatorComp->ApplyEffectToTarget(handle.GetEffect(), handle);
	return handle;
}

void UGAEffectInstanced::NativeOnEffectExpired()
{
	InternalEffectEnded();
	OnEffectInstanceExpired();
}
void UGAEffectInstanced::NativeOnEffectRemoved()
{
	InternalEffectEnded();
	OnEffectInstanceRemoved();
}

void UGAEffectInstanced::InternalEffectEnded()
{
	UGAAttributeComponent* OwningComp = Context.TargetComp.Get();
	if (!OwningComp)
		return;

	for (FGAGameEffectHandle& Handle : OwnedEffects)
	{
		OwningComp->RemoveEffect(Handle);
	}

	MarkPendingKill();
}