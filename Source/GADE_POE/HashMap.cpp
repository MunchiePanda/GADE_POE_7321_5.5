#include "HashMap.h"

UHashMap::UHashMap()
{
    Table.SetNum(TableSize); // Initialize array with null pointers
    for (int32 i = 0; i < TableSize; ++i)
    {
        Table[i] = nullptr; // Assign nullptr to each pointer element
    }
    Size = 0;
}

UHashMap::~UHashMap()
{
    // Safely clean up manually allocated FHashMapEntry nodes
    for (int32 i = 0; i < TableSize; ++i)
    {
        FHashMapEntry* Entry = Table[i];
        while (Entry)
        {
            FHashMapEntry* Temp = Entry;
            Entry = Entry->Next;
            delete Temp; // Delete the FHashMapEntry structure
        }
        Table[i] = nullptr; // Reset to null for safety
    }
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
    FHashMapEntry* Entry = Table[Index];

    if (!Entry)
    {
        Table[Index] = new FHashMapEntry(Key, Value); // Allocate new entry
        Size++;
        return;
    }

    while (Entry)
    {
        if (Entry->Key == Key)
        {
            Entry->Value = Value; // Update value, weak ptr handles reference
            return;
        }
        if (!Entry->Next) break;
        Entry = Entry->Next;
    }
    Entry->Next = new FHashMapEntry(Key, Value); // Allocate new entry for chaining
    Size++;
}

USoundBase* UHashMap::Get(const FString& Key)
{
    uint32 Index = Hash(Key);
    FHashMapEntry* Entry = Table[Index];

    while (Entry)
    {
        if (Entry->Key == Key)
        {
            return Entry->Value.Get(); // Return raw pointer from weak ptr
        }
        Entry = Entry->Next;
    }
    return nullptr;
}

void UHashMap::Remove(const FString& Key)
{
    uint32 Index = Hash(Key);
    FHashMapEntry* Entry = Table[Index];
    FHashMapEntry* Prev = nullptr;

    while (Entry)
    {
        if (Entry->Key == Key)
        {
            if (Prev) Prev->Next = Entry->Next;
            else Table[Index] = Entry->Next;
            delete Entry;
            Size--;
            return;
        }
        Prev = Entry;
        Entry = Entry->Next;
    }
}