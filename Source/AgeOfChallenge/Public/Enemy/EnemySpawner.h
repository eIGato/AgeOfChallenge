#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class UBoxComponent;
class AEnemyBase;

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ToolTip = "Relative weight for weighted random selection"))
	float Weight;

	FEnemySpawnEntry() : Weight(1.0f)
	{
	}
};

UCLASS()
class AGEOFCHALLENGE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	float SpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 MaxEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 EnemyLevelMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 EnemyLevelMax;

	// Weighted list of enemy types to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<FEnemySpawnEntry> SpawnEntries;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnZone;

private:
	FTimerHandle SpawnTimerHandle;
	TArray<TWeakObjectPtr<AEnemyBase>> SpawnedEnemies;

	void SpawnEnemy();
	TSubclassOf<AEnemyBase> PickEnemyClass() const;
	bool GetRandomNavPoint(FVector& OutLocation) const;
};
