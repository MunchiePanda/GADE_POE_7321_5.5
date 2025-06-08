// SFXManager.h
// This class manages all sound effects in the game using a singleton pattern.
// It handles preloading, storing, and playing various game sounds including
// background music, waypoint sounds, checkpoint sounds, and more.

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

    // Key used to identify the sound
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    FString SoundKey;

    // The actual sound asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    USoundBase* SoundAsset;
};

UCLASS()
class GADE_POE_API ASFXManager : public AActor
{
    GENERATED_BODY()

private:
	// Singleton instance of the SFX manager
	static ASFXManager* Instance;

    // HashMap to store sound mappings
    UPROPERTY()
	UHashMap* SoundMap;

    // Audio component for background music
    UPROPERTY()
	UAudioComponent* BackgroundMusicComponent;

    // Preloaded sound assets
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

	UPROPERTY()
	USoundBase* PreloadedBackgroundMusic;

    // UI Sound Effects
    UPROPERTY()
    USoundBase* PreloadedButtonClickSound;

    UPROPERTY()
    USoundBase* PreloadedButtonHoverSound;

    UPROPERTY()
    USoundBase* PreloadedMenuOpenSound;

    UPROPERTY()
    USoundBase* PreloadedMenuCloseSound;

protected:
	// Constructor - initialize components and preload sounds
	ASFXManager();

public:
	// Static function to get the singleton instance
	static ASFXManager* GetInstance(UWorld* World);

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when the actor is being destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Generic sound functions
	UFUNCTION(BlueprintCallable, Category = "SFX")
	void PlaySound(const FString& SoundKey);

	UFUNCTION(BlueprintCallable, Category = "SFX")
	void AddSound(const FString& SoundKey, USoundBase* Sound);

	// Background music functions
	UFUNCTION(BlueprintCallable, Category = "SFX")
	void PlayBackgroundMusic(const FString& SoundKey);

	UFUNCTION(BlueprintCallable, Category = "SFX")
	void StopBackgroundMusic();

	// UI Sound Functions
	UFUNCTION(BlueprintCallable, Category = "SFX|UI")
	void PlayButtonClickSound();

	UFUNCTION(BlueprintCallable, Category = "SFX|UI")
	void PlayButtonHoverSound();

	UFUNCTION(BlueprintCallable, Category = "SFX|UI")
	void PlayMenuOpenSound();

	UFUNCTION(BlueprintCallable, Category = "SFX|UI")
	void PlayMenuCloseSound();

	// Default sound mappings that can be set in the editor
	UPROPERTY(EditAnywhere, Category = "SFX")
	TArray<FSoundMapping> DefaultSoundMappings;
};