#include "Waypoint.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"
#include "AdvancedRaceManager.h"
#include "Kismet/GameplayStatics.h"

AWaypoint::AWaypoint()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(200.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AWaypoint::OnOverlapBegin);
}

void AWaypoint::BeginPlay()
{
    Super::BeginPlay();
}

void AWaypoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AAIRacer* Racer = Cast<AAIRacer>(OtherActor))
    {
        AController* Controller = Racer->GetController();
        if (AAIRacerContoller* AIRacerContoller = Cast<AAIRacerContoller>(Controller))
        {
            AIRacerContoller->OnWaypointReached(this);
        }
    }
}

void AWaypoint::BeginDestroy()
{
    //UE_LOG(LogTemp, Log, TEXT("Waypoint::BeginDestroy - %s, IsPendingKill: %d"), *GetName(), IsPendingKill());
    if (AAdvancedRaceManager* RaceManager = Cast<AAdvancedRaceManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass())))
    {
        if (AGraph* Graph = RaceManager->GetGraph())
        {
            Graph->RemoveNode(this);
            UE_LOG(LogTemp, Log, TEXT("Waypoint::BeginDestroy - Removed %s from graph"), *GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Waypoint::BeginDestroy - Failed to get Graph from AdvancedRaceManager"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Waypoint::BeginDestroy - Failed to find AdvancedRaceManager"));
    }
    Super::BeginDestroy();
}