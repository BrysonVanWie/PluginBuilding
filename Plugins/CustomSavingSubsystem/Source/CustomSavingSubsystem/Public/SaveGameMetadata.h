// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameMetadata.generated.h"

/// <summary>
/// FOR MORE INFORMATION SEE SAVE SUBSYSTEM 
/// </summary>
USTRUCT(BlueprintType)
struct FSaveMetadata 
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saving and Loading")
	FString SlotName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saving and Loading")
	FDateTime Date;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saving and Loading")
	TArray<uint8> BinaryThumbnailTexture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saving and Loading")
	FString SavedLevelName;
};

UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API USaveGameMetadata : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Saving and Loading")
	TMap<FString, FSaveMetadata> SaveGamesMetadata;
};
