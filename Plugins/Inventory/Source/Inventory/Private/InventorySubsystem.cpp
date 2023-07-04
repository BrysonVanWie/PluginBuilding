// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"
#include "Kismet/KismetMathLibrary.h"

//void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//}
//
//void UInventorySubsystem::Deinitialize()
//{
//	Super::Deinitialize();
//}

bool UInventorySubsystem::AddItem(uint8 InItemEnum, int32 NumberToAdd)
{
	int32& NumberOfItem = PlayerInventory.FindOrAdd(InItemEnum);
	NumberOfItem += NumberToAdd;
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
	bool Success = true;
	for (const auto &Item : InItems)
	{
		if (!AddItem(Item.Key, Item.Value)) //add the item and check if it failed
			Success = false;
	}
	return Success;
}


#pragma region Saving and Loading
FString UInventorySubsystem::GetUniqueSaveName_Implementation()
{
	return "Inventory Subsystem";
}

void UInventorySubsystem::OnLoadedData_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Inventory was loaded "));
	OnInventoryChanged.Broadcast();
}
#pragma endregion