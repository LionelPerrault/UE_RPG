// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GASAbilityState.h"
#include "GASAbilityStateCooldown.generated.h"

UCLASS(DefaultToInstanced, EditInLineNew, Within = GASAbilityBase, meta = (DisplayName = "Cooldown State"))
class GAMEABILITIES_API UGASAbilityStateCooldown : public UGASAbilityState
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn), Category = "Config")
		float Cooldown;
protected:
	FTimerHandle CooldownTimerDelegate;
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginState(UGASAbilityState* PrevState) override;
	virtual void EndState() override;
	virtual void ExecuteActionSequence() override;
	virtual void StopActionSequence() override;
	virtual void FinishActionSequence() override;
	UFUNCTION()
		void FinishCooldown();
};