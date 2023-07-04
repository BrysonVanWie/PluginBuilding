// Fill out your copyright notice in the Description page of Project Settings.


#include "SavingSubsystem.h"
#include "TimerManager.h"
#include "kismet/GameplayStatics.h"

void USavingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//make sure we have a metadata file in case the game has never been ran
	USaveGameMetadata* SaveGameMetadata = GetSaveGameMetadata();
	SaveMetadataCache = GetAllSaveMetadataFromMap(SaveGameMetadata->SaveGamesMetadata);

	//query all save interfaces to maybe get a few
	QueryAllSaveInterfaces();
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

bool USavingSubsystem::QueryAllSaveInterfaces()
{
	if (LockSavingAndLoading) return false;
	//clear all the old entries
	SaveInterfaces.Empty();

	TArray<UObject*> SaveInterfaceObjects;
	for (TObjectIterator<UObject> Object; Object; ++Object)
	{
			
		if (Object->Implements<USavingInterface>())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Object %s implements the save interface"), *Object->GetName());
			if (Object->GetName().Contains("REINST_"))//checks if this is a reisnt object
																//only really an issue in editor with hot realoading
			{
				UE_LOG(LogTemp, Warning, TEXT("Object %s was not added to save interface because it was a reinst"), *Object->GetName())
				continue;
			}
			SaveInterfaces.Add(*Object);
		}
	}
	return true;
}

bool USavingSubsystem::SaveGame(FString SlotName)
{
	if (LockSavingAndLoading) return false; // double check that we can currently save and load
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
	USaveGameMetadata* SaveGameMetadata = GetSaveGameMetadata();
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
	OnSaveComplete.Broadcast(SlotName);
	OnSaveSlotsChanged.Broadcast();
	return true;
}


bool USavingSubsystem::LoadGame(FString SlotName)
{
	if (LockSavingAndLoading) return false; // double check that we can currently save and load
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

	OnLoadComplete.Broadcast(SlotName);
	return true;
}

bool USavingSubsystem::DeleteSlot(FString SlotName)
{
	if (LockSavingAndLoading) return false; // double check that we can currently save and load
	if (SlotName == "Current") SlotName = CurrentSaveSlotName;

	//delete the file for the slot
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

	//load and update the metadata file
	USaveGameMetadata* SaveMetadata = GetSaveGameMetadata();
	SaveMetadata->SaveGamesMetadata.Remove(SlotName); // deleting this slot from our list
	UGameplayStatics::SaveGameToSlot(SaveMetadata, MetadataSaveSlot, UserIndex); // save the updated metadata
	SaveMetadataCache = GetAllSaveMetadataFromMap(SaveMetadata->SaveGamesMetadata); //updatte our cache

	OnSlotDeleted.Broadcast(SlotName);
	OnSaveSlotsChanged.Broadcast();
	return true;
}

bool USavingSubsystem::GetNewUniqueSaveName(FString& NewSaveName)
{
	int32 NumSaveSlots = GetAllSaveMetadata().Num();
	if (NumSaveSlots < MaxSaveSlots)
	{
		NewSaveName = "Save Slot " + FString::FromInt(NumSaveSlots);
		return true;
	}
	return false;
}


TArray<uint8> USavingSubsystem::MakeSaveThumbnail()
{
	TArray<uint8> Out;
	if (GetWorld())
	{
		ASnapCapture* SnapCaptureActor = GetWorld()->SpawnActor<ASnapCapture>();
		if (SnapCaptureActor != nullptr)
		{
			if (SnapCaptureActor->SetCaptureToPlayerCameraPosition())
			{
				TArray<FColor> ColorArray;
				if (SnapCaptureActor->CaptureViewSnapshot(ColorArray))
				{
					if (!ColorArray.IsEmpty())
					{
						// 5. Memcpy data of our color array to our BinaryTexture byte array.
						// Calculate the total number of bytes we will copy. Every color is represented by 4 bytes: R, G, B, A.
						const int32 BufferSize = ColorArray.Num() * 4;
						// Pre-allocate enough memory to fit our data. We reserve space before adding uninitialized elements to avoid array 
						// growth operations and we add uninitialized elements to increase array element count properly.
						Out.Reserve(BufferSize);
						Out.AddUninitialized(BufferSize);

						// Copy BufferSize number of bytes starting from the memory address where ColorArray's bulk data starts,
						// to a space in memory starting from the memory address where BinaryTexture's bulk data starts.

						FMemory::Memcpy(Out.GetData(), ColorArray.GetData(), BufferSize);
					}
				}
			}
			SnapCaptureActor->Destroy();
		}
	}
	return Out;
}

UTexture2D* USavingSubsystem::MakeTextureFromPixelByteArray(const TArray<uint8>& BGRA8PixelData)
{
	if (BGRA8PixelData.IsEmpty())
	{
		return nullptr;
	}

	// 1. Create a new texture of the right size, and get reference to the first MIP for convenience.
	// Calculate the resolution from a number of pixels in our array (bytes / 4).
	const float Resolution = FMath::Sqrt(float(BGRA8PixelData.Num() / 4));

	// Create a new transient UTexture2D in a desired pixel format for byte order: B, G, R, A.
	UTexture2D* Texture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);

	// Get a reference to MIP 0, for convenience.
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];

	// 2. Memcpy operation.
	// Calculate the number of bytes we will copy.
	const int32 BufferSize = BGRA8PixelData.Num();

	// Mutex lock the MIP's data, not letting any other thread read or write it now.
	void* MipBulkData = Mip.BulkData.Lock(LOCK_READ_WRITE);

	// Pre-allocate enough space to copy our bytes into the MIP's bulk data. 
	Mip.BulkData.Realloc(BufferSize);

	// Copy BufferSize number of bytes starting from BGRA8PixelData's bulk data address in memory to
	// a block of memory starting from the memory address MipBulkData.
	FMemory::Memcpy(MipBulkData, BGRA8PixelData.GetData(), BufferSize);

	// Mutex unlock the MIP's data, letting all other threads read or lock for writing.
	Mip.BulkData.Unlock();

	// 3. Let the engine process new data.
	Texture->UpdateResource();

	return Texture;
}


USaveGameMetadata* USavingSubsystem::GetSaveGameMetadata()
{
	USaveGameMetadata* SaveGameMetadata = Cast<USaveGameMetadata>(UGameplayStatics::LoadGameFromSlot(MetadataSaveSlot, UserIndex));

	if (SaveGameMetadata == nullptr) // create metadata
	{
		SaveGameMetadata = Cast<USaveGameMetadata>(UGameplayStatics::CreateSaveGameObject(USaveGameMetadata::StaticClass()));
	}
	return SaveGameMetadata;
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
	SaveSubsystem->LockSavingAndLoading = true;
	SaveSubsystem->SaveGame(SaveSlotName);
	SaveSubsystem->LockSavingAndLoading = false;
	SaveFinished.Broadcast();
}
#pragma endregion

#pragma region Snapshot Capture
ASnapCapture::ASnapCapture(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false; // this actor can never tick
	//creating the scene capture component
	SceneCaptureComponent = ObjectInitializer.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("Scene Capture Component"));
	SetRootComponent(SceneCaptureComponent);
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
}
bool ASnapCapture::CaptureViewSnapshot(TArray<FColor>& ColorData, int32 Resolution)
{
	//Make the resolution a power of two.
	Resolution = FMath::FloorLog2(FMath::Max(Resolution, 1) * 2 - 1);
	Resolution *= Resolution;

	// Create a temporary object that we will let die in GC in a moment after this scope ends.
	UTextureRenderTarget2D* TextureRenderTarget = NewObject<UTextureRenderTarget2D>();
	TextureRenderTarget->InitCustomFormat(Resolution, Resolution, PF_B8G8R8A8, false);

	//take the capture
	SceneCaptureComponent->TextureTarget = TextureRenderTarget;
	SceneCaptureComponent->CaptureScene();
	
	//output this capture to a pixel array
	ColorData.Empty();
	ColorData.Reserve(Resolution * Resolution);
	TextureRenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(ColorData);
	ColorData.Shrink();

	return true;
}
bool ASnapCapture::SetCaptureToPlayerCameraPosition()
{
	APlayerCameraManager* PlayerCamera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!IsValid(PlayerCamera)) return false;

	const FVector CamLocation = PlayerCamera->GetCameraLocation();
	const FRotator CamRotation = PlayerCamera->GetCameraRotation();

	SetActorLocationAndRotation(CamLocation, CamRotation);

	SceneCaptureComponent->FOVAngle = PlayerCamera->GetFOVAngle();

	return true;
}
const UTexture2D* ASnapCapture::RawPixelColorDataToTexture(TArray<FColor> RawPixels, int32 Resolution)
{
	//Make the resolution a power of two.
	Resolution = FMath::FloorLog2(FMath::Max(Resolution, 1) * 2 - 1);
	Resolution *= Resolution;
	if (RawPixels.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to convert pixels into a texture but the pixel array was empty"));
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	const int32 BufferSize = RawPixels.Num() * 4;


	TArray<uint8> DataBuffer;
	DataBuffer.Reserve(BufferSize);
	for (int32 i = 0, Max = RawPixels.Num(); i < Max; i++)
	{
		DataBuffer.Append({ RawPixels[i].B, RawPixels[i].G, RawPixels[i].R, 0xff });
	}

	// Memcpy operation.
	void* DataBufferData = DataBuffer.GetData();
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	Mip.BulkData.Realloc(BufferSize);
	FMemory::Memcpy(Data, DataBufferData, BufferSize);
	Mip.BulkData.Unlock();
	// Let the engine process new data.
	Texture->UpdateResource();

	return Texture;
}
#pragma endregion