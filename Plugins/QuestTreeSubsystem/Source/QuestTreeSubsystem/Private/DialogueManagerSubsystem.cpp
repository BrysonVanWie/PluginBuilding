// Fill out your copyright notice in the Description page of Project Settings.

#include "kismet/GameplayStatics.h"
#include "DialogueManagerSubsystem.h"

void UDialogueManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDialogueManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UDialogueManagerSubsystem::StartDialogueSession(FGameplayTag DialogueTag)
{
	if (DialogueCompletionMap.Contains(DialogueTag))
	{
		if (!DialogueCompletionMap.FindRef(DialogueTag))
		{
			return;
		}
	}

	TimeDilationInterfaces.Empty();
	TArray<AActor*> TimeDilationActors;
	UGameplayStatics::GetAllActorsWithInterface(GetGameInstance()->GetWorld(), UTimeDilationControl::StaticClass(),TimeDilationActors);
	UE_LOG(LogTemp, Warning, TEXT("should have gotten the actors with time dilation control interface Num : %s"), *FString::FromInt(TimeDilationActors.Num()));
	for (AActor* TimeDilationActor : TimeDilationActors)
	{
		if (TimeDilationActor->Implements<UTimeDilationControl>())
		{
			TimeDilationInterfaces.Add(TimeDilationActor);
		}

	}

	for (auto& TimeDilationInterface : TimeDilationInterfaces)
	{
		if (TimeDilationInterface.GetObject() == nullptr)
			continue;
		TimeDilationInterface->Execute_SetTimeDilation(TimeDilationInterface.GetObject(), ETimeDilationReason::DIALOGUE_STARTED, 0.0);
	}
	DialogueCompletionMap.Add(DialogueTag, false);
	OnDialogueSessionStarted.Broadcast(DialogueTag);
}

void UDialogueManagerSubsystem::SetDialogueComplete(FGameplayTag DialogueTag)
{
	if (DialogueCompletionMap.Contains(DialogueTag))
	{
		if (DialogueCompletionMap.FindRef(DialogueTag))
		{
			return;
		}
	}

	for (auto& TimeDilationInterface : TimeDilationInterfaces)
	{
		if (TimeDilationInterface.GetObject() == nullptr)
			continue;
		TimeDilationInterface->Execute_SetTimeDilation(TimeDilationInterface.GetObject(), ETimeDilationReason::DIALOGUE_ENDED, 1.0);
	}
	TimeDilationInterfaces.Empty();
	DialogueCompletionMap.Add(DialogueTag, true);
	OnDialogueCompleted.Broadcast(DialogueTag);
}

bool UDialogueManagerSubsystem::IsDialogueComplete(FGameplayTag DialogueTag)
{
	if (DialogueCompletionMap.Contains(DialogueTag))
	{
		if (DialogueCompletionMap.FindRef(DialogueTag))
		{
			return true;
		}
	}
	return false;
}

FString UDialogueManagerSubsystem::GetUniqueSaveName_Implementation()
{
	return FString("DialogueManagerSubsystem");
}
