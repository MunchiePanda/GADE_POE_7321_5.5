#pragma once
#include "CoreMinimal.h"
#include "TempLinkedList.h"

template<typename K, typename V>
class TempHashMap
{
private:
    struct HashNode
    {
        K Key;
        V Value;
        HashNode(const K& InKey, const V& InValue) : Key(InKey), Value(InValue) {}
        bool operator==(const HashNode& Other) const { return Key == Other.Key; }
    };

    static const int32 BucketCount = 16;
    TempLinkedList<HashNode> Buckets[BucketCount];

    int32 GetBucketIndex(const K& Key) const
    {
        return GetTypeHash(Key) & (BucketCount - 1);
    }

public:
    TempHashMap() {}

    void Add(const K& Key, const V& Value)
    {
        int32 Index = GetBucketIndex(Key);
        TNode<HashNode>* Node = Buckets[Index].Find([Key](const HashNode& N) { return N.Key == Key; });
        if (Node)
        {
            Node->Data.Value = Value;
        }
        else
        {
            Buckets[Index].Add(HashNode(Key, Value));
        }
    }

    V* Get(const K& Key) const
    {
        int32 Index = GetBucketIndex(Key);
        TNode<HashNode>* Node = Buckets[Index].Find([Key](const HashNode& N) { return N.Key == Key; });
        return Node ? &Node->Data.Value : nullptr;
    }

    void Clear()
    {
        for (int32 i = 0; i < BucketCount; i++)
        {
            Buckets[i].Clear();
        }
    }

    void GetAllKeys(TArray<K>& OutKeys) const
    {
        for (int32 i = 0; i < BucketCount; i++)
        {
            TNode<HashNode>* Current = Buckets[i].GetHead();
            while (Current)
            {
                OutKeys.Add(Current->Data.Key);
                Current = Current->Next;
            }
        }
    }
};