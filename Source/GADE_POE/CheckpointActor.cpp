#include "CheckpointActor.h"
#include "CheckpointManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
// Sets default values
ACheckpointActor::ACheckpointActor()
{
    PrimaryActorTick.bCanEverTick = true;


    // Create an indicator (e.g., floating arrow, glow effect)
    IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
    IndicatorMesh->SetupAttachment(RootComponent);
    IndicatorMesh->SetVisibility(true); // Initially hidden

    // Bind overlap event
    OnActorBeginOverlap.AddDynamic(this, &ACheckpointActor::OnPlayerEnterCheckpoint);
}

// Called when the game starts or when spawned
void ACheckpointActor::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void ACheckpointActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACheckpointActor::OnPlayerEnterCheckpoint(AActor* OverlappedActor, AActor* OtherActor)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass())) // Check if the other actor is a character
    {
        UE_LOG(LogTemp, Warning, TEXT("Checkpoint Passed!"));

        // Find CheckpointManager in the level
        for (TActorIterator<ACheckpointManager> It(GetWorld()); It; ++It) // Find CheckpointManager in the level
        {
            ACheckpointManager* CheckpointManager = *It;
            if (CheckpointManager)
            {
                CheckpointManager->PlayerReachedCheckpoint(); // Notify CheckpointManager
                return;
            }
        }

        UE_LOG(LogTemp, Error, TEXT("CheckpointManager not found in the level!"));
    }
}

void ACheckpointActor::SetCheckpointState(bool bIsNextCheckpoint, bool bIsPassed)
{
    if (bIsNextCheckpoint)
    {
        IndicatorMesh->SetVisibility(true);
        IndicatorMesh->SetMaterial(0, YellowMaterial); // Show yellow for next checkpoint
    }
    else if (bIsPassed)
    {
        IndicatorMesh->SetVisibility(true);
        IndicatorMesh->SetMaterial(0, GreenMaterial); // Show green for passed checkpoint
    }
}
