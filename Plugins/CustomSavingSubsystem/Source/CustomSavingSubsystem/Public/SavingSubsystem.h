// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingInterface.h"
#include "SaveData.h"
#include "SaveGameMetadata.h"
#include "SavingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveCompleteDelegate);
/**
 * 
 */
UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API USavingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private: //properties
	TArray<TScriptInterface<ISavingInterface>> SaveInterfaces;
	const FString AutosaveSlotName;
	const FString MetadataSaveSlot;

public: // properties
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Saving and Loading")
	FString CurrentSaveSlotName;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Saving and Loading")
	int32 UserIndex;

	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnSaveCompleteDelegate OnSaveComplete;


public: //functions
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//this is a function that will store a copy of every object that is going to want to save.
	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	void QueryAllSaveInterfaces();

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	void SaveGame(FString SlotName =  "Current");

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	void LoadGame(FString SlotName = "Current");

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	void SetCurrentSaveSlot(FString Slot);
	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	bool GetNewSaveName(FString & NewSaveName);
	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	TArray<FString> GetAllUniqueSaveNames();


	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	TArray<uint8> MakeSaveThumbnail();
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")

};
