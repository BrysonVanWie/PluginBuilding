// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingSubsystem.h"
#include "QuestsManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class QUESTTREESUBSYSTEM_API UQuestsManagerSubsystem : public UGameInstanceSubsystem, public ISavingInterface
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma region SavingAndLoading
public:
	virtual FString GetUniqueSaveName_Implementation() override;
	virtual void OnBeforeSave_Implementation() override;
	virtual void OnLoadedData_Implementation() override;
#pragma endregion


};
