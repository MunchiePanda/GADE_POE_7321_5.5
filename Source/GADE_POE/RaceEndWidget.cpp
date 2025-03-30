// Fill out your copyright notice in the Description page of Project Settings.


#include "RaceEndWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "RaceHUDWidget.h"


void URaceEndWidget::NativeConstruct()
{
	//Get the buttons and text from the widget
	RestartButton = Cast<UButton>(GetWidgetFromName(TEXT("RestartButton")));
	MainMenuButton = Cast<UButton>(GetWidgetFromName(TEXT("MainMenuButton")));
	RaceTimeText = Cast<UTextBlock>(GetWidgetFromName(TEXT("RaceTimeText")));

	
}

void URaceEndWidget::OnRestartClicked()
{
	//Restart the game
	UGameplayStatics::OpenLevel(GetWorld(), "CheckpointMap");
}

void URaceEndWidget::OnMainMenuClicked()
{
	//Go back to the main menu
	UGameplayStatics::OpenLevel(GetWorld(), "StartingMenu");
}

