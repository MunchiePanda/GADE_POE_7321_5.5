#include "SFXManager.h"
#include "Kismet/GameplayStatics.h"

// Initialize static instance pointer
ASFXManager* ASFXManager::Instance = nullptr;

ASFXManager::ASFXManager()
{
    // Disable tick since we don't need per-frame updates
    PrimaryActorTick.bCanEverTick = false;

    // Initialize all pointers to nullptr
    SoundMap = nullptr;
    BackgroundMusicComponent = nullptr;
    PreloadedWaypointSound = nullptr;
    PreloadedCheckpointSound = nullptr;
    PreloadedCrashSound = nullptr;
    PreloadedLapSound = nullptr;
    PreloadedEngineSound = nullptr;
    PreloadedBackgroundMusic = nullptr;
    PreloadedButtonClickSound = nullptr;
    PreloadedButtonHoverSound = nullptr;
    PreloadedMenuOpenSound = nullptr;
    PreloadedMenuCloseSound = nullptr;

    // Create audio component for background music
    BackgroundMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BackgroundMusicComponent"));
    BackgroundMusicComponent->bAutoActivate = false;

    // Preload all sound assets
    static ConstructorHelpers::FObjectFinder<USoundBase> WaypointSound(TEXT("/Game/Assets/Sound/ding-36029"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CheckpointSound(TEXT("/Game/Assets/Sound/collect-points-190037"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CrashSound(TEXT("/Game/Assets/Sound/box-crash-106687"));
    static ConstructorHelpers::FObjectFinder<USoundBase> LapSound(TEXT("/Game/Assets/Sound/yay-6120"));
    static ConstructorHelpers::FObjectFinder<USoundBase> EngineSound(TEXT("/Game/Assets/Sound/engine-6000.engine-6000"));
    static ConstructorHelpers::FObjectFinder<USoundBase> BackgroundMusic(TEXT("/Game/Assets/Sound/under-the-streetlights-outrun-racing-1980s-synth-soundtrack-184776.under-the-streetlights-outrun-racing-1980s-synth-soundtrack-184776"));
    static ConstructorHelpers::FObjectFinder<USoundBase> ButtonClickSound(TEXT("/Game/Assets/Sound/ui-button-click-5-327756.ui-button-click-5-327756"));
    static ConstructorHelpers::FObjectFinder<USoundBase> ButtonHoverSound(TEXT("/Game/Assets/Sound/hover-button-287656.hover-button-287656"));
    static ConstructorHelpers::FObjectFinder<USoundBase> MenuOpenSound(TEXT("/Game/Assets/Sound/menu-button-89141.menu-button-89141"));
    static ConstructorHelpers::FObjectFinder<USoundBase> MenuCloseSound(TEXT("/Game/Assets/Sound/menu-button-88360.menu-button-88360"));

    // Assign preloaded sounds if loading was successful
    if (WaypointSound.Succeeded()) PreloadedWaypointSound = WaypointSound.Object;
    if (CheckpointSound.Succeeded()) PreloadedCheckpointSound = CheckpointSound.Object;
    if (CrashSound.Succeeded()) PreloadedCrashSound = CrashSound.Object;
    if (LapSound.Succeeded()) PreloadedLapSound = LapSound.Object;
    if (EngineSound.Succeeded()) PreloadedEngineSound = EngineSound.Object;
    if (BackgroundMusic.Succeeded()) PreloadedBackgroundMusic = BackgroundMusic.Object;
    if (ButtonClickSound.Succeeded()) PreloadedButtonClickSound = ButtonClickSound.Object;
    if (ButtonHoverSound.Succeeded()) PreloadedButtonHoverSound = ButtonHoverSound.Object;
    if (MenuOpenSound.Succeeded()) PreloadedMenuOpenSound = MenuOpenSound.Object;
    if (MenuCloseSound.Succeeded()) PreloadedMenuCloseSound = MenuCloseSound.Object;
}

ASFXManager* ASFXManager::GetInstance(UWorld* World)
{
    // Create new instance if none exists
    if (!Instance && World)
    {
        // Set spawn parameters
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spawn the manager actor
        Instance = World->SpawnActor<ASFXManager>(ASFXManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (Instance)
        {
            Instance->SetActorTickEnabled(false);
            Instance->BeginPlay();
        }
    }
    return Instance;
}

void ASFXManager::BeginPlay()
{
    Super::BeginPlay();

    // Create and initialize the sound map
    SoundMap = NewObject<UHashMap>(this);
    if (!SoundMap)
    {
        UE_LOG(LogTemp, Error, TEXT("ASFXManager: Failed to create SoundMap!"));
        return;
    }

    // Add all preloaded sounds to the sound map
    if (PreloadedWaypointSound) SoundMap->Add("waypoint", PreloadedWaypointSound);
    if (PreloadedCheckpointSound) SoundMap->Add("checkpoint", PreloadedCheckpointSound);
    if (PreloadedCrashSound) SoundMap->Add("crash", PreloadedCrashSound);
    if (PreloadedLapSound) SoundMap->Add("lap", PreloadedLapSound);
    if (PreloadedEngineSound) SoundMap->Add("engine", PreloadedEngineSound);
    if (PreloadedBackgroundMusic) SoundMap->Add("bgm", PreloadedBackgroundMusic);
    if (PreloadedButtonClickSound) SoundMap->Add("button_click", PreloadedButtonClickSound);
    if (PreloadedButtonHoverSound) SoundMap->Add("button_hover", PreloadedButtonHoverSound);
    if (PreloadedMenuOpenSound) SoundMap->Add("menu_open", PreloadedMenuOpenSound);
    if (PreloadedMenuCloseSound) SoundMap->Add("menu_close", PreloadedMenuCloseSound);

    // Add any default sound mappings from the editor
    for (const FSoundMapping& Mapping : DefaultSoundMappings)
    {
        if (Mapping.SoundAsset) SoundMap->Add(Mapping.SoundKey, Mapping.SoundAsset);
    }
}

// Generic sound functions
void ASFXManager::PlaySound(const FString& SoundKey)
{
    if (USoundBase* Sound = Cast<USoundBase>(SoundMap->Get(SoundKey)))
    {
        UGameplayStatics::PlaySound2D(this, Sound);
    }
}

void ASFXManager::AddSound(const FString& SoundKey, USoundBase* Sound)
{
    if (Sound && SoundMap)
    {
        SoundMap->Add(SoundKey, Sound);
    }
}

// Background music functions
void ASFXManager::PlayBackgroundMusic(const FString& SoundKey)
{
    if (USoundBase* Sound = Cast<USoundBase>(SoundMap->Get(SoundKey)))
    {
        if (BackgroundMusicComponent)
        {
            BackgroundMusicComponent->SetSound(Sound);
            BackgroundMusicComponent->Play();
        }
    }
}

void ASFXManager::StopBackgroundMusic()
{
    if (BackgroundMusicComponent && BackgroundMusicComponent->IsPlaying())
    {
        BackgroundMusicComponent->Stop();
    }
}

// UI Sound Functions
void ASFXManager::PlayButtonClickSound()
{
    PlaySound("button_click");
}

void ASFXManager::PlayButtonHoverSound()
{
    PlaySound("button_hover");
}

void ASFXManager::PlayMenuOpenSound()
{
    PlaySound("menu_open");
}

void ASFXManager::PlayMenuCloseSound()
{
    PlaySound("menu_close");
}

void ASFXManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Stop any playing sounds
    if (BackgroundMusicComponent && BackgroundMusicComponent->IsPlaying())
    {
        BackgroundMusicComponent->Stop();
    }

    // Clear the singleton instance
    if (Instance == this)
    {
        Instance = nullptr;
    }
}