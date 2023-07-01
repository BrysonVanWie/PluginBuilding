// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameMetadata.generated.h"


USTRUCT(BlueprintType)
struct FSaveMetadata 
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElSavingAndLoading")
	FString SlotName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElSavingAndLoading")
	FDateTime Date;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElSavingAndLoading")
	TArray<uint8> BinaryThumbnailTexture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElSavingAndLoading")
	FString SavedLevelName;
};

UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API USaveGameMetadata : public USaveGame
{
	GENERATED_BODY()
	
public:
	TMap<FString, FSaveMetadata> SaveGamesMetadata;
};
