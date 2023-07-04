// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SavingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class USavingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CUSTOMSAVINGSUBSYSTEM_API ISavingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//this should return a unique save name to use when saving
	UFUNCTION(BlueprintNativeEvent, Category = "Saving and Loading")
	FString GetUniqueSaveName();

	//called right before the owning actor is saved.
	//usually used to save last minute variables e.g. player positions
	UFUNCTION(BlueprintNativeEvent, Category = "Saving and Loading")
	void OnBeforeSave();


	//called when an objects data has been loaded.
	UFUNCTION(BlueprintNativeEvent, Category = "Saving and Loading")
	void OnLoadedData();
};
