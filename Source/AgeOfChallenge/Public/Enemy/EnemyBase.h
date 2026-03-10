#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UAttributeComponent;
class UWidgetComponent;
class UEnemyHealthWidget;

UCLASS(Abstract)
class AGEOFCHALLENGE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Properties ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FString EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float EXPRewardPerLevel;

	// Called by EnemySpawner after spawn
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetLevel(int32 NewLevel);

	// Override in subclass to perform the actual attack
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	virtual void PerformAttack()
	{
	}

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UAttributeComponent* GetAttributeComponent() const
	{
		return AttributeComponent;
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Death")
	float DeathTiltDuration;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Death")
	float DeathDestroyDelay;

	virtual FLinearColor GetEnemyColor() const;

	UFUNCTION()
	void HandleDeath();

private:
	bool bIsDying;
	float DeathAnimElapsed;
	float DeathTiltDirection;
	FRotator DeathStartRotation;

	FTimerHandle DestroyTimerHandle;

	void ApplyLevelScaling(int32 NewLevel);
	void DestroyEnemy();
};
