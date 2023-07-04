// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingInterface.h"
#include "SaveData.h"
#include "SaveGameMetadata.h"
#include "Stats/Stats.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SavingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleteDelegate, FString, SlotSavedName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleteDelegate, FString, SlotLoadedName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotDeleted, FString, SlotDeletedName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveSlotsChanged);
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
	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnSaveCompleteDelegate OnSaveComplete;
	
	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnLoadCompleteDelegate OnLoadComplete;

	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnSaveSlotsChanged OnSaveSlotsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Saving and Loading")
	FOnSlotDeleted OnSlotDeleted;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Saving and Loading")
	FString CurrentSaveSlotName = "Default";

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Saving and Loading")
	bool LockSavingAndLoading = false;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Saving and Loading")
	int32 UserIndex;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Saving and Loading")
	int32 MaxSaveSlots = 50;


public: //functions
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//this is a function that will store a copy of every object that is going to want to save.
	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	bool QueryAllSaveInterfaces();

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	bool SaveGame(FString SlotName =  "Current");

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	bool LoadGame(FString SlotName = "Current");

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	bool DeleteSlot(FString SlotName = "Current");

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	FORCEINLINE void SetCurrentSaveSlot(FString Slot) {CurrentSaveSlotName = Slot;}

	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	bool GetNewUniqueSaveName(FString &NewSaveName);


	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	TArray<uint8> MakeSaveThumbnail();

	UFUNCTION(BlueprintPure, Category = "Saving and Loading")
	FORCEINLINE TArray<FSaveMetadata> GetAllSaveMetadata() {return SaveMetadataCache;}

	UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	UTexture2D* MakeTextureFromPixelByteArray(const TArray<uint8>& BGRA8PixelData);

		//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")
	//UFUNCTION(BlueprintCallable, Category = "Saving and Loading")

private: // functions
	TArray<FSaveMetadata> GetAllSaveMetadataFromMap(const TMap<FString, FSaveMetadata>& InMetadata);
	//helper function to always get a valid save game metadata file
	USaveGameMetadata* GetSaveGameMetadata();
	//UFUNCTION()
	//void AfterFrameQuerySaveInterfaces() { QueryAllSaveInterfaces(); }
};


#pragma region Async Saving


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncSavingTaskFinished);
UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API UAsyncSavingTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FOnAsyncSavingTaskFinished SaveFinished;
	
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

#pragma endregion


#pragma region Snapshot Capture

UCLASS(BlueprintType)
class CUSTOMSAVINGSUBSYSTEM_API ASnapCapture : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Transient)
	class USceneCaptureComponent2D* SceneCaptureComponent;
public:
	//initializer
	ASnapCapture(const FObjectInitializer& ObjectInitializer);

	//captures what the scene capture component is currently looking at
	UFUNCTION(BlueprintCallable, meta = (Category = "Snapshot Capture", ToolTip = "Capture the current scene capture's view.\n\nResolution - a power of 2 resolution for the view capture, like 512"))
    bool CaptureViewSnapshot(TArray<FColor>& ColorData, int32 Resolution = 512);

	UFUNCTION(BlueprintCallable, meta = (Category = "Snapshot Capture"))
	bool SetCaptureToPlayerCameraPosition();

	static const UTexture2D* RawPixelColorDataToTexture(TArray<FColor> RawPixels, int32 Resolution = 512);
};

#pragma endregion
