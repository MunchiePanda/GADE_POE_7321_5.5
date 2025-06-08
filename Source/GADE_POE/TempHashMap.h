#pragma once
#include "CoreMinimal.h"
#include "TempLinkedList.h"

template<typename K, typename V>
class TempHashMap
{
private:
    struct HashNode
    {
		K Key; // key of the node, should be a pointer type
        V Value; // value of the key if found
        HashNode(const K& InKey, const V& InValue) : Key(InKey), Value(InValue) {}
        bool operator==(const HashNode& Other) const { return Key == Other.Key; }
    };

    static const int32 BucketCount = 64;
    TempLinkedList<HashNode> Buckets[BucketCount]; // array of linked lists

    int32 GetBucketIndex(const K& Key) const // hash function
    {
        if (!Key || !Key->IsValidLowLevel()) // check if key is valid
        {
            UE_LOG(LogTemp, Warning, TEXT("TempHashMap::GetBucketIndex - Invalid or null key"));
            return 0;
        }
        return GetTypeHash(Key) & (BucketCount - 1);
    }

public:
    TempHashMap() {}

    void Add(const K& Key, const V& Value)
    {
        if (!Key || !Key->IsValidLowLevel()) // check if key is valid
        {
            UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Add - Invalid or null key"));
            return;
        }
        int32 Index = GetBucketIndex(Key);
        TNode<HashNode>* Node = Buckets[Index].Find([Key](const HashNode& N) {
            if (!N.Key)
            {
                UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Add - Null key in HashNode"));
                return false;
            }
            return N.Key && N.Key->IsValidLowLevel() && Key && Key->IsValidLowLevel() && N.Key == Key;
            });
        if (Node)
        {
            Node->Data.Value = Value;
        }
        else
        {
            Buckets[Index].Add(HashNode(Key, Value));
        }
    }

    V* Get(const K& Key) const // get value by key from hash table
    {
        if (!Key || !Key->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Get - Invalid or null key"));
            return nullptr;
        }
        int32 Index = GetBucketIndex(Key);
        TNode<HashNode>* Node = Buckets[Index].Find([Key](const HashNode& N) {
            if (!N.Key)
            {
                UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Get - Null key in HashNode"));
                return false;
            }
            if (!N.Key->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Get - Invalid node key in bucket"));
                return false;
            }
            return N.Key == Key;
            });
        return Node ? &Node->Data.Value : nullptr;
    }

	void Remove(const K& Key) // remove key from hash table 
    {
        if (!Key || !Key->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Remove - Invalid or null key"));
            return;
        }
        int32 Index = GetBucketIndex(Key);
        int32 OldCount = Buckets[Index].GetCount();
        Buckets[Index].Remove([Key](const HashNode& N) {
            if (!N.Key)
            {
                UE_LOG(LogTemp, Warning, TEXT("TempHashMap::Remove - Null key in HashNode"));
                return false;
            }
            return N.Key && N.Key->IsValidLowLevel() && Key && Key->IsValidLowLevel() && N.Key == Key;
            });
        if (Buckets[Index].GetCount() < OldCount)
        {
            UE_LOG(LogTemp, Log, TEXT("TempHashMap::Remove - Removed key from bucket %d"), Index);
        }
    }

	void Clear() // clear all buckets in hash table
    {
        for (int32 i = 0; i < BucketCount; i++)
        {
            Buckets[i].Clear();
        }
    }

    void GetAllKeys(TArray<K>& OutKeys) const // get all keys from hash table
    {
        OutKeys.Empty();
        for (int32 i = 0; i < BucketCount; i++)
        {
            TNode<HashNode>* Current = Buckets[i].GetHead();
            while (Current)
            {
                if (Current->Data.Key && Current->Data.Key->IsValidLowLevel())
                {
                    OutKeys.Add(Current->Data.Key);
                }
                Current = Current->Next;
            }
        }
    }
};