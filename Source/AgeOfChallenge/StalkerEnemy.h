#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "StalkerEnemy.generated.h"

class AKnifeProjectile;

UCLASS()
class AGEOFCHALLENGE_API AStalkerEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	AStalkerEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void PerformAttack() override;

	// Projectile class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	TSubclassOf<AKnifeProjectile> KnifeClass;

	// Launch speed of spawned knife
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	float KnifeSpeed;

	// Damage dealt by each knife
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	float KnifeDamage;

protected:
	virtual FLinearColor GetEnemyColor() const override;

private:
	float TimeSinceLastAttack;

	bool HasLineOfSightTo(const AActor* Target) const;
};
