#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "MyFPSCharacter.generated.h"

class UAttributeComponent;
class UCameraComponent;
class UDamageOverlayWidget;
class UInputMappingContext;
class UInputAction;
class UMyHUDWidget;
class UActionBarWidget;
class UHealFlashWidget;
class AKnifeProjectile;
class AFireballProjectile;

UENUM(BlueprintType)
enum class EPlayerCombatMode : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged"),
};

UCLASS()
class AGEOFCHALLENGE_API AMyFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyFPSCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	// -----------------------------------------------------------------------
	// Components
	// -----------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttributeComponent> AttributeComponent;

	// -----------------------------------------------------------------------
	// Input
	// -----------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SwitchMeleeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SwitchRangedAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HealAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireballAction;

	// -----------------------------------------------------------------------
	// UI widgets
	// -----------------------------------------------------------------------
	UPROPERTY()
	TObjectPtr<UMyHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<UDamageOverlayWidget> DamageOverlayWidget;

	UPROPERTY()
	TObjectPtr<UActionBarWidget> ActionBarWidget;

	UPROPERTY()
	TObjectPtr<UHealFlashWidget> HealFlashWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMyHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDamageOverlayWidget> DamageOverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UActionBarWidget> ActionBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHealFlashWidget> HealFlashWidgetClass;

	// -----------------------------------------------------------------------
	// Projectile classes
	// -----------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ranged")
	TSubclassOf<AKnifeProjectile> KnifeProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fireball")
	TSubclassOf<AFireballProjectile> FireballProjectileClass;

	// -----------------------------------------------------------------------
	// Combat — melee
	// -----------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Melee")
	float MeleeDamage = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Melee")
	float MeleeRange = 180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Melee")
	float MeleeRadius = 55.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Melee")
	float MeleeCooldown = 0.6f;

	// -----------------------------------------------------------------------
	// Combat — ranged knife
	// -----------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ranged")
	float KnifeCooldown = 0.8f;

	// -----------------------------------------------------------------------
	// Combat — heal spell
	// -----------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heal")
	float HealBaseAmount = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heal")
	float HealAmountPerLevel = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heal")
	float HealBaseMPCost = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heal")
	float HealMPCostPerLevel = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heal")
	float HealCooldown = 3.0f;

	// -----------------------------------------------------------------------
	// Combat — fireball spell
	// -----------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fireball")
	float FireballBaseDamage = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fireball")
	float FireballDamagePerLevel = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fireball")
	float FireballBaseMPCost = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fireball")
	float FireballMPCostPerLevel = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fireball")
	float FireballCooldown = 4.0f;

	// -----------------------------------------------------------------------
	// Runtime combat state
	// -----------------------------------------------------------------------
	UPROPERTY()
	EPlayerCombatMode CombatMode = EPlayerCombatMode::Melee;

	// Slot layout: 0=Heal, 1=Fireball, 2=Melee, 3=Ranged
	float SlotCooldownRemaining[4] = {};
	float SlotCooldownTotal[4] = {};

	// -----------------------------------------------------------------------
	// Death / restart state
	// -----------------------------------------------------------------------
	UPROPERTY()
	FTimerHandle RestartPromptTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathAnimationDuration = 1.0f;

	UPROPERTY()
	bool bIsDying = false;

	UPROPERTY()
	bool bCanRestart = false;

	UPROPERTY()
	float DeathAnimationElapsed = 0.0f;

	UPROPERTY()
	FVector DeathCameraStartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector DeathCameraTargetLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator DeathCameraStartRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FRotator DeathCameraTargetRotation = FRotator::ZeroRotator;

	UPROPERTY()
	bool bInputContextAdded = false;

	// -----------------------------------------------------------------------
	// Methods
	// -----------------------------------------------------------------------
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void TryInitializeLocalPlayerUIAndInput();

	void PerformAttack(const FInputActionValue& Value);
	void SwitchToMelee(const FInputActionValue& Value);
	void SwitchToRanged(const FInputActionValue& Value);
	void CastHeal(const FInputActionValue& Value);
	void CastFireball(const FInputActionValue& Value);

	void DoMeleeAttack();
	void DoRangedAttack();

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHP, float MaxHP);

	void ShowRestartPrompt();
	void HandleRestartRequested();
};
