#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HashMap.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "SFXManager.generated.h"

USTRUCT(BlueprintType)
struct FSoundMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    FString SoundKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    USoundBase* SoundAsset;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GADE_POE_API USFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    USFXManager();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "SFX")
    void PlaySound(const FString& SoundKey);

    UFUNCTION(BlueprintCallable, Category = "SFX")
    void PlayBackgroundMusic(const FString& SoundKey);

    UFUNCTION(BlueprintCallable, Category = "SFX")
    void StopBackgroundMusic();

    UFUNCTION(BlueprintCallable, Category = "SFX")
    void AddSound(const FString& SoundKey, USoundBase* Sound);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    TArray<FSoundMapping> DefaultSoundMappings;

private:
    UPROPERTY()
    UHashMap* SoundMap;

    UPROPERTY()
    UAudioComponent* BackgroundMusicComponent;

    // Temporary storage for pre-loaded sounds
    UPROPERTY()
    USoundBase* PreloadedWaypointSound;

    UPROPERTY()
    USoundBase* PreloadedCheckpointSound;

    UPROPERTY()
    USoundBase* PreloadedCrashSound;

    UPROPERTY()
    USoundBase* PreloadedLapSound;

    UPROPERTY()
    USoundBase* PreloadedEngineSound;

};