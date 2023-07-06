// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SavingSubsystem.h"
#include <GameplayTagContainer.h>
#include "TimeDilationControl.h"
#include "DialogueManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, FGameplayTag, DialogueTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueSessionStarted, FGameplayTag, DialogueTag);
UCLASS()
class QUESTTREESUBSYSTEM_API UDialogueManagerSubsystem : public UGameInstanceSubsystem, public ISavingInterface
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Manager Subsystem", SaveGame)
	TMap<FGameplayTag, bool> DialogueCompletionMap;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Manager Subsystem")
	FOnDialogueCompleted OnDialogueCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Manager Subsystem")
	FOnDialogueSessionStarted OnDialogueSessionStarted;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue Manager Subsystem")
	TArray<TScriptInterface<ITimeDilationControl>> TimeDilationInterfaces;
public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager Subsystem")
	void StartDialogueSession(FGameplayTag DialogueTag);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Manager Subsystem")
	void SetDialogueComplete(FGameplayTag DialogueTag);

	UFUNCTION(BlueprintPure, Category = "Dialogue Manager Subsystem")
	bool IsDialogueComplete(FGameplayTag DialogueTag);

public:
	virtual FString GetUniqueSaveName_Implementation() override;
};
