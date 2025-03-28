// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu_GMB.h"

void AMainMenu_GMB::BeginPlay()
{

	if (MainMenuClass)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuClass);
		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
		}
	}
}
