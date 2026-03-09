#pragma once

#include "BaseProjectile.h"
#include "CoreMinimal.h"
#include "BombProjectile.generated.h"

UCLASS()
class AGEOFCHALLENGE_API ABombProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:
	ABombProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Bomb")
	float ExplosionRadius;

protected:
	virtual void HandleStop(const FHitResult& Hit) override;
};
