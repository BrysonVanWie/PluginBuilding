// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingSubsystem.h"
#include "GameplayTagContainer.h"
#include "QuestsManagerSubsystem.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestsUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestsLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveUpdated, FGameplayTag, UpdatedObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveRegistered, FGameplayTag, RegisteredObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, FGameplayTag, CompletedObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveActivated, FGameplayTag, ActivatedObjectiveID);
/**
 * 
 */

USTRUCT(BlueprintType, Blueprintable)
struct FObjectiveInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective Info", SaveGame)
	FString ObjectiveName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective Info", SaveGame)
	FGameplayTag ObjectiveIDTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective Info", SaveGame)
	bool IsComplete = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective Info", SaveGame)
	bool IsActive = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective Info", SaveGame)
	bool LoadedAsComplete = false;
};


UCLASS(BlueprintType, Blueprintable)
class QUESTTREESUBSYSTEM_API UQuestsManagerSubsystem : public UGameInstanceSubsystem, public ISavingInterface
{
	GENERATED_BODY()
public: //properties
	UPROPERTY(BlueprintAssignable, Category = "Quest Manager")
	FOnQuestsUpdated OnQuestsUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Quest Manager")
	FOnObjectiveUpdated OnObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest Manager")
	FOnObjectiveRegistered OnObjectiveRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Quest Manager")
	FOnObjectiveCompleted OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest Manager")
	FOnObjectiveActivated OnObjectiveActivated;

	UPROPERTY(BlueprintAssignable, Category = "Quest Manager")
	FOnQuestsLoaded OnQuestsLoaded;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Manager", SaveGame)
	TMap<FGameplayTag, FObjectiveInfo> ObjectiveInformation;


public://functions
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//only reregisters unregistered objectives. Returns if it successfully registered a new objective
	UFUNCTION(BlueprintCallable, Category = "Quest Manager")
	bool RegisterObjective(FGameplayTag ObjectiveTagID, FObjectiveInfo InInfo);

	//you can update any info about an objective using this function, this returns if it successfully found and edited an objectives info
	UFUNCTION(BlueprintCallable, Category = "Quest Manager")
	bool UpdateObjectiveInfo(FGameplayTag ObjectiveTagID, FObjectiveInfo NewInfo);

	//sets an objective to be complete if it was not, returns if the objective was registered
	UFUNCTION(BlueprintCallable, Category = "Quest Manager")
	bool SetObjectiveComplete(FGameplayTag ObjectiveTagID);

	//sets an objective to be active, returns if the objective was registered
	UFUNCTION(BlueprintCallable, Category = "Quest Manager")
	bool ActivateObjective(FGameplayTag ObjectiveTagID);

	//attempts to get the info about an objective, returns if the objective info was found.
	UFUNCTION(BlueprintPure, Category = "Quest Manager")
	bool GetObjectiveInfo(FGameplayTag ObjectiveTagID, FObjectiveInfo& OutObjectiveInfo);

	//returns information about every objective stored
	UFUNCTION(BlueprintPure, Category = "Quest Manager")
	TArray<FObjectiveInfo> GetAllObjectiveInfo();

	//checks if we have registered an objective into the objective information map
	UFUNCTION(BlueprintPure, Category = "Quest Manager")
	bool IsObjectiveRegistered(FGameplayTag ObjectiveIDTag);



	//all quests sub objectives ID's should be children of the quest tag ID
	UFUNCTION(BlueprintPure, Category = "Quest Manager")
	TArray<FObjectiveInfo> GetAllObjectivesInfoUnderTag(FGameplayTag CollectionTagID);


#pragma region SavingAndLoading
public:
	virtual FString GetUniqueSaveName_Implementation() override;
	virtual void OnBeforeSave_Implementation() override;
	virtual void OnLoadedData_Implementation() override;
#pragma endregion
};
