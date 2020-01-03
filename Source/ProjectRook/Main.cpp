// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
#include "Enemy.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "MainPlayerController.h"
#include "ProjectSave.h"
#include "ItemStorage.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InitComponents();
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	//LoadGameNoSwitch();

	if (MainPlayerController)
		MainPlayerController->GameModeOnly();
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsAlive()) return;
	
	const auto DeltaStamina = StaminaDrainRate * DeltaTime;
	
	switch (StaminaStatus)
	{
	case EStaminaStatus::Ess_Normal:

		if (bShiftKeyDown && IsMoving())
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::Ess_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::Ems_Sprinting);
		}
		else // Shift Key Up
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::Ems_Normal);
		}
		break;
	case EStaminaStatus::Ess_BelowMinimum:

		if (bShiftKeyDown && IsMoving())
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::Ess_Exhausted);
				Stamina = .0f;
				SetMovementStatus(EMovementStatus::Ems_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				SetMovementStatus(EMovementStatus::Ems_Sprinting);
			}
		}
		else // Shift Key Up
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::Ess_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::Ems_Normal);
		}
		break;
	case EStaminaStatus::Ess_Exhausted:

		if (bShiftKeyDown && IsMoving())
		{
			Stamina = 0.f;
		}
		else // Shift Key Up
		{
			SetStaminaStatus(EStaminaStatus::Ess_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::Ems_Normal);
		break;
	case EStaminaStatus::Ess_ExhaustedRecovering:

		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::Ess_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::Ems_Normal);
		break;
	default:
		;
	}

	if (bInterpToEnemy && CombatTarget)
	{
		const FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		const FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
	
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::EscKeyDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::EscKeyUp);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	// APawn::AddControllerYaw & PitchInput are used by default Pawn.h
	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookupAtRate);
}

FRotator AMain::GetLookAtRotationYaw(const FVector Target) const
{
	const auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	const FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);

	return LookAtRotationYaw;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;

	if (MovementStatus == EMovementStatus::Ems_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::LMBDown()
{
	bLMBDown = true;

	if (!IsAlive()) return;

	if (MainPlayerController)
		if (MainPlayerController->bIsPauseMenuVisible)
			return;
	
	if (ActiveOverlappingItem)
	{
		const auto Weapon = Cast<AWeapon>(ActiveOverlappingItem);

		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquippedWeapon)
	{
		Attack();
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::EscKeyDown()
{
	bEscDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::EscKeyUp()
{
	bEscDown = false;
}

void AMain::Attack()
{
	if (!bAttacking && IsAlive())
	{
		bAttacking = true;
		SetInterpToEnemy(true);
		
		PlayAnimMontage_P(FName("Attack_1"), 1.55f);
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	if (bLMBDown)
		Attack();
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

bool AMain::CanMove(const float Value) const
{
	if (MainPlayerController)
		return Value != 0.f && !bAttacking && IsAlive() && !MainPlayerController->bIsPauseMenuVisible;

	return false;
}

void AMain::MoveForward(float Value)
{
	if (CanMove(Value))
	{
		// Finds out which way is forward
		const auto Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMain::MoveRight(float Value)
{
	if (CanMove(Value))
	{
		const auto Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

	}
}

void AMain::Turn(float Value)
{
	if (CanMove(Value))
	{
		AddControllerYawInput(Value);
	}
}

void AMain::LookUp(float Value)
{
	if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

bool AMain::IsMoving() const
{
	return GetVelocity().Size() != 0.f ? true : false;
}

void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookupAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookupRate * GetWorld()->GetDeltaSeconds());
}

void AMain::Jump()
{
	if (MainPlayerController)
		if (MainPlayerController->bIsPauseMenuVisible)
			return;
	
	if (IsAlive())
	{
		Super::Jump();
	}
}

void AMain::IncrementCoinValue(const int32 Amount)
{
	Coins += Amount;
}

void AMain::IncrementHealth(const float Amount)
{
	if (Health + Amount >= MaxHealth)
		Health = MaxHealth;
	else
		Health += Amount;
}

void AMain::DecrementHealth(const float Amount)
{

	if (Health - Amount <= 0.f)
		Die();
	else
		Health -= Amount; 
}

void AMain::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	auto ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy)
	{
		const auto Location = GetActorLocation();
		auto MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();
		for (auto Actor : OverlappingActors)
		{
			const auto Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				const auto DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

float AMain::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DecrementHealth(DamageAmount);

	return DamageAmount;
}

bool AMain::IsAlive() const
{
	return (MovementStatus != EMovementStatus::Ems_Dead) ? true: false;
}

void AMain::Die()
{
	if (!IsAlive()) return;
	
	PlayAnimMontage_P(FName("Death"), 1.f);
	SetMovementStatus(EMovementStatus::Ems_Dead);
}

void AMain::PlayAnimMontage_P(const FName MName, const float PlayRate) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(MName, CombatMontage);
	}
}

void AMain::EndAnimation()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::SwitchLevel(const FName LevelName)
{
	const auto World = GetWorld();
	if (World)
	{
		const auto CurrentLevel = World->GetMapName();
		const FName CurrentLevelName(*CurrentLevel);

		if (CurrentLevelName != LevelName)
		{
			SaveGame();
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	auto SaveGameInstance = Cast<UProjectSave>(UGameplayStatics::CreateSaveGameObject(UProjectSave::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coin = Coins;
	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	auto MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;

	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}
	
	UGameplayStatics::SaveGameToSlot
	(
		SaveGameInstance,
		SaveGameInstance->PlayerName,
		SaveGameInstance->UserIndex
	);

	UE_LOG(LogTemp, Warning, TEXT("Game Saved!"))
}

void AMain::LoadGame(bool SetPosition)
{
	auto LoadGameInstance = Cast<UProjectSave>(UGameplayStatics::CreateSaveGameObject(UProjectSave::StaticClass()));

	LoadGameInstance = Cast<UProjectSave>(UGameplayStatics::LoadGameFromSlot
												(
													LoadGameInstance->PlayerName, 
													LoadGameInstance->UserIndex
												));
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coin;

	if (WeaponStorage)
	{
		auto WeaponToRestore = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (WeaponToRestore)
		{
			const auto SavedWeaponName = LoadGameInstance->CharacterStats.WeaponName;
			if (SavedWeaponName != TEXT(""))
			{
				auto WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(WeaponToRestore->WeaponMap[SavedWeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}
	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}
	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		const FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		SwitchLevel(LevelName);
	}

	UE_LOG(LogTemp, Warning, TEXT("Game Loaded!"))
	SetMovementStatus(EMovementStatus::Ems_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void AMain::LoadGameNoSwitch()
{
	auto LoadGameInstance = Cast<UProjectSave>(UGameplayStatics::CreateSaveGameObject(UProjectSave::StaticClass()));

	LoadGameInstance = Cast<UProjectSave>(UGameplayStatics::LoadGameFromSlot
	(
		LoadGameInstance->PlayerName,
		LoadGameInstance->UserIndex
	));
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coin;

	if (WeaponStorage)
	{
		auto WeaponToRestore = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (WeaponToRestore)
		{
			const auto SavedWeaponName = LoadGameInstance->CharacterStats.WeaponName;
			if (SavedWeaponName != TEXT(""))
			{
				auto WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(WeaponToRestore->WeaponMap[SavedWeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Game Loaded!"))
	SetMovementStatus(EMovementStatus::Ems_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}


void AMain::InitComponents()
{
	// Create Camera boom (pulls towards the player if there's a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->RelativeRotation = FRotator(-30.f, 0.f, 0.f);
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller
	CameraBoom->bInheritPitch = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.f;

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(48.f, 100.f);
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Attach the camera to the end of the boom and Let boom adjusts to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookupRate = 65.f;

	// Don't rotate when the controller rotates
	// Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure Character Movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 450.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Health = 90.f;
	MaxHealth = 100.f;
	Stamina = 75.f;
	MaxStamina = 75.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;
	StaminaDrainRate = 5.f;
	MinSprintStamina = 20.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;
	
	bShiftKeyDown = false;
	bLMBDown = false;
	bEscDown = false;

	bHasCombatTarget = false;

	// Initialize Enums
	MovementStatus = EMovementStatus::Ems_Normal;
	StaminaStatus = EStaminaStatus::Ess_Normal;
}

void AMain::ShowPickupLocations()
{
	for (const auto Location: PickupLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 100.f, 12, FLinearColor::White, 5.f);
	}
}
