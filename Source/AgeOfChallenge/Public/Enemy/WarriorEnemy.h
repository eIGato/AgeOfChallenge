#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "WarriorEnemy.generated.h"

UCLASS()
class AGEOFCHALLENGE_API AWarriorEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	AWarriorEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called by EnemyAIController when in range and LOS
	virtual void PerformAttack() override;

protected:
	virtual FLinearColor GetEnemyColor() const override;

private:
	float TimeSinceLastAttack;

	bool HasLineOfSightTo(const AActor* Target) const;
};
