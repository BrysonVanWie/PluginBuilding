// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingSubsystem.h"
#include "InventorySubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChangedDelegate);

USTRUCT(BlueprintType)
struct FItemInfo
{
	GENERATED_BODY()
		UPROPERTY(BlueprintReadWrite, Category = "Inventory Subsystem")
		FString ItemName;
};


UCLASS(BlueprintType)
class INVENTORY_API UInventorySubsystem : public UGameInstanceSubsystem, public ISavingInterface
{
	GENERATED_BODY()
	
public: // properties
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Inventory Subsystem")
		FOnInventoryChangedDelegate OnInventoryChanged;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Inventory Subsystem", SaveGame)
	TMap<uint8, int32> PlayerInventory;
public: //functions
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory Subsystem")
	bool AddItem(uint8 InItemEnum, int32 NumberToAdd);

	UFUNCTION(BlueprintCallable, Category = "Inventory Subsystem")
	bool HasItem(uint8 InItemEnum, int32 NumberNeeded = 1);
	UFUNCTION(BlueprintCallable, Category = "Inventory Subsystem")
	bool AddManyItems(TMap<uint8,int32> InItems);
	UFUNCTION(BlueprintCallable, Category = "Inventory Subsystem")
	TMap<uint8, int32> GetPlayerInventory();

	//this will return your current inventory as a string, however it will not be a very readable string
	UFUNCTION(BlueprintPure, Category = "Inventory Subsystem")
	FString GetInventoryAsString();
	UFUNCTION(BlueprintPure, Category = "Inventory Subsystem")
	TMap<uint8, int32> GetInventoryFromString(const FString& InString);

#pragma region Saving and Loading
	virtual FString GetUniqueSaveName_Implementation() override;
	virtual void OnBeforeSave_Implementation() override;
	virtual void OnLoadedData_Implementation() override;
#pragma endregion
};