#include "AIRacerFactory.h"
#include "NavigationSystem.h"
#include "EngineUtils.h" // For TActorIterator
#include "Kismet/GameplayStatics.h"
#include "AIRacer.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "RacerTypes.h"
#include "RacerSpawnPoint.h"
#include "AIRacerContoller.h"

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
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerFactory: World is null"));
        return;
    }

    SpawnedRacers.Empty();

    // Find all spawn points and remove duplicates
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ARacerSpawnPoint::StaticClass(), FoundActors);

    TMap<FVector, AActor*> UniqueSpawnPoints;
    for (AActor* Actor : FoundActors)
    {
        ARacerSpawnPoint* SpawnPoint = Cast<ARacerSpawnPoint>(Actor);
        if (SpawnPoint)
        {
            FVector Location = SpawnPoint->GetActorLocation();
            if (!UniqueSpawnPoints.Contains(Location))
            {
                UniqueSpawnPoints.Add(Location, SpawnPoint);
                UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Found spawn point at %s"), *Location.ToString());
            }
        }
    }

    SpawnPoints.Empty();
    SpawnPoints.Reserve(UniqueSpawnPoints.Num());
    for (const TPair<FVector, AActor*>& Pair : UniqueSpawnPoints)
    {
        SpawnPoints.Add(Cast<ARacerSpawnPoint>(Pair.Value));
    }

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No spawn points found"));
        return;
    }

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

    int32 RacersToSpawn = FMath::Min(InMaxRacers, SpawnPoints.Num());
    UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Attempting to spawn %d racers"), RacersToSpawn);
    for (int32 i = 0; i < RacersToSpawn; i++)
    {
        FVector SpawnLocation = SpawnPoints[i]->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Processing spawn location %s"), *SpawnLocation.ToString());

        // Reintroduce NavMesh check
        FNavLocation NavLocation;
        UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
        bool bIsOnNavMesh = NavSystem && NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
        if (!bIsOnNavMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is not on NavMesh"), *SpawnLocation.ToString());
            continue;
        }
        SpawnLocation = NavLocation.Location;

        // Reintroduce collision check
        FHitResult Hit;
        bool bCanSpawn = !World->SweepSingleByChannel(
            Hit,
            SpawnLocation,
            SpawnLocation + FVector(0.0f, 0.0f, 1.0f),
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeCapsule(40.0f, 96.0f)
        );

        if (!bCanSpawn)
        {
            FString BlockingActorName = Hit.GetActor() ? Hit.GetActor()->GetName() : TEXT("Unknown");
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is blocked by %s"), *SpawnLocation.ToString(), *BlockingActorName);
            continue;
        }

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

        if (!RacerClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: RacerClass is null for type %s"), *UEnum::GetValueAsString(RacerType));
            continue;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        AAIRacer* NewRacer = World->SpawnActor<AAIRacer>(RacerClass, SpawnLocation, InSpawnRotation, SpawnParams);

        if (NewRacer)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Successfully spawned racer at %s"), *SpawnLocation.ToString());
            NewRacer->RacerType = RacerType;
            NewRacer->SetupRacerAttributes();

            AAIRacerContoller* AIController = World->SpawnActor<AAIRacerContoller>(AAIRacerContoller::StaticClass(), SpawnLocation, InSpawnRotation);
            if (AIController)
            {
                AIController->Possess(NewRacer);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Failed to spawn AIController for racer at %s"), *SpawnLocation.ToString());
            }

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