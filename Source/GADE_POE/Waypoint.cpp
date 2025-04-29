#include "Waypoint.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"

// Sets default values
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


// Called when the game starts or when spawned
void AWaypoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult) // Called when the trigger sphere overlaps with another actor
{
    if (AAIRacer* Racer = Cast<AAIRacer>(OtherActor)) // Check if the other actor is an AI Racer
    {
        AController* Controller = Racer->GetController();
        if (AAIRacerContoller* AIRacerContoller = Cast<AAIRacerContoller>(Controller))
        {
            AIRacerContoller->OnWaypointReached(this); // Call the OnWaypointReached function
        }
    }
}
