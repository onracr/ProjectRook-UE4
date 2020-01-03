// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemStorage.generated.h"

UCLASS()
class PROJECTROOK_API AItemStorage : public AActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, Category = SaveDate)
	TMap<FString, TSubclassOf<class AWeapon>> WeaponMap;
	
	AItemStorage();

protected:
	virtual void BeginPlay() override;

public:	

};
