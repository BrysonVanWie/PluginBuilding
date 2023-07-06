// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TimeDilationControl.generated.h"


UENUM(BlueprintType, Blueprintable)
enum ETimeDilationReason
{
	CUTSCENE_STARTED UMETA(DisplayName = "Cutscene Started"),
	CUTSCENE_ENDED UMETA(DisplayName = "Cutscene Ended"),
	DIALOGUE_STARTED   UMETA(DisplayName = "Dialogue Started"),
	DIALOGUE_ENDED   UMETA(DisplayName = "Dialogue Ended"),
};
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTimeDilationControl : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class QUESTTREESUBSYSTEM_API ITimeDilationControl
{
	GENERATED_BODY()

public:
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	UFUNCTION(BlueprintNativeEvent, Category = "Time Dilation Control")
	void SetTimeDilation(ETimeDilationReason Reason, float NewTimeDilation);
};
