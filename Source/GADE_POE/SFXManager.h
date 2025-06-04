#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HashMap.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "SFXManager.generated.h"

// Structure to map sound keys to sound assets 
USTRUCT(BlueprintType)
struct FSoundMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    FString SoundKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    USoundBase* SoundAsset;
};

UCLASS()
class GADE_POE_API ASFXManager : public AActor
{
    GENERATED_BODY()

private:
	static ASFXManager* Instance; // Singleton instance

    UPROPERTY()
	UHashMap* SoundMap; // HashMap to store sound mappings

    UPROPERTY()
	UAudioComponent* BackgroundMusicComponent; // Audio component for background music

    UPROPERTY()
	USoundBase* PreloadedWaypointSound; // Preloaded sound for waypoints

    UPROPERTY() 
    USoundBase* PreloadedCheckpointSound; // Preloaded sound for checkpoints

    UPROPERTY()
	USoundBase* PreloadedCrashSound; // Preloaded sound for crashes

    UPROPERTY()
	USoundBase* PreloadedLapSound; // Preloaded sound for laps

    UPROPERTY()
	USoundBase* PreloadedEngineSound; // Preloaded sound for engine

	UPROPERTY()
	USoundBase* PreloadedBackgroundMusic; // New property for background music

protected:
    ASFXManager();

public:
    static ASFXManager* GetInstance(UWorld* World); // Static function to get the singleton instance

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    ~ASFXManager();

    UFUNCTION(BlueprintCallable, Category = "SFX") 
    void PlaySound(const FString& SoundKey); // Blueprint function to play a sound

    UFUNCTION(BlueprintCallable, Category = "SFX")
    void PlayBackgroundMusic(const FString& SoundKey); // Blueprint function to play background music

    UFUNCTION(BlueprintCallable, Category = "SFX")
	void StopBackgroundMusic(); // Blueprint function to stop background music

    UFUNCTION(BlueprintCallable, Category = "SFX")
    void AddSound(const FString& SoundKey, USoundBase* Sound); // Blueprint function to add a sound

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TArray<FSoundMapping> DefaultSoundMappings; // Default sound mappings for the SFX Manager
};