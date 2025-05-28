#include "SFXManager.h"
#include "Kismet/GameplayStatics.h"

USFXManager::USFXManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    SoundMap = nullptr; // Will be initialized in BeginPlay
    BackgroundMusicComponent = nullptr;
    PreloadedWaypointSound = nullptr;
    PreloadedCheckpointSound = nullptr;
    PreloadedCrashSound = nullptr;
    PreloadedLapSound = nullptr;
	PreloadedEngineSound = nullptr;

    // Use FObjectFinder in constructor to pre-load assets
    static ConstructorHelpers::FObjectFinder<USoundBase> WaypointSound(TEXT("/Game/Assets/Sound/ding-36029"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CheckpointSound(TEXT("/Game/Assets/Sound/collect-points-190037"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CrashSound(TEXT("/Game/Assets/Sound/box-crash-106687"));
    static ConstructorHelpers::FObjectFinder<USoundBase> LapSound(TEXT("/Game/Assets/Sound/yay-6120"));
	static ConstructorHelpers::FObjectFinder<USoundBase> EngineSound(TEXT("/Game/Assets/Sound/motor-loop-83480.motor-loop-83480")); // Example path for engine sound

    // Store pre-loaded assets with logging
    if (WaypointSound.Succeeded())
    {
        PreloadedWaypointSound = WaypointSound.Object;
        UE_LOG(LogTemp, Log, TEXT("USFXManager: Preloaded waypoint sound: %s"), *PreloadedWaypointSound->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: Failed to preload waypoint sound at /Game/Assets/Sound/ding-36029"));
    }

    if (CheckpointSound.Succeeded())
    {
        PreloadedCheckpointSound = CheckpointSound.Object;
        UE_LOG(LogTemp, Log, TEXT("USFXManager: Preloaded checkpoint sound: %s"), *PreloadedCheckpointSound->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: Failed to preload checkpoint sound at /Game/Assets/Sound/collect-points-190037"));
    }

    if (CrashSound.Succeeded())
    {
        PreloadedCrashSound = CrashSound.Object;
        UE_LOG(LogTemp, Log, TEXT("USFXManager: Preloaded crash sound: %s"), *PreloadedCrashSound->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: Failed to preload crash sound at /Game/Assets/Sound/box-crash-106687"));
    }

    if (LapSound.Succeeded())
    {
        PreloadedLapSound = LapSound.Object;
        UE_LOG(LogTemp, Log, TEXT("USFXManager: Preloaded lap sound: %s"), *PreloadedLapSound->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: Failed to preload lap sound at /Game/Assets/Sound/yay-6120"));
    }

	if (EngineSound.Succeeded())
	{
		PreloadedEngineSound = EngineSound.Object;
		UE_LOG(LogTemp, Log, TEXT("USFXManager: Preloaded engine sound: %s"), *PreloadedEngineSound->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USFXManager: Failed to preload engine sound at /Game/Assets/Sound/engine-123456"));
	}
}

void USFXManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize SoundMap
    SoundMap = NewObject<UHashMap>(this);
    if (!SoundMap)
    {
        UE_LOG(LogTemp, Error, TEXT("USFXManager: Failed to create SoundMap in BeginPlay!"));
        return;
    }

    // Populate SoundMap with preloaded sounds if they exist
    if (PreloadedWaypointSound) SoundMap->Add("waypoint", PreloadedWaypointSound);
    if (PreloadedCheckpointSound) SoundMap->Add("checkpoint", PreloadedCheckpointSound);
    if (PreloadedCrashSound) SoundMap->Add("crash", PreloadedCrashSound);
    if (PreloadedLapSound) SoundMap->Add("lap", PreloadedLapSound);
    if (PreloadedEngineSound) SoundMap->Add("engine", PreloadedEngineSound);

    // Override with DefaultSoundMappings from Blueprint
    int32 InitialSoundCount = SoundMap->GetSize();
    for (const FSoundMapping& Mapping : DefaultSoundMappings)
    {
        if (Mapping.SoundAsset)
        {
            SoundMap->Add(Mapping.SoundKey, Mapping.SoundAsset);
        }
    }

    // Log initialization status
    if (DefaultSoundMappings.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("USFXManager: Added %d sound mappings from Blueprint. Total sounds: %d"), DefaultSoundMappings.Num(), SoundMap->GetSize());
    }
    else if (InitialSoundCount == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: No Blueprint mappings provided, and no preloaded sounds available!"));
    }
}

void USFXManager::PlaySound(const FString& SoundKey)
{
    if (!GetWorld()) // Safety check
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: No valid world context for PlaySound!"));
        return;
    }

    if (!SoundMap)
    {
        UE_LOG(LogTemp, Error, TEXT("USFXManager: SoundMap is null!"));
        return;
    }

    USoundBase* Sound = SoundMap->Get(SoundKey);
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: Sound %s not found!"), *SoundKey);
    }
}

void USFXManager::PlayBackgroundMusic(const FString& SoundKey)
{
    if (!GetWorld()) // Safety check
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: No valid world context for PlayBackgroundMusic!"));
        return;
    }

    if (!SoundMap)
    {
        UE_LOG(LogTemp, Error, TEXT("USFXManager: SoundMap is null!"));
        return;
    }

    USoundBase* Sound = SoundMap->Get(SoundKey);
    if (Sound)
    {
        if (!BackgroundMusicComponent)
        {
            BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), Sound);
            BackgroundMusicComponent->bAutoDestroy = false;
            BackgroundMusicComponent->SetBoolParameter(FName("Looping"), true);
        }
        else
        {
            BackgroundMusicComponent->SetSound(Sound);
            BackgroundMusicComponent->Play();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USFXManager: Background music %s not found!"), *SoundKey);
    }
}

void USFXManager::StopBackgroundMusic()
{
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->Stop();
    }
}

void USFXManager::AddSound(const FString& SoundKey, USoundBase* Sound)
{
    if (!SoundMap)
    {
        UE_LOG(LogTemp, Error, TEXT("USFXManager: SoundMap is null! Cannot add sound."));
        return;
    }

    if (Sound)
    {
        SoundMap->Add(SoundKey, Sound);
    }
}