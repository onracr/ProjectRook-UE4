// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	Ems_Normal UMETA(DisplayName = "Normal"),
	Ems_Sprinting UMETA(DisplayName = "Sprinting"),
	Ems_Dead UMETA(DisplayName = "Dead"),

	Ems_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	Ess_Normal UMETA(DisplayName = Normal),
	Ess_BelowMinimum UMETA(DisplayName BelowMinimum),
	Ess_Exhausted UMETA(DisplayName = Exhausted),
	Ess_ExhaustedRecovering UMETA(DisplayName = ExhaustedRecovering),

	Ess_Max UMETA(DisplayName = DefaultMax)
};

UCLASS()
class PROJECTROOK_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	TArray<FVector> PickupLocations;
	
	UFUNCTION(BlueprintCallable)
	void ShowPickupLocations();

	UPROPERTY(EditDefaultsOnly, Category = SaveData)
	TSubclassOf<class AItemStorage> WeaponStorage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Enums)
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Enums)
	EStaminaStatus StaminaStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	class AEnemy* CombatTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	TSubclassOf<AEnemy> EnemyFilter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Controller)
	class AMainPlayerController* MainPlayerController;
	
	/** Camera Boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	class UAnimMontage* CombatMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles)
	class USoundCue* HitSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	FVector CombatTargetLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	bool bAttacking;

	/** Base turn rates to scale turning function for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookupRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bHasCombatTarget;

	float InterpSpeed;
	bool bInterpToEnemy;

	/**
	 *
	 * Player Stats
	 * 
	 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float Stamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerStats)
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	int32 Coins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Running)
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Running)
	float SprintingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float MinSprintStamina;

	bool bShiftKeyDown;
	bool bLMBDown;
	bool bEscDown;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Set movement status and running speed */
	void SetMovementStatus(EMovementStatus);

	void LMBDown();
	void LMBUp();
	
	void ShiftKeyDown();
	void ShiftKeyUp();

	void EscKeyDown();
	void EscKeyUp();
	
	/** Called for forwards/ backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called for Yaw Rotation */
	void Turn(float);

	/** Called for Pitch Rotation */
	void LookUp(float);
	
	/** Called via input to turn at a given rate
	 *  @param Rate => Normalized rate, i.e. 1.0 means %100 of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate
	 *  @param Rate => Normalized rate, i.e. 1.0 means %100 of desired look up/down rate
	 */
	void LookupAtRate(float Rate);

	virtual void Jump() override;

	UFUNCTION(BlueprintCallable)
	void IncrementCoinValue(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	void DecrementHealth(float);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void SetInterpToEnemy(bool);
	
	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	bool IsAlive() const;
	
	void Die();
	void UpdateCombatTarget();

	FRotator GetLookAtRotationYaw(FVector) const;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE void SetStaminaStatus(const EStaminaStatus Status) { StaminaStatus = Status; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	FORCEINLINE void SetEquippedWeapon(AWeapon* WeaponToSet) { EquippedWeapon = WeaponToSet; }
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FORCEINLINE void SetHasCombatTarget(const bool HasTarget) { bHasCombatTarget = HasTarget; }

	void SwitchLevel(FName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	void LoadGameNoSwitch();

private:
	void InitComponents();
	void PlayAnimMontage_P(FName, float) const;
	bool IsMoving() const;
	bool CanMove(float) const;

	UFUNCTION(BlueprintCallable)
	void EndAnimation();
};
