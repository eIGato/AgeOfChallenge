#include "Enemy/EnemyAIController.h"

#include "Enemy/EnemyBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	AIUpdateInterval = 0.2f;
	TimeSinceAIUpdate = 0.0f;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<AEnemyBase>(InPawn);
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();
	StopMovement();
	ControlledEnemy = nullptr;
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ControlledEnemy)
	{
		return;
	}

	TimeSinceAIUpdate += DeltaTime;
	if (TimeSinceAIUpdate >= AIUpdateInterval)
	{
		TimeSinceAIUpdate = 0.0f;
		UpdateAI();
	}
}

void AEnemyAIController::UpdateAI()
{
	if (!ControlledEnemy)
	{
		return;
	}

	// Cache player reference once
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	if (!PlayerCharacter)
	{
		return;
	}

	const float DistToPlayer = FVector::Dist(ControlledEnemy->GetActorLocation(), PlayerCharacter->GetActorLocation());
	const float AttackRange = ControlledEnemy->AttackRange;

	if (DistToPlayer <= AttackRange && HasLineOfSightToPlayer())
	{
		// In range + LOS: stop and attack
		StopMovement();
		ControlledEnemy->PerformAttack();
	}
	else
	{
		// Chase player; stop a bit inside attack range to avoid overshooting
		MoveToActor(PlayerCharacter, AttackRange * 0.75f);
	}
}

bool AEnemyAIController::HasLineOfSightToPlayer() const
{
	if (!ControlledEnemy || !PlayerCharacter)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledEnemy);
	Params.AddIgnoredActor(PlayerCharacter);

	const FVector Start = ControlledEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector End = PlayerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	return !bBlocked;
}
