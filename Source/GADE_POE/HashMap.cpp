#include "HashMap.h"

UHashMap::UHashMap()
{
    Table.SetNum(TableSize); // Initialize 2D array
    Size = 0;
}

UHashMap::~UHashMap()
{
    // No manual deletion needed; TArray handles cleanup
}

uint32 UHashMap::Hash(const FString& Key) const
{
    uint32 Hash = 0;
    for (const TCHAR* Char = *Key; *Char; ++Char)
    {
        Hash = (Hash * 31) + (*Char - 'A'); // Case-insensitive basic hash
    }
    return Hash % TableSize;
}

void UHashMap::Add(const FString& Key, USoundBase* Value)
{
    uint32 Index = Hash(Key);
    TArray<FHashMapEntry>& Chain = Table[Index];

    // Check for existing key to update or append
    for (FHashMapEntry& Entry : Chain)
    {
        if (Entry.Key == Key)
        {
            Entry.Value = Value; // Update existing value
            return;
        }
    }

    // Add new entry
    Chain.Add(FHashMapEntry(Key, Value));
    Size++;
}

USoundBase* UHashMap::Get(const FString& Key)
{
    uint32 Index = Hash(Key);
    const TArray<FHashMapEntry>& Chain = Table[Index];

    for (const FHashMapEntry& Entry : Chain)
    {
        if (Entry.Key == Key)
        {
            return Entry.Value.Get(); // Return raw pointer from weak ptr
        }
    }
    return nullptr;
}

void UHashMap::Remove(const FString& Key)
{
    uint32 Index = Hash(Key);
    TArray<FHashMapEntry>& Chain = Table[Index];

    for (int32 i = 0; i < Chain.Num(); ++i)
    {
        if (Chain[i].Key == Key)
        {
            Chain.RemoveAt(i);
            Size--;
            return;
        }
    }
}