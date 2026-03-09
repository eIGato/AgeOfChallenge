#include "BomberEnemy.h"

#include "AttributeComponent.h"
#include "BombProjectile.h"
#include "EnemySettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABomberEnemy::ABomberEnemy()
{
	BombClass = ABombProjectile::StaticClass();
	BombTossSpeed = 900.0f;
	BombDamage = 30.0f;
	BombExplosionRadius = 300.0f;
	TimeSinceLastAttack = 0.0f;
}

void ABomberEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (const UEnemySettings* Settings = GetDefault<UEnemySettings>())
	{
		AttackDamage = Settings->BomberBaseAttackDamage;
		AttackRange = Settings->BomberAttackRange;
		AttackCooldown = Settings->BomberAttackCooldown;
		BombTossSpeed = Settings->BomberBombTossSpeed;
		BombDamage = Settings->BomberBaseAttackDamage;
		BombExplosionRadius = Settings->BomberExplosionRadius;

		if (AttributeComponent)
		{
			AttributeComponent->MaxHP = Settings->BaseMaxHP;
			AttributeComponent->CurrentHP = AttributeComponent->MaxHP;
		}
	}
}

void ABomberEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeSinceLastAttack += DeltaTime;
}

void ABomberEnemy::PerformAttack()
{
	if (!AttributeComponent || !AttributeComponent->bIsAlive)
	{
		return;
	}

	if (TimeSinceLastAttack < AttackCooldown)
	{
		return;
	}

	if (!BombClass)
	{
		return;
	}

	ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerChar)
	{
		return;
	}

	// Spawn position: well above and in front of the enemy to prevent immediate self-hit
	const FVector SpawnOffset = GetActorForwardVector() * 100.0f + FVector(0.0f, 0.0f, 120.0f);
	const FVector SpawnLocation = GetActorLocation() + SpawnOffset;
	const FVector TargetLocation = PlayerChar->GetActorLocation();

	// Calculate arc launch velocity using UE's built-in suggestion
	// (suppress deprecation: FSuggestProjectileVelocityParameters not yet available in this UE build)
	FVector TossVelocity = FVector::ZeroVector;
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const bool bFoundSolution = UGameplayStatics::SuggestProjectileVelocity(
	    this,
	    TossVelocity,
	    SpawnLocation,
	    TargetLocation,
	    BombTossSpeed,
	    true, // favor high arc
	    0.0f,
	    0.0f,
	    ESuggestProjVelocityTraceOption::DoNotTrace);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	if (!bFoundSolution)
	{
		// Fallback: direct lob with a fixed upward angle
		const FVector ToTarget = (TargetLocation - SpawnLocation).GetSafeNormal();
		TossVelocity = (ToTarget + FVector(0.0f, 0.0f, 0.7f)).GetSafeNormal() * BombTossSpeed;
		UE_LOG(LogTemp, Warning, TEXT("ABomberEnemy: SuggestProjectileVelocity failed, using fallback lob"));
	}

	const FRotator SpawnRotation = TossVelocity.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABombProjectile* Bomb = GetWorld()->SpawnActor<ABombProjectile>(
	    BombClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Bomb)
	{
		Bomb->Damage = BombDamage;
		Bomb->ExplosionRadius = BombExplosionRadius;

		if (UProjectileMovementComponent* ProjMove = Bomb->FindComponentByClass<UProjectileMovementComponent>())
		{
			ProjMove->Velocity = TossVelocity;
			ProjMove->InitialSpeed = TossVelocity.Size();
			ProjMove->MaxSpeed = BombTossSpeed * 1.5f;
		}

		UE_LOG(LogTemp, Log, TEXT("ABomberEnemy: %s lobbed bomb toward %s (vel=%.1f)"),
		    *GetName(), *PlayerChar->GetName(), TossVelocity.Size());

		TimeSinceLastAttack = 0.0f;
	}
}

FLinearColor ABomberEnemy::GetEnemyColor() const
{
	return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
}
