#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "UObject/Object.h"
#include "HashMap.generated.h"

struct FHashMapEntry
{
    FString Key;
    TWeakObjectPtr<USoundBase> Value; // Use weak pointer for UObjects

    FHashMapEntry() {}
    FHashMapEntry(const FString& K, USoundBase* V) : Key(K), Value(V) {}
};

UCLASS()
class GADE_POE_API UHashMap : public UObject
{
    GENERATED_BODY()

public:
    UHashMap();
    virtual ~UHashMap();

    void Add(const FString& Key, USoundBase* Value);
    USoundBase* Get(const FString& Key);
    void Remove(const FString& Key);
    int32 GetSize() const { return Size; }

private:
    TArray<TArray<FHashMapEntry>> Table; // 2D array for chaining
    int32 Size;
    static const int32 TableSize = 16;

    uint32 Hash(const FString& Key) const;
};