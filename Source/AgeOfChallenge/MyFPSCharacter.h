#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MyFPSCharacter.generated.h"
#include "TimerManager.h"

class UAttributeComponent;
class UCameraComponent;
class UDamageOverlayWidget;
class UInputMappingContext;
class UInputAction;
class UMyHUDWidget;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttributeComponent> AttributeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY()
	TObjectPtr<UMyHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<UDamageOverlayWidget> DamageOverlayWidget;

	UPROPERTY()
	FTimerHandle RestartPromptTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMyHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDamageOverlayWidget> DamageOverlayWidgetClass;

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

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathAnimationDuration = 1.0f;

	UPROPERTY()
	bool bInputContextAdded = false;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void TryInitializeLocalPlayerUIAndInput();

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHP, float MaxHP);

	void ShowRestartPrompt();
	void HandleRestartRequested();
};
