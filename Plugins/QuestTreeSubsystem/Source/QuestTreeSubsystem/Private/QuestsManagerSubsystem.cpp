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

bool UQuestsManagerSubsystem::RegisterObjective(FGameplayTag ObjectiveTagID, FObjectiveInfo InInfo)
{
	if (!IsObjectiveRegistered(ObjectiveTagID))
	{
		ObjectiveInformation.Add(ObjectiveTagID, InInfo);
		OnQuestsUpdated.Broadcast();
		OnObjectiveRegistered.Broadcast(ObjectiveTagID);
		OnObjectiveUpdated.Broadcast(ObjectiveTagID);
		return true;

	}
	return false;
}

bool UQuestsManagerSubsystem::UpdateObjectiveInfo(FGameplayTag ObjectiveTagID, FObjectiveInfo NewInfo)
{
	if (ObjectiveInformation.Contains(ObjectiveTagID))
	{
		ObjectiveInformation.Emplace(ObjectiveTagID, NewInfo);
		OnObjectiveUpdated.Broadcast(ObjectiveTagID);
		OnQuestsUpdated.Broadcast();
		return true;
	}
	return false;
}

bool UQuestsManagerSubsystem::SetObjectiveComplete(FGameplayTag ObjectiveTagID)
{
	if (ObjectiveInformation.Contains(ObjectiveTagID))
	{
		FObjectiveInfo ObjectiveInfo = ObjectiveInformation.FindRef(ObjectiveTagID);
		if (!ObjectiveInfo.IsComplete)
		{
			ObjectiveInfo.IsComplete = true;
			ObjectiveInformation.Emplace(ObjectiveTagID, ObjectiveInfo);
			OnObjectiveUpdated.Broadcast(ObjectiveTagID);
			OnObjectiveCompleted.Broadcast(ObjectiveTagID);
		}

		return true;
	}
	return false;
}

bool UQuestsManagerSubsystem::ActivateObjective(FGameplayTag ObjectiveTagID)
{
	if (ObjectiveInformation.Contains(ObjectiveTagID))
	{
		FObjectiveInfo ObjectiveInfo = ObjectiveInformation.FindRef(ObjectiveTagID);
		if (!ObjectiveInfo.IsActive)
		{
			ObjectiveInfo.IsActive = true;
			ObjectiveInformation.Emplace(ObjectiveTagID, ObjectiveInfo);
			OnObjectiveUpdated.Broadcast(ObjectiveTagID);
			OnObjectiveActivated.Broadcast(ObjectiveTagID);
		}
		return true;
	}
	return false;
}

bool UQuestsManagerSubsystem::GetObjectiveInfo(FGameplayTag ObjectiveTagID, FObjectiveInfo& OutObjectiveInfo)
{
	if (ObjectiveInformation.Contains(ObjectiveTagID))
	{
		OutObjectiveInfo = ObjectiveInformation.FindRef(ObjectiveTagID);
		return true;
	}
	return false;
}

TArray<FObjectiveInfo> UQuestsManagerSubsystem::GetAllObjectiveInfo()
{
	TArray<FObjectiveInfo> Out;
	for (auto& ObjectiveInfo : ObjectiveInformation)
	{
		Out.Push(ObjectiveInfo.Value);
	}
	return Out;
}

bool UQuestsManagerSubsystem::IsObjectiveRegistered(FGameplayTag ObjectiveIDTag)
{
	return ObjectiveInformation.Contains(ObjectiveIDTag);
}

TArray<FObjectiveInfo> UQuestsManagerSubsystem::GetAllObjectivesInfoUnderTag(FGameplayTag CollectionTagID)
{
	TArray<FObjectiveInfo> Out;
	for (auto& Objective : ObjectiveInformation)
	{
		if (Objective.Key.MatchesTag(CollectionTagID))
		{
			Out.Push(Objective.Value);
		}
	}
	return Out;
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
	for (auto& Objective : ObjectiveInformation)
	{
		if (Objective.Value.IsComplete)
		{
			Objective.Value.LoadedAsComplete = true;
		}
		else
		{
			Objective.Value.LoadedAsComplete = false;
		}
	}
	OnQuestsUpdated.Broadcast();
	OnQuestsLoaded.Broadcast();
}
#pragma endregion