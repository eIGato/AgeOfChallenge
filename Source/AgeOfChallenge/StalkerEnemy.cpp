#include "StalkerEnemy.h"

#include "AttributeComponent.h"
#include "EnemySettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "KnifeProjectile.h"

AStalkerEnemy::AStalkerEnemy()
{
	KnifeClass = AKnifeProjectile::StaticClass();
	KnifeSpeed = 1800.0f;
	KnifeDamage = 15.0f;
	TimeSinceLastAttack = 0.0f;
}

void AStalkerEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (const UEnemySettings* Settings = GetDefault<UEnemySettings>())
	{
		AttackDamage = Settings->StalkerBaseAttackDamage;
		AttackRange = Settings->StalkerAttackRange;
		AttackCooldown = Settings->StalkerAttackCooldown;
		KnifeSpeed = Settings->StalkerKnifeSpeed;
		KnifeDamage = Settings->StalkerBaseAttackDamage;

		if (AttributeComponent)
		{
			AttributeComponent->MaxHP = Settings->BaseMaxHP;
			AttributeComponent->CurrentHP = AttributeComponent->MaxHP;
		}
	}
}

void AStalkerEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeSinceLastAttack += DeltaTime;
}

void AStalkerEnemy::PerformAttack()
{
	if (!AttributeComponent || !AttributeComponent->bIsAlive)
	{
		return;
	}

	if (TimeSinceLastAttack < AttackCooldown)
	{
		return;
	}

	if (!KnifeClass)
	{
		return;
	}

	ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerChar)
	{
		return;
	}

	if (!HasLineOfSightTo(PlayerChar))
	{
		return;
	}

	// Spawn position: a bit in front and at eye height
	const FVector SpawnOffset = GetActorForwardVector() * 80.0f + FVector(0.0f, 0.0f, 50.0f);
	const FVector SpawnLocation = GetActorLocation() + SpawnOffset;

	const FVector TargetLocation = PlayerChar->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
	const FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AKnifeProjectile* Knife = GetWorld()->SpawnActor<AKnifeProjectile>(
	    KnifeClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Knife)
	{
		Knife->Damage = KnifeDamage;
		// Override initial velocity to match configured speed
		if (UProjectileMovementComponent* ProjMove = Knife->FindComponentByClass<UProjectileMovementComponent>())
		{
			ProjMove->InitialSpeed = KnifeSpeed;
			ProjMove->MaxSpeed = KnifeSpeed;
			ProjMove->Velocity = Direction * KnifeSpeed;
		}

		UE_LOG(LogTemp, Log, TEXT("AStalkerEnemy: %s fired knife toward %s"),
		    *GetName(), *PlayerChar->GetName());

		TimeSinceLastAttack = 0.0f;
	}
}

FLinearColor AStalkerEnemy::GetEnemyColor() const
{
	return FLinearColor(0.0f, 0.8f, 0.1f); // Green
}

bool AStalkerEnemy::HasLineOfSightTo(const AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Target);

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	return !bBlocked;
}
