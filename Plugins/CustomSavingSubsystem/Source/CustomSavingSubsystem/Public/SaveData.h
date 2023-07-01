// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveData.generated.h"

USTRUCT()
struct FSavedData
{
	GENERATED_BODY()

		UPROPERTY(VisibleAnywhere)
		TArray<uint8> Data;
};

UCLASS()
class CUSTOMSAVINGSUBSYSTEM_API USaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FSavedData> SerializedData;
};
