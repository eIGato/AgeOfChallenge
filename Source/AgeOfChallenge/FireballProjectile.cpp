#include "FireballProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AFireballProjectile::AFireballProjectile()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 1400.0f;
		ProjectileMovement->MaxSpeed = 1400.0f;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
		ProjectileMovement->bRotationFollowsVelocity = false;
	}

	if (ProjectileMesh)
	{
		// Slightly larger, orange-tinted mesh (tint applied via Dynamic Material in Blueprint, or just scale)
		ProjectileMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.45f));
	}

	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(20.0f);
	}

	Damage = 40.0f;
	ExplosionRadius = 350.0f;
	InitialLifeSpan = 6.0f;
}
