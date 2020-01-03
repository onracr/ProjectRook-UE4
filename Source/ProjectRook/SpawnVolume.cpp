// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "AIController.h"
#include "Enemy.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InitComponent();
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ASpawnVolume::GetSpawnPoint()
{
	const FVector Extent = SpawningBox->GetScaledBoxExtent();
	const FVector Origin = SpawningBox->GetComponentLocation();

	const FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);

	return Point;
}

TSubclassOf<AActor> ASpawnVolume::GetSpawnActors()
{
	if (SpawnArray.Num() > 0)
	{
		const auto Selection = FMath::RandRange(0, SpawnArray.Num() - 1);

		return SpawnArray[Selection];
	}
	else
		return nullptr;
}

void ASpawnVolume::SpawnOurActor_Implementation(UClass* ToSpawn, const FVector& Location)
{
	if (ToSpawn)
	{
		UWorld* World = GetWorld();
		const FActorSpawnParameters SpawnParams;

		if (World)
		{
			const auto Actor = World->SpawnActor<AActor>(ToSpawn, Location, FRotator(0.f), SpawnParams);
		}
	}
}


void ASpawnVolume::InitComponent()
{
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));

}


