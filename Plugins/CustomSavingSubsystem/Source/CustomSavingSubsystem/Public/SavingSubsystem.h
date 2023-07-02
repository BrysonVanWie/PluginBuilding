// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingInterface.h"
#include "SaveData.h"
#include "SaveGameMetadata.h"
#include "Stats/Stats.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SavingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveCompleteDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadCompleteDelegate);
/**
 *  This is a pretty big manager class that handles saving
 * to reduce the amounnt of header comments on classes i plan to save what everything does here
 *	- SaveGameMetadata
 *		- This stores data about save files (File name, Date saved, ect.)
 *		- We create one save metadata save file to store this information to the disk.
 *  -  SaveData
 *		- this is the save game object that stores every saving objects game data
 * 
 */
UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API USavingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private: //properties
	TArray<TScriptInterface<ISavingInterface>> SaveInterfaces;
	const FString AutosaveSlotName = "Autosaves";
	const FString MetadataSaveSlot = "SaveGamesMetadata";
	TArray<FSaveMetadata> SaveMetadataCache;

public: // properties
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Saving and Loading")
	FString CurrentSaveSlotName = "Default";
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Saving and Loading")
	int32 UserIndex;

	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnSaveCompleteDelegate OnSaveComplete;
	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnLoadCompleteDelegate OnLoadComplete;


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
	void DeleteSlot(FString SlotName = "Current");

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	void SetCurrentSaveSlot(FString Slot);
	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	bool GetNewSaveName(FString & NewSaveName);
	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	TArray<FString> GetAllUniqueSaveNames();


	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	TArray<uint8> MakeSaveThumbnail();

	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	TArray<FSaveMetadata> GetAllSaveMetadata();
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")

private: // functions
	TArray<FSaveMetadata> GetAllSaveMetadataFromMap(const TMap<FString, FSaveMetadata>& InMetadata);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncSavingTaskFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncSavedObject, FString, SavedObjectName);
UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API UAsyncSavingTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FOnAsyncSavingTaskFinished SaveFinished;

	UPROPERTY(BlueprintAssignable)
	FOnAsyncSavedObject SavedObject;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Saving and Loading")
	static UAsyncSavingTask* AsyncSaveGame(const UObject* WorldContextObject, FString SlotName = "Current");

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~UBlueprintAsyncActionBase interface

private:
	FString SaveSlotName;
	const UObject* WorldContextObject;
	bool Active;
	/*...*/
};