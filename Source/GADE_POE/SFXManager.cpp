#include "SFXManager.h"
#include "Kismet/GameplayStatics.h"

ASFXManager* ASFXManager::Instance = nullptr;

ASFXManager::ASFXManager()
{
    PrimaryActorTick.bCanEverTick = false;

	// Initialize variables to nullptr and preload sounds here
    SoundMap = nullptr;
    BackgroundMusicComponent = nullptr;
    PreloadedWaypointSound = nullptr;
    PreloadedCheckpointSound = nullptr;
    PreloadedCrashSound = nullptr;
    PreloadedLapSound = nullptr;
    PreloadedEngineSound = nullptr;
    PreloadedBackgroundMusic = nullptr; 

    // Preload sounds
    static ConstructorHelpers::FObjectFinder<USoundBase> WaypointSound(TEXT("/Game/Assets/Sound/ding-36029"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CheckpointSound(TEXT("/Game/Assets/Sound/collect-points-190037"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CrashSound(TEXT("/Game/Assets/Sound/box-crash-106687"));
    static ConstructorHelpers::FObjectFinder<USoundBase> LapSound(TEXT("/Game/Assets/Sound/yay-6120"));
    static ConstructorHelpers::FObjectFinder<USoundBase> EngineSound(TEXT("/Game/Assets/Sound/engine-6000.engine-6000"));
    static ConstructorHelpers::FObjectFinder<USoundBase> BackgroundMusic(TEXT("/Game/Assets/Sound/under-the-streetlights-outrun-racing-1980s-synth-soundtrack-184776.under-the-streetlights-outrun-racing-1980s-synth-soundtrack-184776")); // Add background music

    // Assign the preloaded sound to the member variable 
    if (WaypointSound.Succeeded()) PreloadedWaypointSound = WaypointSound.Object;
    if (CheckpointSound.Succeeded()) PreloadedCheckpointSound = CheckpointSound.Object;
    if (CrashSound.Succeeded()) PreloadedCrashSound = CrashSound.Object;
    if (LapSound.Succeeded()) PreloadedLapSound = LapSound.Object;
    if (EngineSound.Succeeded()) PreloadedEngineSound = EngineSound.Object;
    if (BackgroundMusic.Succeeded()) PreloadedBackgroundMusic = BackgroundMusic.Object;
}

ASFXManager* ASFXManager::GetInstance(UWorld* World)
{
    // If no instance exists, create one and return it
    if (!Instance && World)
    {
        FActorSpawnParameters SpawnParams; // Set spawn parameters
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // Set spawn collision handling
        Instance = World->SpawnActor<ASFXManager>(ASFXManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (Instance) // If the instance was successfully created
        {
            Instance->SetActorTickEnabled(false);
            Instance->BeginPlay(); // Call BeginPlay immediately to ensure SoundMap is populated
        }
    }
    return Instance;
}

void ASFXManager::BeginPlay()
{
    Super::BeginPlay();

	// Ensure SoundMap is initialized and populated
    SoundMap = NewObject<UHashMap>(this);
    if (!SoundMap)
    {
        UE_LOG(LogTemp, Error, TEXT("ASFXManager: Failed to create SoundMap!"));
        return;
    }

	// Add preloaded sounds to the SoundMap 
    if (PreloadedWaypointSound) SoundMap->Add("waypoint", PreloadedWaypointSound);
    if (PreloadedCheckpointSound) SoundMap->Add("checkpoint", PreloadedCheckpointSound);
    if (PreloadedCrashSound) SoundMap->Add("crash", PreloadedCrashSound);
    if (PreloadedLapSound) SoundMap->Add("lap", PreloadedLapSound);//
    if (PreloadedEngineSound) SoundMap->Add("engine", PreloadedEngineSound);
    if (PreloadedBackgroundMusic) SoundMap->Add("bgm", PreloadedBackgroundMusic); // Add background music to SoundMap

	// Populate SoundMap with default sound mappings
    for (const FSoundMapping& Mapping : DefaultSoundMappings)
    {
        if (Mapping.SoundAsset) SoundMap->Add(Mapping.SoundKey, Mapping.SoundAsset);
    }
}

void ASFXManager::PlaySound(const FString& SoundKey)
{
	// Check if the world and SoundMap are valid before proceeding
    if (!GetWorld() || !SoundMap) return;

    // Try to retrieve the sound from the SoundMap using the provided key
    USoundBase* Sound = SoundMap->Get(SoundKey);
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ASFXManager: Sound key '%s' not found in SoundMap"), *SoundKey);
    }
}

void ASFXManager::PlayBackgroundMusic(const FString& SoundKey)
{
    // Check if the world and SoundMap are valid before proceeding
    if (!GetWorld() || !SoundMap) return;

    // Try to retrieve the sound from the SoundMap using the provided key 
    USoundBase* Sound = SoundMap->Get(SoundKey);
    if (Sound)
    {
        // If there's an existing component, stop and destroy it first
        if (BackgroundMusicComponent)
        {
            BackgroundMusicComponent->Stop();
            BackgroundMusicComponent->DestroyComponent();
            BackgroundMusicComponent = nullptr;
        }

        // Create a new component
        BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), Sound);
        if (BackgroundMusicComponent)
        {
            BackgroundMusicComponent->bAutoDestroy = false;
            BackgroundMusicComponent->SetVolumeMultiplier(0.5f); // Reduce volume a bit
            BackgroundMusicComponent->bIsUISound = true; // This helps prevent issues during level transitions
            BackgroundMusicComponent->Play();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ASFXManager: Failed to create BackgroundMusicComponent for '%s'"), *SoundKey);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ASFXManager: Background music key '%s' not found in SoundMap"), *SoundKey);
    }
}

void ASFXManager::StopBackgroundMusic() // Stop the background music
{
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->Stop();
    }
}

void ASFXManager::AddSound(const FString& SoundKey, USoundBase* Sound)
{
	// Check if SoundMap is valid before adding the sound 
    if (SoundMap && Sound) SoundMap->Add(SoundKey, Sound);
}

void ASFXManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->Stop();
        BackgroundMusicComponent->DestroyComponent();
        BackgroundMusicComponent = nullptr;
    }
    
    Instance = nullptr;
}

ASFXManager::~ASFXManager()
{
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->Stop();
        BackgroundMusicComponent = nullptr;
    }
    Instance = nullptr;
}