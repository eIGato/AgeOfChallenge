#include "Enemy/EnemySpawner.h"

#include "Enemy/BomberEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/EnemySettings.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Enemy/StalkerEnemy.h"
#include "Enemy/WarriorEnemy.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnZone = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnZone"));
	SpawnZone->SetBoxExtent(FVector(500.0f, 500.0f, 100.0f));
	SpawnZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SpawnZone;

	// Hardcoded defaults; BeginPlay will use EnemySettings if configured
	SpawnInterval = 5.0f;
	MaxEnemies = 5;
	EnemyLevelMin = 1;
	EnemyLevelMax = 3;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Apply settings defaults (only if not overridden per-instance in the editor)
	if (const UEnemySettings* Settings = GetDefault<UEnemySettings>())
	{
		if (SpawnInterval <= 0.0f)
			SpawnInterval = Settings->DefaultSpawnInterval;
		if (MaxEnemies <= 0)
			MaxEnemies = Settings->DefaultMaxEnemies;
		if (EnemyLevelMin <= 0)
			EnemyLevelMin = Settings->DefaultEnemyLevelMin;
		if (EnemyLevelMax <= 0)
			EnemyLevelMax = Settings->DefaultEnemyLevelMax;
	}

	// No Blueprint needed: populate default entries when nothing is configured.
	if (SpawnEntries.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("AEnemySpawner [%s]: No SpawnEntries configured — using default mix."), *GetName());

		FEnemySpawnEntry WarriorEntry;
		WarriorEntry.EnemyClass = AWarriorEnemy::StaticClass();
		WarriorEntry.Weight = 3.0f;
		SpawnEntries.Add(WarriorEntry);

		FEnemySpawnEntry StalkerEntry;
		StalkerEntry.EnemyClass = AStalkerEnemy::StaticClass();
		StalkerEntry.Weight = 2.0f;
		SpawnEntries.Add(StalkerEntry);

		FEnemySpawnEntry BomberEntry;
		BomberEntry.EnemyClass = ABomberEnemy::StaticClass();
		BomberEntry.Weight = 1.0f;
		SpawnEntries.Add(BomberEntry);
	}

	// Initial delay = SpawnInterval so the level has time to load
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, SpawnInterval, true, SpawnInterval);
}

void AEnemySpawner::SpawnEnemy()
{
	// Remove stale (dead/destroyed) references
	SpawnedEnemies.RemoveAll([](const TWeakObjectPtr<AEnemyBase>& Ref)
	    { return !Ref.IsValid(); });

	if (SpawnedEnemies.Num() >= MaxEnemies)
	{
		return;
	}

	TSubclassOf<AEnemyBase> EnemyClass = PickEnemyClass();
	if (!EnemyClass)
	{
		return;
	}

	FVector SpawnLocation;
	if (!GetRandomNavPoint(SpawnLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemySpawner [%s]: No valid NavMesh point found — skipping spawn."), *GetName());
		return;
	}

	// NavMesh Z is the floor surface; ACharacter origin is the capsule center, so lift by half-height
	if (const ACharacter* CDO = Cast<ACharacter>(EnemyClass->GetDefaultObject()))
	{
		SpawnLocation.Z += CDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (!Enemy)
	{
		return;
	}

	const int32 Level = FMath::RandRange(EnemyLevelMin, EnemyLevelMax);
	Enemy->SetLevel(Level);
	SpawnedEnemies.Add(Enemy);

	UE_LOG(LogTemp, Log, TEXT("AEnemySpawner [%s]: Spawned %s (Lv.%d) at %s"),
	    *GetName(), *Enemy->EnemyName, Level, *SpawnLocation.ToString());
}

TSubclassOf<AEnemyBase> AEnemySpawner::PickEnemyClass() const
{
	float TotalWeight = 0.0f;
	for (const FEnemySpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.EnemyClass)
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (const FEnemySpawnEntry& Entry : SpawnEntries)
	{
		if (!Entry.EnemyClass)
		{
			continue;
		}
		Roll -= Entry.Weight;
		if (Roll <= 0.0f)
		{
			return Entry.EnemyClass;
		}
	}

	// Fallback (floating-point safety)
	return SpawnEntries.Last().EnemyClass;
}

bool AEnemySpawner::GetRandomNavPoint(FVector& OutLocation) const
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return false;
	}

	const FBox Box = SpawnZone->Bounds.GetBox();

	for (int32 Attempt = 0; Attempt < 10; ++Attempt)
	{
		const FVector RandomPoint = FMath::RandPointInBox(Box);
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(RandomPoint, NavLoc))
		{
			OutLocation = NavLoc.Location;
			return true;
		}
	}

	return false;
}
