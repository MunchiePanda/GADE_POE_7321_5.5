#include "AIRacer.h"
#include "WaypointManager.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

AAIRacer::AAIRacer()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("Pawn"));

    MoveSpeed = 600.0f; // Adjust in Blueprint
    CurrentWaypointIndex = 0;
}

void AAIRacer::BeginPlay()
{
    Super::BeginPlay();

    // Find WaypointManager
    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (WaypointManager)
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: WaypointManager found."), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacer %s: WaypointManager not found in level! AI navigation disabled."), *GetName());
        return;
    }

    // Get Navigation System
    NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
    if (NavSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Navigation System found."), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacer %s: Navigation System not found! AI navigation disabled."), *GetName());
        return;
    }

    // Ensure AI Controller exists
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: AI Controller found."), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacer %s: AI Controller not found! AI navigation disabled."), *GetName());
        return;
    }

    // Start navigation
    MoveToNextWaypoint();
}

void AAIRacer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAIRacer::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: Overlapped with a null actor."), *GetName());
        return;
    }

    if (!WaypointManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: WaypointManager is null during overlap."), *GetName());
        return;
    }

    AActor* CurrentWaypoint = WaypointManager->GetWaypoint(CurrentWaypointIndex);
    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: Current waypoint at index %d is null during overlap."), *GetName(), CurrentWaypointIndex);
        return;
    }

    if (OtherActor == CurrentWaypoint)
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Overlapped with waypoint %s at index %d."), *GetName(), *CurrentWaypoint->GetName(), CurrentWaypointIndex);
        CurrentWaypointIndex++;
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Moving to next waypoint index %d."), *GetName(), CurrentWaypointIndex);
        MoveToNextWaypoint();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Overlapped with actor %s, but expected waypoint %s at index %d."),
            *GetName(), *OtherActor->GetName(), *CurrentWaypoint->GetName(), CurrentWaypointIndex);
    }
}

void AAIRacer::MoveToNextWaypoint()
{
    if (!WaypointManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: WaypointManager is null. Cannot move to next waypoint."), *GetName());
        return;
    }

    if (!NavSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: NavSystem is null. Cannot move to next waypoint."), *GetName());
        return;
    }

    AActor* NextWaypoint = WaypointManager->GetWaypoint(CurrentWaypointIndex);
    if (!NextWaypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: Next waypoint at index %d is null."), *GetName(), CurrentWaypointIndex);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Targeting waypoint %s at index %d."), *GetName(), *NextWaypoint->GetName(), CurrentWaypointIndex);

    FNavLocation NavLocation;
    FVector WaypointLocation = NextWaypoint->GetActorLocation();
    FVector Extent(50.f, 50.f, 100.f); // Increased Z extent for projection
    if (!NavSystem->ProjectPointToNavigation(WaypointLocation, NavLocation, Extent))
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: Failed to project waypoint %s to NavMesh. Waypoint location: %s"),
            *GetName(), *NextWaypoint->GetName(), *WaypointLocation.ToString());
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: AI Controller is null. Cannot move to waypoint %s."), *GetName(), *NextWaypoint->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Moving to waypoint %s at NavMesh location %s."),
        *GetName(), *NextWaypoint->GetName(), *NavLocation.Location.ToString());

    EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(NavLocation.Location);
    if (Result != EPathFollowingRequestResult::Type::RequestSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacer %s: Failed to start navigation to waypoint %s. Result: %d"),
            *GetName(), *NextWaypoint->GetName(), static_cast<int32>(Result));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Successfully started navigation to waypoint %s."), *GetName(), *NextWaypoint->GetName());
    }
}