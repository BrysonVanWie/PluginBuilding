// Fill out your copyright notice in the Description page of Project Settings.


#include "SavingSubsystem.h"
#include "kismet/GameplayStatics.h"

void USavingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USavingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USavingSubsystem::QueryAllSaveInterfaces()
{
	//clear all the old entries
	SaveInterfaces.Empty();

	TArray<UObject*> SaveInterfaceObjects;
	for (TObjectIterator<UObject> Object; Object; ++Object)
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Checking if object " + Object->GetName() + " has the save interface");
		//	UE_LOG(LogTemp, Warning, TEXT("Checking if actor %s implements the save interface"), *Object->GetName());
		//}
			
		if (Object->Implements<USavingInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Object %s implements the save interface"), *Object->GetName());
			SaveInterfaces.Add(*Object);
		}
	}

	//UGameInstance* GameInstance = GetGameInstance();
	//for (auto Subsystem : GameInstance->GetSubsystemArray<UGameInstanceSubsystem>())
	//{
	//	UObject* Object;
	//	Object = Cast<UObject>(Subsystem);
	//	if (Object == nullptr) return;
	//	if (GEngine)
	//		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(Object->GetName()));
	//	if (Object->Implements<USavingInterface>())
	//	{
	//		SaveInterfaces.Add(Object);
	//	}
	//}
}

void USavingSubsystem::SaveGame(FString SlotName)
{
	if (SlotName == "Current") SlotName = CurrentSaveSlotName;

	//create a new save data instance
	USaveData* SaveGameData = Cast<USaveData>(UGameplayStatics::CreateSaveGameObject(USaveData::StaticClass()));

	//go through all actors with the save interface and save them
	for (auto& SaveInterface : SaveInterfaces)
	{
		if (SaveInterface.GetObject() == nullptr)
		{
			continue;
		}

		//notify the object that it is about to be saved
		SaveInterface->Execute_OnBeforeSave(SaveInterface.GetObject());

		//get the unique name and create save data for the object
		FString UniqueSaveName = SaveInterface->Execute_GetUniqueSaveName(SaveInterface.GetObject());
		FSavedData& SaveData = SaveGameData->SerializedData.Add(UniqueSaveName);

		//actually serialize this object data
		FMemoryWriter MemWriter = FMemoryWriter(SaveData.Data);
		MemWriter.ArIsSaveGame = true; // only saving properties marked as ar is save game
		MemWriter.ArNoDelta = true; //Optional // Serialize variables even if weren't modified and mantain their default values.
									//most useful for saving / loading without resetting the level completely
		SaveInterface.GetObject()->Serialize(MemWriter);
	}

	//save the game to an actual file
	UGameplayStatics::SaveGameToSlot(SaveGameData, SlotName, UserIndex);

	//load the metadata file to update it
	USaveGameMetadata* SaveGameMetadata = Cast<USaveGameMetadata>(UGameplayStatics::LoadGameFromSlot(MetadataSaveSlot, UserIndex));

	//actually update the metadata file to reflect changes
	FSaveMetadata& ThisSaveMetadata = SaveGameMetadata->SaveGamesMetadata.FindOrAdd(SlotName);
	ThisSaveMetadata.SlotName = SlotName;
	ThisSaveMetadata.Date = FDateTime::Now();
	//clear the old thumbnail
	ThisSaveMetadata.BinaryThumbnailTexture.Empty();
	ThisSaveMetadata.BinaryThumbnailTexture = MakeSaveThumbnail(); // make a new thumbnail
	ThisSaveMetadata.SavedLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());

	//save the game metadata
	UGameplayStatics::SaveGameToSlot(SaveGameMetadata, MetadataSaveSlot, UserIndex);
	OnSaveComplete.Broadcast();
}


void USavingSubsystem::LoadGame(FString SlotName)
{
	if (SlotName == "Current") SlotName = CurrentSaveSlotName;
}



void USavingSubsystem::SetCurrentSaveSlot(FString Slot)
{
	CurrentSaveSlotName = Slot;
}

bool USavingSubsystem::GetNewSaveName(FString& NewSaveName)
{
	return false;
}

TArray<FString> USavingSubsystem::GetAllUniqueSaveNames()
{
	TArray<FString> Out;
	for (auto& SaveInterface : SaveInterfaces)
	{
		if (SaveInterface.GetObject() == nullptr)
		{
			continue;
		}
		Out.Add(SaveInterface->Execute_GetUniqueSaveName(SaveInterface.GetObject()));
	}
	return Out;
}

TArray<uint8> USavingSubsystem::MakeSaveThumbnail()
{
	return TArray<uint8>();
}
