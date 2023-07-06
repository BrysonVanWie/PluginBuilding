// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LevelSequencePlayer.h"
#include "InputAction.h"
#include "MovieSceneSequencePlayer.h"
#include <EnhancedInputSubsystems.h>
#include "SavingSubsystem.h"
#include "GameplayTagContainer.h"
#include "TimeDilationControl.h"
#include "CutsceneManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCutsceneFinished);

USTRUCT(BlueprintType, Blueprintable)
struct FCutsceneParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Skippable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool PauseActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* SkipInputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputMappingContext* InputContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CutsceneIDTag;
};

UCLASS(BlueprintType)
class QUESTTREESUBSYSTEM_API UCutsceneManagerSubsystem : public UWorldSubsystem, public ISavingInterface
{
	GENERATED_BODY()
private:
	USavingSubsystem* SavingSubsystem;
	bool DidPauseActors = false;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Cutscene Manager")
	TArray<TScriptInterface<ITimeDilationControl>> TimeDilationInterfaces;
	UPROPERTY(BlueprintReadOnly, Category = "Cutscene Manager", SaveGame)
	TMap<FGameplayTag, bool> PlayedCutscenesMap;
	UPROPERTY(BlueprintAssignable, Category = "Cutscene Manager")
	FOnCutsceneFinished OnCutsceneFinished;

	UPROPERTY(BlueprintReadOnly, Category = "Cutscene Manager") 
	UInputMappingContext* PreviousContext;
	UPROPERTY(BlueprintReadOnly, Category = "Cutscene Manager") 
	UEnhancedInputLocalPlayerSubsystem* InputSystem;
	UPROPERTY(BlueprintReadOnly, Category = "Cutscene Manager") 
	ALevelSequenceActor* SequenceActor;
	UPROPERTY(BlueprintReadOnly, Category = "Cutscene Manager") 
	ULevelSequencePlayer* PlayingSequence;

	UFUNCTION(BlueprintCallable, Category = "Cutscene Manager")
	bool PlayCutscene(ULevelSequence* CutsceneToPlay, FCutsceneParameters Params);

	UFUNCTION(BlueprintPure, Category = "Cutscene Manager")
	bool HasPlayedCutscene(FGameplayTag InCutsceneTag);

	UFUNCTION(BlueprintCallable, Category = "Cutscene Manager")
	void SkipButtonPressed();
	UFUNCTION(Category = "Cutscene Manager")
	void CutsceneFinished();
	UFUNCTION(Category = "Cutscene Manager")
	void GameLoaded(FString LoadedFile);


	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual FString GetUniqueSaveName_Implementation() override;
};
