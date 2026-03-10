#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "EnemyAIController.generated.h"

class AEnemyBase;

UCLASS()
class AGEOFCHALLENGE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// How often (seconds) the AI logic updates
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float AIUpdateInterval;

private:
	TObjectPtr<AEnemyBase> ControlledEnemy;
	TObjectPtr<ACharacter> PlayerCharacter;
	float TimeSinceAIUpdate;

	void UpdateAI();
	bool HasLineOfSightToPlayer() const;
};
