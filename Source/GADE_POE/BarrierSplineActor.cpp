// BarrierSplineActor.cpp
#include "BarrierSplineActor.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABarrierSplineActor::ABarrierSplineActor()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = false;

    // Create and setup the spline component
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = Spline;
}

// Called when the game starts or when spawned
void ABarrierSplineActor::BeginPlay()
{
    Super::BeginPlay();
    BuildBarrierSpline();
}

// Called every frame
void ABarrierSplineActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABarrierSplineActor::PostLoad()
{
    Super::PostLoad();
    BuildBarrierSpline();
}

void ABarrierSplineActor::BuildBarrierSpline()
{
    // Cleanup old mesh segments
    for (USplineMeshComponent* MeshComp : SplineMeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->DestroyComponent();
        }
    }
    SplineMeshComponents.Empty();

    // Only proceed if we have enough points
    const int32 NumPoints = Spline->GetNumberOfSplinePoints();
    if (NumPoints < 2) return;

    // Create spline mesh components for each segment
    for (int32 i = 0; i < NumPoints - 1; ++i)
    {
        FVector StartPos = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
        FVector StartTan = Spline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
        FVector EndPos = Spline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
        FVector EndTan = Spline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

        USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
        if (!SplineMesh || !BarrierMesh)
        {
            continue;
        }

        SplineMeshComponents.Add(SplineMesh);
        SplineMesh->SetStaticMesh(BarrierMesh);
        
        // Setup collision
        SetupCollision(SplineMesh);

        SplineMesh->SetStartScale(FVector2D(BarrierScale, BarrierScale));
        SplineMesh->SetEndScale(FVector2D(BarrierScale, BarrierScale));
        SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);

        SplineMesh->RegisterComponent();
        SplineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);

        // Draw debug collision if enabled
        if (bShowDebugCollision)
        {
            FVector MidPoint = (StartPos + EndPos) * 0.5f;
            FVector Direction = (EndPos - StartPos).GetSafeNormal();
            FVector Up = FVector::UpVector;
            FVector Right = FVector::CrossProduct(Direction, Up);

            DrawDebugBox(
                GetWorld(),
                MidPoint,
                FVector(FVector::Dist(StartPos, EndPos) * 0.5f, CollisionThickness * 0.5f, CollisionHeight * 0.5f),
                FQuat::FindBetweenVectors(FVector::ForwardVector, Direction),
                FColor::Red,
                true,
                -1.f,
                0,
                5.f
            );
        }
    }
}

void ABarrierSplineActor::SetupCollision(USplineMeshComponent* SplineMesh)
{
    if (!SplineMesh) return;

    if (bEnableCollision)
    {
        // Enable collision
        SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Set collision profile to BlockAll
        SplineMesh->SetCollisionProfileName(TEXT("BlockAll"));
        
        // Make sure it blocks vehicles and pawns
        SplineMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        SplineMesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
        
        // Set up collision shape
        SplineMesh->SetCollisionObjectType(ECC_WorldStatic);
        SplineMesh->SetGenerateOverlapEvents(true);
    }
    else
    {
        // Disable collision if not wanted
        SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SplineMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        SplineMesh->SetGenerateOverlapEvents(false);
    }

    // Always keep it static
    SplineMesh->SetMobility(EComponentMobility::Static);
}
