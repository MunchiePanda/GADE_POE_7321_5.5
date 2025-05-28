// SFXManager.cpp
#include "SFXManager.h"
#include "Kismet/GameplayStatics.h"

USFXManager::USFXManager()
{
    SoundMap = NewObject<UHashMap>(); // Custom HashMap instance
}

void USFXManager::Initialize()
{
    // Placeholder sound assets (replace with actual paths in Content/Sounds)
    static ConstructorHelpers::FObjectFinder<USoundBase> WaypointSound(TEXT("/Game/Assets/Sound/ding-36029.ding-36029"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CheckpointSound(TEXT("/Game/Assets/Sound/collect-points-190037.collect-points-190037"));
    static ConstructorHelpers::FObjectFinder<USoundBase> CrashSound(TEXT("/Game/Assets/Sound/box-crash-106687.box-crash-106687"));
    static ConstructorHelpers::FObjectFinder<USoundBase> LapSound(TEXT("/Game/Assets/Sound/yay-6120.yay-6120/Game/Assets/Sound/yay-6120.yay-6120"));

    if (WaypointSound.Succeeded()) SoundMap->Add("waypoint", WaypointSound.Object);
    if (CheckpointSound.Succeeded()) SoundMap->Add("checkpoint", CheckpointSound.Object);
    if (CrashSound.Succeeded()) SoundMap->Add("crash", CrashSound.Object);
    if (LapSound.Succeeded()) SoundMap->Add("lap", LapSound.Object);
}

void USFXManager::PlaySound(const FString& SoundKey)
{
    USoundBase* Sound = SoundMap->Get(SoundKey);
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound %s not found!"), *SoundKey);
    }
}

void USFXManager::AddSound(const FString& SoundKey, USoundBase* Sound)
{
    SoundMap->Add(SoundKey, Sound);
}