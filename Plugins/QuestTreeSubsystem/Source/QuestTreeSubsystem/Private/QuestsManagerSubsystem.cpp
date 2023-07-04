// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestsManagerSubsystem.h"

void UQuestsManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UQuestsManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

#pragma region Saving and Loading
FString UQuestsManagerSubsystem::GetUniqueSaveName_Implementation()
{
	return "QuestManagerSubsystem";
}

void UQuestsManagerSubsystem::OnBeforeSave_Implementation()
{
}

void UQuestsManagerSubsystem::OnLoadedData_Implementation()
{
}
#pragma endregion