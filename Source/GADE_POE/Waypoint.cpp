#include "Waypoint.h"
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
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AWaypoint::OnOverlapBegin);
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
