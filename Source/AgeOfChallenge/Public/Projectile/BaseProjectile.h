#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(Abstract)
class AGEOFCHALLENGE_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float Damage;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	// Regular C++ virtual — subclasses override this for custom hit behavior
	virtual void HandleStop(const FHitResult& Hit);

private:
	// Non-virtual UFUNCTION wrapper bound to OnProjectileStop delegate
	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);
};
