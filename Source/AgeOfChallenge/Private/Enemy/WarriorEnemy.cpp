#include "Enemy/WarriorEnemy.h"

#include "Player/AttributeComponent.h"
#include "Enemy/EnemySettings.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

AWarriorEnemy::AWarriorEnemy()
{
	TimeSinceLastAttack = 0.0f;
}

void AWarriorEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (const UEnemySettings* Settings = GetDefault<UEnemySettings>())
	{
		AttackDamage = Settings->WarriorBaseAttackDamage;
		AttackRange = Settings->WarriorAttackRange;
		AttackCooldown = Settings->WarriorAttackCooldown;

		if (AttributeComponent)
		{
			AttributeComponent->MaxHP = Settings->BaseMaxHP;
			AttributeComponent->CurrentHP = AttributeComponent->MaxHP;
		}
	}
}

void AWarriorEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeSinceLastAttack += DeltaTime;
}

void AWarriorEnemy::PerformAttack()
{
	if (!AttributeComponent || !AttributeComponent->bIsAlive)
	{
		return;
	}

	if (TimeSinceLastAttack < AttackCooldown)
	{
		return;
	}

	// Sphere overlap — catches both player and other enemies
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UKismetSystemLibrary::SphereOverlapActors(
	    this,
	    GetActorLocation(),
	    AttackRange,
	    ObjectTypes,
	    ACharacter::StaticClass(),
	    IgnoredActors,
	    OverlappedActors);

	bool bHitAnything = false;

	for (AActor* Target : OverlappedActors)
	{
		ACharacter* TargetChar = Cast<ACharacter>(Target);
		if (!TargetChar)
		{
			continue;
		}

		if (!HasLineOfSightTo(Target))
		{
			continue;
		}

		UAttributeComponent* TargetAttr = TargetChar->FindComponentByClass<UAttributeComponent>();
		if (TargetAttr && TargetAttr->bIsAlive)
		{
			TargetAttr->TakeDamage(AttackDamage);
			UE_LOG(LogTemp, Log, TEXT("AWarriorEnemy: %s hit %s for %.1f damage"),
			    *GetName(), *Target->GetName(), AttackDamage);
			bHitAnything = true;
		}
	}

	if (bHitAnything)
	{
		TimeSinceLastAttack = 0.0f;
	}
}

FLinearColor AWarriorEnemy::GetEnemyColor() const
{
	return FLinearColor::Red;
}

bool AWarriorEnemy::HasLineOfSightTo(const AActor* Target) const
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
