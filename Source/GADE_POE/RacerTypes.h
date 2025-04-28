#pragma once

UENUM(BlueprintType)
enum class ERacerType : uint8
{
    Fast UMETA(DisplayName = "Fast Racer"),
    Medium UMETA(DisplayName = "Medium Racer"),
    Slow UMETA(DisplayName = "Slow Racer")
};