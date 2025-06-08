// Fill out your copyright notice in the Description page of Project Settings.


#include "AdvancedRaceGMB.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

AAdvancedRaceGMB::AAdvancedRaceGMB()
{
    // Set default values
    PrimaryActorTick.bCanEverTick = false;
}

void AAdvancedRaceGMB::BeginPlay()
{
    Super::BeginPlay();

    // Initialize all race components
    InitializeRaceComponents();

    // Setup the race UI
    SetupRaceUI();
}

void AAdvancedRaceGMB::InitializeRaceComponents()
{
    // Create the Graph if GraphClass is set
    if (GraphClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        Graph = GetWorld()->SpawnActor<AGraph>(GraphClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (Graph)
        {
            UE_LOG(LogTemp, Log, TEXT("AdvancedRaceGMB: Graph created successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AdvancedRaceGMB: Failed to create Graph"));
        }
    }

    // Create the RaceManager if RaceManagerClass is set
    if (RaceManagerClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        RaceManager = GetWorld()->SpawnActor<AAdvancedRaceManager>(RaceManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (RaceManager)
        {
            // Initialize the race manager with the track and graph
            RaceManager->InitializeTrack(RaceTrack, Graph);
            UE_LOG(LogTemp, Log, TEXT("AdvancedRaceGMB: RaceManager created and initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AdvancedRaceGMB: Failed to create RaceManager"));
        }
    }
}

void AAdvancedRaceGMB::SetupRaceUI()
{
    // Create and display the race HUD
    if (RaceHUDClass)
    {
        UUserWidget* RaceHUD = CreateWidget<UUserWidget>(GetWorld(), RaceHUDClass);
        if (RaceHUD)
        {
            RaceHUD->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("AdvancedRaceGMB: Race HUD created successfully"));
        }
    }
}

