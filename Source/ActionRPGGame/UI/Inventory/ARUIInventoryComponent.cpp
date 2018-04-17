// Fill out your copyright notice in the Description page of Project Settings.

#include "ARUIInventoryComponent.h"
#include "ARPlayerController.h"
#include "ARCharacter.h"

#include "Weapons/ARItemWeapon.h"

#include "UI/ARHUD.h"
#include "UI/Inventory/ARInventoryScreenWidget.h"
#include "UI/Inventory/Weapons/ARListItemWeaponWidget.h"


// Sets default values for this component's properties
UARUIInventoryComponent::UARUIInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UARUIInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UARUIInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UARUIInventoryComponent::CreateInventoryView(AARPlayerController* PC)
{
	if (InventoryViewClass)
	{
		InventoryView = CreateWidget<UARInventoryScreenWidget>(PC, InventoryViewClass);

		InventoryView->SetVisibility(ESlateVisibility::Collapsed);
		InventoryView->AddToViewport();

		if (AARCharacter* Character = Cast<AARCharacter>(PC->GetPawn()))
		{
			Character->WeaponInventory->GetOnItemAdded().AddUObject(this, &UARUIInventoryComponent::OnWeaponAdded);
			Character->WeaponInventory->GetOnItemUpdated().AddUObject(this, &UARUIInventoryComponent::OnWeaponUpdated);
			Character->WeaponInventory->GetOnItemRemoved().AddUObject(this, &UARUIInventoryComponent::OnWeaponRemoved);
		}
	}
}

void UARUIInventoryComponent::ShowHideInventory()
{
	if (InventoryView->GetVisibility() == ESlateVisibility::Collapsed)
	{
		InventoryView->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else if (InventoryView->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
	{
		InventoryView->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UARUIInventoryComponent::OnWeaponAdded(uint8 NetIndex, uint8 LocalIndex, class UIFItemBase* Item)
{
	InventoryView->OnWeaponAdded(NetIndex, LocalIndex, Item);
}
void UARUIInventoryComponent::OnWeaponUpdated(uint8 NetIndex, uint8 LocalIndex, class UIFItemBase* Item)
{
	InventoryView->OnWeaponUpdated(NetIndex, LocalIndex, Item);
}
void UARUIInventoryComponent::OnWeaponRemoved(uint8 NetIndex, uint8 LocalIndex, class UIFItemBase* Item)
{
	InventoryView->OnWeaponRemoved(NetIndex, LocalIndex, Item);
}

void UARUIInventoryComponent::ShowWeaponsForSlot(class UARItemView* ForSlot)
{
	TArray<UARItemWeapon*> Items;
	AARPlayerController* PC = nullptr;
	if (AARHUD* HUD = Cast<AARHUD>(GetOwner()))
	{
		PC = Cast<AARPlayerController>(HUD->PlayerOwner);
		if (PC)
		{
			Items = PC->MainInventory->GetItems<UARItemWeapon>(UARItemWeapon::StaticClass());
		}
	}

	InventoryView->AddItems<UARItemWeapon, UARListItemWeaponWidget>(Items, ListItemWeaponClass, PC, ForSlot);
}

void UARUIInventoryComponent::AddWeaponToSlot(uint8 TargetNetIndex
	, uint8 TargetLocalIndex
	, uint8 SourceNetIndex
	, uint8 SourceLocalIndex)
{
	AARHUD* HUD = Cast<AARHUD>(GetOwner());
	if (!HUD)
		return;
	AARPlayerController* PC = Cast<AARPlayerController>(HUD->PlayerOwner);
	if (!PC)
		return;
	
	AARCharacter* Character = Cast<AARCharacter>(PC->GetPawn());
	if (!Character)
		return;

	Character->WeaponInventory->AddItemFromOtherInventory(PC->MainInventory, SourceLocalIndex, TargetLocalIndex);

}

void UARUIInventoryComponent::UnequipWeaponFromSlot(uint8 SourceNetIndex, uint8 SourceLocalIndex)
{
	AARHUD* HUD = Cast<AARHUD>(GetOwner());
	if (!HUD)
		return;
	AARPlayerController* PC = Cast<AARPlayerController>(HUD->PlayerOwner);
	if (!PC)
		return;

	AARCharacter* Character = Cast<AARCharacter>(PC->GetPawn());
	if (!Character)
		return;

	PC->MainInventory->AddItemFromOtherInventoryAny(Character->WeaponInventory, SourceLocalIndex);
	Character->WeaponInventory->Unequip(SourceLocalIndex);
}