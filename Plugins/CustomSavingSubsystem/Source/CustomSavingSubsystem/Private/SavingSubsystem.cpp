// Fill out your copyright notice in the Description page of Project Settings.


#include "SavingSubsystem.h"

#include "kismet/GameplayStatics.h"

void USavingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//make sure we have a metadata file in case the game has never been ran
	USaveGameMetadata* SaveGameMetadata = Cast<USaveGameMetadata>(UGameplayStatics::LoadGameFromSlot(MetadataSaveSlot, UserIndex));

	if (SaveGameMetadata == nullptr) // create metadata
	{
		SaveGameMetadata = Cast<USaveGameMetadata>(UGameplayStatics::CreateSaveGameObject(USaveGameMetadata::StaticClass()));
		UGameplayStatics::SaveGameToSlot(SaveGameMetadata, MetadataSaveSlot, UserIndex);
	}
	SaveMetadataCache = GetAllSaveMetadataFromMap(SaveGameMetadata->SaveGamesMetadata);
}

TArray<FSaveMetadata> USavingSubsystem::GetAllSaveMetadataFromMap(const TMap<FString, FSaveMetadata>& InMetadata)
{
	TArray<FSaveMetadata> Out;
	Out.Reserve(InMetadata.Num());

	//add all the save meta to the array
	for (const auto& Metadata : InMetadata)
	{
		Out.Push(Metadata.Value);
	}

	return Out;
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
			if (Object->GetName().RemoveFromStart("REINST_")) //checks if this is a reisnt object
																//only really an issue in editor with hot realoading
			{
				UE_LOG(LogTemp, Warning, TEXT("Object %s was not added to save interface because it was a reinst"), *Object->GetName())
				continue;
			}
			SaveInterfaces.Add(*Object);
		}
	}
}

void USavingSubsystem::SaveGame(FString SlotName)
{
	if (SlotName == "Current") SlotName = CurrentSaveSlotName;

	//create a new save data instance
	USaveData* SaveGameData = Cast<USaveData>(UGameplayStatics::CreateSaveGameObject(USaveData::StaticClass()));

	//go through all actors with the save interface and save them
	for (auto& SaveInterface : SaveInterfaces)
	{
		//if (SaveInterface == nullptr)
		//{
		//	continue;
		//}
		if (SaveInterface.GetObject() == nullptr)
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("Object %s is about to be svaed"), *SaveInterface.GetObject()->GetName());
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
	if (SaveGameMetadata == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TRIED TO LOAD METADATA BUT IT WAS NULL"))
		UE_LOG(LogTemp, Warning, TEXT("TRIED TO LOAD METADATA BUT IT WAS NULL"))
		return;
	}
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
	SaveMetadataCache = GetAllSaveMetadataFromMap(SaveGameMetadata->SaveGamesMetadata); //storing a cached copy of the metadata
	OnSaveComplete.Broadcast();
}


void USavingSubsystem::LoadGame(FString SlotName)
{
	if (SlotName == "Current") SlotName = CurrentSaveSlotName;
	//load the data from file
	USaveData* SaveGameData = Cast<USaveData>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

	//double check that we even have a slot to load in this file?
	if (SaveGameData == nullptr)
	{
		SaveGame(SlotName); // just create a new save in the slot
		//reload the newly saved slot
		SaveGameData = Cast<USaveData>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}

	//implant data on those that need to be loaded
	for (auto& SaveInterface : SaveInterfaces)
	{
		if (SaveInterface.GetObject() == nullptr)
		{
			continue;
		}

		FString UniqueSaveName = SaveInterface->Execute_GetUniqueSaveName(SaveInterface.GetObject());
		//double check the save name is valid
		if (UniqueSaveName == "") continue;

		FSavedData* ObjectData = SaveGameData->SerializedData.Find(UniqueSaveName); // get the objects saved data
		//double check that the save data is valid
		if (ObjectData == nullptr) continue;

		//deserialize the data into the object
		FMemoryReader MemReader(ObjectData->Data); // create a memory writer
		MemReader.ArIsSaveGame = true; //only load data from variables tagged as save game
		MemReader.ArNoDelta = true; //Optional // Serialize variables even if weren't modified and load their saved values.
									//most useful for saving / loading without resetting the level completely

		//actually deserialize the data into the object
		SaveInterface.GetObject()->Serialize(MemReader);
		SaveInterface->Execute_OnLoadedData(SaveInterface.GetObject()); //notify the object that it was loaded
	}

	OnLoadComplete.Broadcast();
}

void USavingSubsystem::DeleteSlot(FString SlotName)
{
	if (SlotName == "Current") SlotName = CurrentSaveSlotName;

	//delete the file for the slot
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

	//load and update the metadata file
	USaveGameMetadata* SaveMetadata = Cast< USaveGameMetadata>(UGameplayStatics::LoadGameFromSlot(MetadataSaveSlot, UserIndex));
	SaveMetadata->SaveGamesMetadata.Remove(SlotName); // deleting this slot from our list
	UGameplayStatics::SaveGameToSlot(SaveMetadata, MetadataSaveSlot, UserIndex); // save the updated metadata
	SaveMetadataCache = GetAllSaveMetadataFromMap(SaveMetadata->SaveGamesMetadata); //updatte our cache
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
		FString UniqueSaveName = SaveInterface->Execute_GetUniqueSaveName(SaveInterface.GetObject());
		Out.Add(UniqueSaveName);
	}
	return Out;
}

TArray<uint8> USavingSubsystem::MakeSaveThumbnail()
{
	TArray<uint8> Out;
	return Out;
}

TArray<FSaveMetadata> USavingSubsystem::GetAllSaveMetadata()
{
	return SaveMetadataCache;
}



#pragma region Async Saving

UAsyncSavingTask* UAsyncSavingTask::AsyncSaveGame(const UObject* WorldContextObject, FString SlotName)
{
	UAsyncSavingTask* MyTask = NewObject<UAsyncSavingTask>();
	MyTask->SaveSlotName = SlotName;
	MyTask->WorldContextObject = WorldContextObject;
	return MyTask;
}

void UAsyncSavingTask::Activate()
{
	if (WorldContextObject == nullptr)
	{
		return;
	}
	if (Active)
	{
		UE_LOG(LogTemp, Warning, TEXT("TRIED running async save task again but it is already acttive"))
		return;
	}
	Active = true;
	USavingSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<USavingSubsystem>();
	if (SaveSubsystem == nullptr)
	{
		Active = false;
		UE_LOG(LogTemp, Warning, TEXT("async task failed because save subsystem was null"))
		return;
	}
	SaveSubsystem->SaveGame(SaveSlotName);
	SaveFinished.Broadcast();
}


#pragma endregion

