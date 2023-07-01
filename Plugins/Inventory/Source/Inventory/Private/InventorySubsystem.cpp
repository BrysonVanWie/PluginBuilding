// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"

void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UInventorySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UInventorySubsystem::AddItem(uint8 InItemEnum, int32 NumberToAdd)
{
	if (PlayerInventory.Contains(InItemEnum))
	{
		NumberToAdd += PlayerInventory.FindRef(InItemEnum);
	}
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, FString("Tried to add %s items") + FString::FromInt(NumberToAdd));
	PlayerInventory.Emplace(InItemEnum, NumberToAdd);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventorySubsystem::HasItem(uint8 InItemEnum, int32 NumberNeeded)
{
	if (PlayerInventory.Contains(InItemEnum))
	{
		return PlayerInventory.FindRef(InItemEnum) >= NumberNeeded;
	}
	return false;
}

bool UInventorySubsystem::AddManyItems(TMap<uint8, int32> InItems)
{
	TArray<uint8> Keys;
	InItems.GetKeys(Keys);
	for (uint8 Key : Keys)
	{
		AddItem(Key, InItems.FindRef(Key));
	}
	return true;
}

TMap<uint8, int32> UInventorySubsystem::GetPlayerInventory()
{
	return PlayerInventory;
}
