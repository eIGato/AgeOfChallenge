#include "Projectile/KnifeProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AKnifeProjectile::AKnifeProjectile()
{
	// Cone mesh — tip points up (+Z) by default, rotate to face forward (+X via Pitch=-90)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone"));
	if (ConeFinder.Succeeded() && ProjectileMesh)
	{
		ProjectileMesh->SetStaticMesh(ConeFinder.Object);
		ProjectileMesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.35f));
		ProjectileMesh->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}

	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(10.0f);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 1800.0f;
		ProjectileMovement->MaxSpeed = 1800.0f;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}

	Damage = 15.0f;
	InitialLifeSpan = 6.0f;
}
