// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"
#include "Kismet/KismetMathLibrary.h"

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
FString UInventorySubsystem::GetInventoryAsString()
{
	FString Out;
	for (auto &Item : PlayerInventory)
	{
		//format is ItemByteValue-Number,
		//NOTE:: bytes to string adds +1 to the byte value
		int32 KeyInt;
		KeyInt = UKismetMathLibrary::Conv_ByteToInt(Item.Key);
		Out = Out + FString::FromInt(KeyInt)+ "-" + *FString::FromInt(Item.Value) + ",\n";

	}
	return Out;
}
TMap<uint8, int32> UInventorySubsystem::GetInventoryFromString(const FString &InString)
{
	TMap<uint8, int32> Out;
	//TArray<FS
	//for (const FString Char : InString.ParseIntoArrayLines())
	//{
	//	FString
	//	TCHAR::
	//	if (GEngine)
	//		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Orange, TEXT(Char.));
	//}
	return Out;
}
#pragma region Saving and Loading


FString UInventorySubsystem::GetUniqueSaveName_Implementation()
{
	return "Inventory Subsystem";
}

void UInventorySubsystem::OnBeforeSave_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Orange, "Inventory on save" + GetInventoryAsString());
}

void UInventorySubsystem::OnLoadedData_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Orange, "Inventory subsystem loaded " + GetInventoryAsString());
}

#pragma endregion