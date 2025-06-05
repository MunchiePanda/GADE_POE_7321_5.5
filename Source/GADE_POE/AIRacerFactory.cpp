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
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AAIRacerFactory::AAIRacerFactory()
{
    PrimaryActorTick.bCanEverTick = false;
	// Set default values for the factory
    MaxRacers = 9;
    FastChance = 0.2f;
    MediumChance = 0.5f;
    SlowChance = 0.3f;
    SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
    SpawnedRacers.Empty();
    
    static ConstructorHelpers::FClassFinder<ARacerSpawnPoint> SpawnPointClassFinder(TEXT("/Game/Blueprints/AI_Racers/MyRacerSpawnPoint"));
    if (SpawnPointClassFinder.Succeeded()) // Check if the class was successfully found
    {
        SpawnPointClass = SpawnPointClassFinder.Class;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Failed to find MyRacerSpawnPoint blueprint class"));
    }
}

void AAIRacerFactory::BeginPlay()
{
    Super::BeginPlay();

    // Find all ARacerSpawnPoint actors in the level
    
    SpawnPoints.Empty(); // Clear the array of spawn points 
	for (TActorIterator<ARacerSpawnPoint> It(GetWorld(), SpawnPointClass); It; ++It) // Iterate through all actors of the specified class
    {
        // Cast the actor to ARacerSpawnPoint and add it to the array
        ARacerSpawnPoint* SpawnPoint = *It;
        if (SpawnPoint)
        {
            SpawnPoints.Add(SpawnPoint);
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Found spawn point at %s"), *SpawnPoint->GetActorLocation().ToString());
        }
    }

    // Check if any spawn points were found 
    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No spawn points found in level! Cannot spawn racers."));
        return;
    }

	SpawnRacersWithDefaults(GetWorld()); // Spawn racers with default values
	
}

AAIRacer* AAIRacerFactory::CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& InSpawnRotation)
{
    if (!World) // Check if the world is valid 
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid World"));
        return nullptr;
    }

    TSubclassOf<AAIRacer> SelectedClass = nullptr; // Variable to store the selected class
    switch (RacerType) // Switch case to select the class based on the RacerType 
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

	if (!SelectedClass) // Check if the selected class is valid
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No class set for RacerType %s"), *UEnum::GetValueAsString(RacerType));
        return nullptr;
    }

	FNavLocation NavLocation; // Variable to store the navigation location
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World); // Get the navigation system 
    FVector FinalSpawnLocation = SpawnLocation; // Variable to store the final spawn location 
    if (NavSystem)
    {
        // Try to project to nav mesh with a smaller extent
        FVector QueryExtent(50.0f, 50.0f, 50.0f);
        if (NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, QueryExtent))
        {
            FinalSpawnLocation = NavLocation.Location + FVector(0, 0, 50.0f); // Reduced height offset
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Successfully projected spawn location to nav mesh at %s"), *FinalSpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Failed to project spawn location %s to NavMesh"), *SpawnLocation.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No NavSystem available"));
    }

    FActorSpawnParameters SpawnParams; // Variable to store the spawn parameters 
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AAIRacer* NewRacer = World->SpawnActor<AAIRacer>(SelectedClass, FinalSpawnLocation, InSpawnRotation, SpawnParams);
    if (NewRacer)
    {
        NewRacer->RacerType = RacerType;
		NewRacer->SetupRacerAttributes(); // Call the setup function to initialize the racer
        SpawnedRacers.Add(NewRacer);
        UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Spawned %s at %s"), *UEnum::GetValueAsString(RacerType), *FinalSpawnLocation.ToString());
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

    // Create a map to store unique spawn points and their probabilities 
    TMap<FVector, AActor*> UniqueSpawnPoints;
    for (AActor* Actor : FoundActors) // Loop through the found actors 
    {
        ARacerSpawnPoint* SpawnPoint = Cast<ARacerSpawnPoint>(Actor); // Cast the actor to ARacerSpawnPoint
        if (SpawnPoint)
        {
			FVector Location = SpawnPoint->GetActorLocation(); // Get the location of the spawn point
            if (!UniqueSpawnPoints.Contains(Location)) // Check if the location is already in the map
            {
                UniqueSpawnPoints.Add(Location, SpawnPoint); // Add the spawn point to the map 
                UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Found spawn point at %s"), *Location.ToString()); 
            }
        }
    }

	// Convert the map to an array of spawn points
    SpawnPoints.Empty();
    SpawnPoints.Reserve(UniqueSpawnPoints.Num()); // Reserve space in the array
    for (const TPair<FVector, AActor*>& Pair : UniqueSpawnPoints) // Loop through the map
    {
		SpawnPoints.Add(Cast<ARacerSpawnPoint>(Pair.Value)); // Add the spawn point to the array
    }

    if (SpawnPoints.Num() == 0) // Check if any spawn points were found
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No spawn points found"));
        return;
    }

    float TotalProb = InFastChance + InMediumChance + InSlowChance;
    if (TotalProb <= 0.0f) // Check if the probabilities are valid 
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Invalid probabilities, using default")); 
        InFastChance = 0.33f;
        InMediumChance = 0.33f;
        InSlowChance = 0.34f;
        TotalProb = 1.0f;
    }

    float FastProb = InFastChance / TotalProb;
    float MediumProb = InMediumChance / TotalProb;

    // Calculate the number of racers to spawn
    int32 RacersToSpawn = FMath::Min(InMaxRacers, SpawnPoints.Num());
    UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Attempting to spawn %d racers"), RacersToSpawn);
    for (int32 i = 0; i < RacersToSpawn; i++)
    {
        FVector SpawnLocation = SpawnPoints[i]->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Processing spawn location %s"), *SpawnLocation.ToString());

        // Validate spawn point height with a longer trace
        FHitResult GroundHit;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 500.0f);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 500.0f);
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = true;
        QueryParams.AddIgnoredActor(this);
        
        bool bFoundGround = World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
        if (bFoundGround)
        {
            SpawnLocation = GroundHit.Location + FVector(0, 0, 100.0f); // Increased height offset
            DrawDebugLine(World, TraceStart, GroundHit.Location, FColor::Green, false, 5.0f);
            DrawDebugPoint(World, SpawnLocation, 20.0f, FColor::Red, false, 5.0f);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: No ground found below spawn point %s"), *SpawnLocation.ToString());
            continue;
        }

        // Check for overlaps at spawn location
        FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(40.0f, 96.0f);
        bool bHasOverlap = World->OverlapBlockingTestByChannel(
            SpawnLocation,
            FQuat::Identity,
            ECC_Pawn,
            CapsuleShape
        );

        if (bHasOverlap)
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerFactory: Spawn location %s is blocked"), *SpawnLocation.ToString());
            continue;
        }

        // Spawn parameters
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        SpawnParams.bNoFail = true;

        // Determine racer type and spawn the racer
        float RandomValue = FMath::FRand();
        ERacerType RacerType;
        TSubclassOf<AAIRacer> RacerClass;
        //staements check if the random value is less than the cumulative probability of each type of racer 
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
        //spawn the racer and AI controller 
        AAIRacer* NewRacer = World->SpawnActor<AAIRacer>(RacerClass, SpawnLocation, InSpawnRotation, SpawnParams);

        if (NewRacer)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerFactory: Successfully spawned racer at %s"), *SpawnLocation.ToString());
			NewRacer->RacerType = RacerType; // Set the racer type
            NewRacer->SetupRacerAttributes(); // Set the racer attributes

            AAIRacerContoller* AIController = World->SpawnActor<AAIRacerContoller>(AAIRacerContoller::StaticClass(), SpawnLocation, InSpawnRotation);
            if (AIController)
            {
                AIController->Possess(NewRacer); //possess the spawned in racer
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

void AAIRacerFactory::SpawnRacersWithDefaults(UWorld* World) // Function to spawn racers with default values
{
    SpawnRacers(World, MaxRacers, FastChance, MediumChance, SlowChance, SpawnRotation);
}