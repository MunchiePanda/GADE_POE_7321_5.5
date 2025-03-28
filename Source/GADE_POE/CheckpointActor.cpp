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
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Checkpoint Passed!"));

        // Find CheckpointManager in the level
        for (TActorIterator<ACheckpointManager> It(GetWorld()); It; ++It)
        {
            ACheckpointManager* CheckpointManager = *It;
            if (CheckpointManager)
            {
                CheckpointManager->PlayerReachedCheckpoint();
                return;
            }
        }

        UE_LOG(LogTemp, Error, TEXT("CheckpointManager not found in the level!"));
    }
}
