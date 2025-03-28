// Fill out your copyright notice in the Description page of Project Settings.


#include "StartingMenu_GMB.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AStartingMenu_GMB::BeginPlay()
{

	Super::BeginPlay();

	if (StartingMenuClass)
	{
		CurrentWidget=CreateWidget<UUserWidget>(GetWorld(), StartingMenuClass);
		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
			
		}
	}
}
