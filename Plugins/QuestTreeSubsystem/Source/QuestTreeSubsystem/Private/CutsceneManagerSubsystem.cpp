// Fill out your copyright notice in the Description page of Project Settings.


#include "kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "TimeDilationControl.h"
#include "CutsceneManagerSubsystem.h"

void UCutsceneManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
    if (GameInstance != nullptr)
    {
        SavingSubsystem = GameInstance->GetSubsystem<USavingSubsystem>();
        if (SavingSubsystem != nullptr)
        {
            SavingSubsystem->OnLoadComplete.AddDynamic(this, &UCutsceneManagerSubsystem::GameLoaded);
        }
    }
}

bool UCutsceneManagerSubsystem::PlayCutscene(ULevelSequence* CutsceneToPlay, FCutsceneParameters Params)
{
    if (PlayingSequence)
    {
        if (PlayingSequence->IsPlaying())
            return false;
    }
    
    PlayingSequence = ULevelSequencePlayer::CreateLevelSequencePlayer(this, CutsceneToPlay, Params.PlaybackSettings, SequenceActor);

    PlayingSequence->Play();
    PlayedCutscenesMap.Add(Params.CutsceneIDTag, true);
    PlayingSequence->OnFinished.AddDynamic(this, &UCutsceneManagerSubsystem::CutsceneFinished);

    if (Params.PauseActors)
    {
        TimeDilationInterfaces.Empty();
        TArray<AActor*> TimeDilationActors;
        UGameplayStatics::GetAllActorsWithInterface(this, UTimeDilationControl::StaticClass(), TimeDilationActors);
        for (AActor* TimeActor : TimeDilationActors)
        {
            if (TimeActor == nullptr) continue;
            ITimeDilationControl* TimeInterface = Cast<ITimeDilationControl>(TimeActor);
            TimeDilationInterfaces.Add(TimeActor);
        }

        for (auto& TimeInterface : TimeDilationInterfaces)
        {
            if (TimeInterface.GetObject() == nullptr)
                continue;
            TimeInterface->Execute_SetTimeDilation(TimeInterface.GetObject(), ETimeDilationReason::CUTSCENE_STARTED, 0.0);
        }
        DidPauseActors = true;
    }

    if (!Params.Skippable) return true;
    if (Params.InputContext)
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (PlayerController)
        {
            //UE_LOG(LogTemp, Warning, TEXT("we successfully got the player controller 1/4"))
            if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player))
            {
                //UE_LOG(LogTemp, Warning, TEXT("we successfully got the LocalPlayer 2/4"))
                InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
                if (InputSystem)
                {
                    //UE_LOG(LogTemp, Warning, TEXT("we successfully got the InputSystem 3/4"))
                    InputSystem->AddMappingContext(Params.InputContext, 0);
                    PreviousContext = Params.InputContext;
                    UEnhancedInputComponent* PlayerInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
                    if (PlayerInputComponent)
                    {
                        PlayerInputComponent->BindAction(Params.SkipInputAction, ETriggerEvent::Started, this, &UCutsceneManagerSubsystem::SkipButtonPressed);
                        //UE_LOG(LogTemp, Warning, TEXT("we successfully bound to the action to skip cutscenesS"))
                    }
                }
            }
        }
    }
    return true;
}

bool UCutsceneManagerSubsystem::HasPlayedCutscene(FGameplayTag InCutsceneTag)
{
    if (PlayedCutscenesMap.Contains(InCutsceneTag))
    {
        return PlayedCutscenesMap.FindRef(InCutsceneTag);
    }
    return false;
}

void UCutsceneManagerSubsystem::SkipButtonPressed()
{
    if (PlayingSequence)
    {
        if (PlayingSequence->IsPlaying())
        {
            PlayingSequence->GoToEndAndStop();
            CutsceneFinished();
        }
    }
}

void UCutsceneManagerSubsystem::CutsceneFinished()
{
    PlayingSequence->OnFinished.RemoveDynamic(this, &UCutsceneManagerSubsystem::CutsceneFinished);
    if (DidPauseActors)
    {
        for (auto& TimeInterface : TimeDilationInterfaces)
        {
            if (TimeInterface.GetObject() == nullptr)
                continue;
            TimeInterface->Execute_SetTimeDilation(TimeInterface.GetObject(), ETimeDilationReason::CUTSCENE_ENDED, 1.0);
        }
        TimeDilationInterfaces.Empty();
        DidPauseActors = false;
    }
    OnCutsceneFinished.Broadcast();
    PlayingSequence = nullptr;

    if (InputSystem)
        if (PreviousContext)
        {
            InputSystem->RemoveMappingContext(PreviousContext);
            //UE_LOG(LogTemp, Warning, TEXT("REMOVED THE SKIPPING INPUT MAP CONTEXT"))
        }
        else 
        {
            //UE_LOG(LogTemp, Warning, TEXT("FAO:ED REMOVED THE SKIPPING INPUT MAP CONTEXT")

        }
}

void UCutsceneManagerSubsystem::GameLoaded(FString LoadedFile)
{
    if (PlayingSequence)
    {
        PlayingSequence->GoToEndAndStop();
    }
    if (DidPauseActors)
    {
        for (auto& TimeInterface : TimeDilationInterfaces)
        {
            if (TimeInterface.GetObject() == nullptr)
                continue;
            TimeInterface->Execute_SetTimeDilation(TimeInterface.GetObject(), ETimeDilationReason::CUTSCENE_ENDED, 1.0);
        }
        TimeDilationInterfaces.Empty();
        DidPauseActors = false;
    }
}

FString UCutsceneManagerSubsystem::GetUniqueSaveName_Implementation()
{
    return FString("CutsceneManagerSubsystem");
}
