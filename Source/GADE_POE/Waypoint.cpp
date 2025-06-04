#include "Waypoint.h"
#include "Kismet/GameplayStatics.h"
#include "AdvancedRaceManager.h"
#include "Graph.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"

AWaypoint::AWaypoint()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(200.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerSphere->SetGenerateOverlapEvents(true);

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Bind overlap event
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AWaypoint::OnOverlapBegin);
}

void AWaypoint::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("Waypoint %s: BeginPlay called."), *GetName());
}

void AWaypoint::BeginDestroy()
{
    if (UWorld* World = GetWorld())
    {
        AAdvancedRaceManager* Manager = Cast<AAdvancedRaceManager>(
            UGameplayStatics::GetActorOfClass(World, AAdvancedRaceManager::StaticClass()));
        if (Manager && Manager->GetGraph())
        {
            Manager->GetGraph()->RemoveNode(this);
            UE_LOG(LogTemp, Log, TEXT("Waypoint %s: Removed from AdvancedRaceManager graph."), *GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Waypoint %s: Failed to find AdvancedRaceManager or Graph in BeginDestroy."), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Waypoint %s: Null world in BeginDestroy."), *GetName());
    }
    Super::BeginDestroy();
}

void AWaypoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AAIRacer* Racer = Cast<AAIRacer>(OtherActor))
    {
        UE_LOG(LogTemp, Log, TEXT("Waypoint %s: Overlapped by AIRacer %s"), *GetName(), *Racer->GetName());
        if (AAIRacerContoller* Controller = Cast<AAIRacerContoller>(Racer->GetController()))
        {
            Controller->OnWaypointReached(this);
        }
    }
}