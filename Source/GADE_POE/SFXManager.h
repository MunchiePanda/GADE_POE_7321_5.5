// SFXManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HashMap.h"
#include "Sound/SoundBase.h"
#include "SFXManager.generated.h"

UCLASS()
class GADE_POE_API USFXManager : public UObject
{
    GENERATED_BODY()
public:
    USFXManager();
    void Initialize();
    void PlaySound(const FString& SoundKey);
    void AddSound(const FString& SoundKey, USoundBase* Sound);

private:
    UPROPERTY()
    UHashMap* SoundMap;
};