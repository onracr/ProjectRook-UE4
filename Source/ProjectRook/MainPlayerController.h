// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROOK_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> EnemyHealthBarAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* PauseMenu;
	
	/** Variable to hold the widget after creating it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* HUDOverlay;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* EnemyHealthBar;

	bool bEnemyHealthBarVisible;
	bool bIsPauseMenuVisible;

	FVector EnemyLocation;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = HUD)
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = HUD)
	void RemovePauseMenu();

	void TogglePauseMenu();
	void GameModeOnly();
	
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
