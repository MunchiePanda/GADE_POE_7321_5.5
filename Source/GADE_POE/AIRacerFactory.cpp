#include "AIRacerFactory.h"
#include "NavigationSystem.h"
#include "EngineUtils.h" // For TActorIterator
#include "Kismet/GameplayStatics.h"
#include "AIRacer.h"
#include "GameFramework/Actor.h"

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
    FVector FinalSpawnLocation = SpawnLocation;
    if (NavSystem && NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(100.0f, 100.0f, 100.0f)))
    {
        FinalSpawnLocation = NavLocation.Location;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is not on NavMesh, using original location"), *SpawnLocation.ToString());
    }

    FActorSpawnParameters SpawnParams;
    AAIRacer* NewRacer = World->SpawnActor<AAIRacer>(SelectedClass, FinalSpawnLocation, InSpawnRotation, SpawnParams);
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
    if (!World) return;

    // Clear any existing spawned racers
    SpawnedRacers.Empty();

    // Find all spawn points in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ARacerSpawnPoint::StaticClass(), FoundActors);

    SpawnPoints.Empty();
    for (AActor* Actor : FoundActors)
    {
        ARacerSpawnPoint* SpawnPoint = Cast<ARacerSpawnPoint>(Actor);
        if (SpawnPoint)
        {
            SpawnPoints.Add(SpawnPoint);
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Found spawn point at %s"), *SpawnPoint->GetActorLocation().ToString());
        }
    }

    // Ensure we have spawn points
    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No spawn points found"));
        return;
    }

    // Normalize probabilities
    float TotalProb = InFastChance + InMediumChance + InSlowChance;
    if (TotalProb <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid probabilities, using default"));
        InFastChance = 0.33f;
        InMediumChance = 0.33f;
        InSlowChance = 0.34f;
        TotalProb = 1.0f;
    }

    float FastProb = InFastChance / TotalProb;
    float MediumProb = InMediumChance / TotalProb;

    // Spawn racers
    int32 RacersToSpawn = FMath::Min(InMaxRacers, SpawnPoints.Num());
    for (int32 i = 0; i < RacersToSpawn; i++)
    {
        FVector SpawnLocation = SpawnPoints[i]->GetActorLocation();
        SpawnLocation.Z += 300.0f; // Increased Z-offset to avoid collision

        // Check if the spawn location is on the NavMesh
        FNavLocation NavLocation;
        UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
        bool bIsOnNavMesh = NavSystem && NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(1000.0f, 1000.0f, 1000.0f));
        if (!bIsOnNavMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is not on NavMesh, using original location"), *SpawnLocation.ToString());
        }
        else
        {
            SpawnLocation = NavLocation.Location;
        }

        // Determine racer type based on probabilities
        float RandomValue = FMath::FRand();
        ERacerType RacerType;
        TSubclassOf<AAIRacer> RacerClass;

        if (RandomValue < FastProb)
        {
            RacerType = ERacerType::Fast;
            RacerClass = FastRacerClass;
        }
        else if (RandomValue < FastProb + MediumProb)
        {
            RacerType = ERacerType::Medium;
            RacerClass = MediumRacerClass;
        }
        else
        {
            RacerType = ERacerType::Slow;
            RacerClass = SlowRacerClass;
        }

        // Perform a sweep test to check for collisions
        FHitResult Hit;
        bool bCanSpawn = !World->SweepSingleByChannel(
            Hit,
            SpawnLocation,
            SpawnLocation + FVector(0.0f, 0.0f, 1.0f),
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeCapsule(40.0f, 96.0f) // Match AAIRacer's capsule size
        );

        if (!bCanSpawn)
        {
            //UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is blocked by %s"), *SpawnLocation.ToString(), *Hit.Actor->GetName());
            continue; // Skip this spawn point
        }

        // Spawn the racer
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        AAIRacer* NewRacer = World->SpawnActor<AAIRacer>(RacerClass, SpawnLocation, InSpawnRotation, SpawnParams);

        if (NewRacer)
        {
            NewRacer->RacerType = RacerType;
            NewRacer->SetupRacerAttributes();
            SpawnedRacers.Add(NewRacer);
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Spawned %s at %s"), *UEnum::GetValueAsString(RacerType), *NewRacer->GetActorLocation().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Failed to spawn %s at %s"), *UEnum::GetValueAsString(RacerType), *SpawnLocation.ToString());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Successfully spawned %d racers"), SpawnedRacers.Num());
}

void AAIRacerFactory::SpawnRacersWithDefaults(UWorld* World)
{
    SpawnRacers(World, MaxRacers, FastChance, MediumChance, SlowChance, SpawnRotation);
}