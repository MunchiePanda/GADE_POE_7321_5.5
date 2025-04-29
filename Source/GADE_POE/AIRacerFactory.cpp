#include "AIRacerFactory.h"
#include "NavigationSystem.h"

AAIRacerFactory::AAIRacerFactory()
{
    PrimaryActorTick.bCanEverTick = false;

    MaxRacers = 9;
    FastChance = 0.2f;
    MediumChance = 0.5f;
    SlowChance = 0.3f;
    SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
    SpawnedRacers.Empty();
    
    static ConstructorHelpers::FClassFinder<ARacerSpawnPoint> SpawnPointClassFinder(TEXT("/Game/Blueprints/BP_RacerSpawnPoint"));
    if (SpawnPointClassFinder.Succeeded())
    {
        SpawnPointClass = SpawnPointClassFinder.Class;
    }
    
}

void AAIRacerFactory::BeginPlay()
{
    Super::BeginPlay();

    // Find all ARacerSpawnPoint actors in the level
    SpawnPoints.Empty();
    for (TActorIterator<ARacerSpawnPoint> It(GetWorld(), SpawnPointClass); It; ++It)
    {
        ARacerSpawnPoint* SpawnPoint = *It;
        if (SpawnPoint)
        {
            SpawnPoints.Add(SpawnPoint);
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Found spawn point at %s"), *SpawnPoint->GetActorLocation().ToString());
        }
    }

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No spawn points found in level! Cannot spawn racers."));
        return;
    }

    SpawnRacersWithDefaults(GetWorld());
}

AAIRacer* AAIRacerFactory::CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& InSpawnRotation)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid World"));
        return nullptr;
    }

    TSubclassOf<AAIRacer> SelectedClass = nullptr;
    switch (RacerType)
    {
    case ERacerType::Fast:
        SelectedClass = FastRacerClass;
        break;
    case ERacerType::Medium:
        SelectedClass = MediumRacerClass;
        break;
    case ERacerType::Slow:
        SelectedClass = SlowRacerClass;
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid RacerType"));
        return nullptr;
    }

    if (!SelectedClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No class set for RacerType %s"), *UEnum::GetValueAsString(RacerType));
        return nullptr;
    }

    FNavLocation NavLocation;
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (!NavSystem || !NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(100.0f, 100.0f, 100.0f)))
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is not on NavMesh"), *SpawnLocation.ToString());
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    AAIRacer* NewRacer = World->SpawnActor<AAIRacer>(SelectedClass, NavLocation.Location, InSpawnRotation, SpawnParams);
    if (NewRacer)
    {
        NewRacer->RacerType = RacerType;
        NewRacer->SetupRacerAttributes();
        SpawnedRacers.Add(NewRacer);
        UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Spawned %s at %s"), *UEnum::GetValueAsString(RacerType), *NavLocation.Location.ToString());
    }

    return NewRacer;
}

void AAIRacerFactory::SpawnRacers(UWorld* World, int32 InMaxRacers, float InFastChance, float InMediumChance, float InSlowChance, const FRotator& InSpawnRotation)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid World"));
        return;
    }

    float TotalChance = InFastChance + InMediumChance + InSlowChance;
    if (TotalChance <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid chances (sum must be greater than 0)"));
        return;
    }

    float FastProb = InFastChance / TotalChance;
    float MediumProb = InMediumChance / TotalChance;
    float SlowProb = InSlowChance / TotalChance;

    SpawnedRacers.Empty();

    int32 RacersToSpawn = FMath::Min(InMaxRacers, SpawnPoints.Num());
    for (int32 i = 0; i < RacersToSpawn; i++)
    {
        FVector SpawnLocation = SpawnPoints[i]->GetActorLocation();

        float RandomValue = FMath::FRand();
        ERacerType RacerType;

        if (RandomValue < FastProb)
        {
            RacerType = ERacerType::Fast;
        }
        else if (RandomValue < FastProb + MediumProb)
        {
            RacerType = ERacerType::Medium;
        }
        else
        {
            RacerType = ERacerType::Slow;
        }

        CreateRacer(World, RacerType, SpawnLocation, InSpawnRotation);
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Spawned %d racers"), RacersToSpawn);
}

void AAIRacerFactory::SpawnRacersWithDefaults(UWorld* World)
{
    SpawnRacers(World, MaxRacers, FastChance, MediumChance, SlowChance, SpawnRotation);
}