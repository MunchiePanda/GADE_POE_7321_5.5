#include "AIRacer.h"
#include "WaypointManager.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

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

    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

    if (WaypointManager)
    {
        MoveToNextWaypoint();
    }
}

void AAIRacer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Bind the overlap event to the OnOverlapBegin function
void AAIRacer::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{ // Check if the overlapping actor is a waypoint
    if (OtherActor && WaypointManager && OtherActor == WaypointManager->GetWaypoint(CurrentWaypointIndex))
    {
        CurrentWaypointIndex++;
        MoveToNextWaypoint();
    }
}

void AAIRacer::MoveToNextWaypoint() // Move to the next waypoint 
{
	// Check if WaypointManager and NavSystem are valid
    if (WaypointManager && NavSystem) 
    {
        AActor* NextWaypoint = WaypointManager->GetWaypoint(CurrentWaypointIndex);
        if (NextWaypoint)
        {
            FNavLocation NavLocation;
            NavSystem->ProjectPointToNavigation(NextWaypoint->GetActorLocation(), NavLocation);
            AAIController* AIController = Cast<AAIController>(GetController());
            if (AIController)
            {
                AIController->MoveToLocation(NavLocation.Location);
            }
        }
    }
}