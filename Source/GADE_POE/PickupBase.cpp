#include "PickupBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "PlayerHamster.h" // Ensure these are included if not already in your project
#include "AIRacer.h"
#include "TimerManager.h"

APickupBase::APickupBase()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    if (TriggerVolume)
    {
        TriggerVolume->SetupAttachment(RootComponent);
        TriggerVolume->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
        TriggerVolume->SetCollisionProfileName("Trigger");
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnOverlapBegin);
    }
}

void APickupBase::BeginPlay()
{
    Super::BeginPlay();
}

void APickupBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || !OtherActor) return;

    if (OtherActor->IsA(APlayerHamster::StaticClass()) || OtherActor->IsA(AAIRacer::StaticClass()))
    {
        ApplyEffect(OtherActor);
        if (PickupSound) UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
        DeactivatePickup();
    }
}

void APickupBase::DeactivatePickup()
{
    bIsActive = false;
    SetActorHiddenInGame(true);
    GetWorldTimerManager().SetTimer(TimerHandle, this, &APickupBase::ReactivatePickup, EffectDuration, false);
}

void APickupBase::ReactivatePickup()
{
    bIsActive = true;
    SetActorHiddenInGame(false);
}