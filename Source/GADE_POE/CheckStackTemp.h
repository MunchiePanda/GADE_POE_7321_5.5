// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
template <typename T>
/**
 * 
 */
class GADE_POE_API CheckStackTemp
{
public:
    CheckStackTemp() {}
    ~CheckStackTemp() { Clear(); }

    /** Pushes an item onto the stack */
    void Push(const T& Item)
    {
        StackArray.Add(Item);
    }

    /** Pops the top item from the stack */
    bool Pop(T& OutItem)
    {
        if (StackArray.Num() == 0)
            return false;

        OutItem = StackArray.Last();
        StackArray.RemoveAt(StackArray.Num() - 1);
        return true;
    }

    /** Returns the top item without removing it */
    bool Peek(T& OutItem) const
    {
        if (StackArray.Num() == 0)
            return false;

        OutItem = StackArray.Last();
        return true;
    }

    /** Checks if the stack is empty */
    bool IsEmpty() const
    {
        return StackArray.Num() == 0;
    }

    /** Clears the stack */
    void Clear()
    {
        StackArray.Empty();
    }

    int32 Size() const
    {
        return StackArray.Num();
    }

private:
    TArray<T> StackArray; // Internal storage for stack elements
};
