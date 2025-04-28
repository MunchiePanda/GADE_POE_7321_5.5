#include "BeginnerRace_GMB.h"

ABeginnerRace_GMB::ABeginnerRace_GMB()
{
    RacerFactory = CreateDefaultSubobject<UAIRacerFactory>(TEXT("RacerFactory"));
}

void ABeginnerRace_GMB::BeginPlay()
{
    Super::BeginPlay();

    if (RacerFactory)
    {
        // Set the racer classes in the factory before spawning
        RacerFactory->FastRacerClass = FastRacerClass;
        RacerFactory->MediumRacerClass = MediumRacerClass;
        RacerFactory->SlowRacerClass = SlowRacerClass;

        RacerFactory->SpawnRacersWithDefaults(GetWorld());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BeginnerRace_GMB: RacerFactory is null!"));
    }
}