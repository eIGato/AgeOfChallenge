#include "Projectile/BaseProjectile.h"

#include "Player/AttributeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionSphere;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1500.0f;
	ProjectileMovement->MaxSpeed = 1500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bShouldBounce = false;

	Damage = 10.0f;
	InitialLifeSpan = 10.0f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// OnProjectileStop fires for all geometry types (BSP, static mesh, characters)
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ABaseProjectile::OnProjectileStopped);

	if (AActor* ProjectileOwner = GetOwner())
	{
		CollisionSphere->IgnoreActorWhenMoving(ProjectileOwner, true);
	}
}

void ABaseProjectile::OnProjectileStopped(const FHitResult& ImpactResult)
{
	HandleStop(ImpactResult);
}

void ABaseProjectile::HandleStop(const FHitResult& Hit)
{
	AActor* OtherActor = Hit.GetActor();

	if (OtherActor && OtherActor != this)
	{
		UAttributeComponent* Attr = OtherActor->FindComponentByClass<UAttributeComponent>();
		if (Attr && Attr->bIsAlive)
		{
			Attr->TakeDamage(Damage);
			UE_LOG(LogTemp, Log, TEXT("ABaseProjectile: %s hit %s for %.1f damage"),
			    *GetName(), *OtherActor->GetName(), Damage);
		}
	}

	Destroy();
}
