#include "Projectile/BombProjectile.h"

#include "Player/AttributeComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ABombProjectile::ABombProjectile()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded() && ProjectileMesh)
	{
		ProjectileMesh->SetStaticMesh(SphereFinder.Object);
		ProjectileMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
	}

	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(18.0f);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 900.0f;
		ProjectileMovement->MaxSpeed = 1400.0f;
		ProjectileMovement->ProjectileGravityScale = 1.0f;
		ProjectileMovement->bRotationFollowsVelocity = false;
	}

	Damage = 30.0f;
	ExplosionRadius = 300.0f;
	InitialLifeSpan = 8.0f;
}

void ABombProjectile::HandleStop(const FHitResult& Hit)
{
	const FVector ExplosionCenter = GetActorLocation();

	AActor* TriggerActor = Hit.GetActor();
	UE_LOG(LogTemp, Warning, TEXT("ABombProjectile: Explosion at %s, radius=%.1f, triggered by %s"),
	    *ExplosionCenter.ToString(), ExplosionRadius,
	    TriggerActor ? *TriggerActor->GetName() : TEXT("none"));

	// Area overlap — find all Pawns in radius
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UKismetSystemLibrary::SphereOverlapActors(
	    this,
	    ExplosionCenter,
	    ExplosionRadius,
	    ObjectTypes,
	    nullptr,
	    IgnoredActors,
	    OverlappedActors);

	for (AActor* Target : OverlappedActors)
	{
		if (!Target)
		{
			continue;
		}

		UAttributeComponent* Attr = Target->FindComponentByClass<UAttributeComponent>();
		if (Attr && Attr->bIsAlive)
		{
			Attr->TakeDamage(Damage);
			UE_LOG(LogTemp, Log, TEXT("ABombProjectile: Explosion hit %s for %.1f damage"),
			    *Target->GetName(), Damage);
		}
	}

	if (GetWorld())
	{
		DrawDebugSphere(GetWorld(), ExplosionCenter, ExplosionRadius, 16, FColor::Red, false, 2.0f);
	}

	Destroy();
}
