#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BomberEnemy.generated.h"

class ABombProjectile;

UCLASS()
class AGEOFCHALLENGE_API ABomberEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABomberEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void PerformAttack() override;

	// Projectile class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	TSubclassOf<ABombProjectile> BombClass;

	// Toss speed used for arc calculation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	float BombTossSpeed;

	// Damage dealt by the explosion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	float BombDamage;

	// Explosion radius
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	float BombExplosionRadius;

protected:
	virtual FLinearColor GetEnemyColor() const override;

private:
	float TimeSinceLastAttack;
};
