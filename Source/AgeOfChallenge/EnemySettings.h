#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EnemySettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Enemy Settings"))
class AGEOFCHALLENGE_API UEnemySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEnemySettings();

	// --- Base ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Base", meta = (ToolTip = "Base MaxHP at level 1"))
	float BaseMaxHP = 50.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Base", meta = (ToolTip = "EXP awarded = Level * EXPRewardPerLevel"))
	float EXPRewardPerLevel = 10.0f;

	// --- Per-level scaling ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Scaling", meta = (ToolTip = "Additional MaxHP per level above 1"))
	float HPPerLevel = 20.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Scaling", meta = (ToolTip = "Additional AttackDamage per level above 1"))
	float DamagePerLevel = 2.0f;

	// --- Warrior ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Warrior")
	float WarriorBaseAttackDamage = 10.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Warrior")
	float WarriorAttackRange = 150.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Warrior")
	float WarriorAttackCooldown = 1.5f;

	// --- Spawner defaults ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Spawner")
	float DefaultSpawnInterval = 5.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Spawner")
	int32 DefaultMaxEnemies = 5;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Spawner")
	int32 DefaultEnemyLevelMin = 1;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Spawner")
	int32 DefaultEnemyLevelMax = 3;

	// --- Stalker ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Stalker")
	float StalkerBaseAttackDamage = 15.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Stalker")
	float StalkerAttackRange = 600.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Stalker")
	float StalkerAttackCooldown = 2.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Stalker", meta = (ToolTip = "Launch speed of spawned knife"))
	float StalkerKnifeSpeed = 1800.0f;

	// --- Bomber ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Bomber")
	float BomberBaseAttackDamage = 30.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Bomber")
	float BomberAttackRange = 1200.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Bomber")
	float BomberAttackCooldown = 3.5f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Bomber", meta = (ToolTip = "Toss speed for arc calculation"))
	float BomberBombTossSpeed = 900.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Bomber", meta = (ToolTip = "Explosion area radius"))
	float BomberExplosionRadius = 300.0f;

	// --- Death ---
	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Death", meta = (ToolTip = "Seconds to tilt 90 degrees on death"))
	float DeathTiltDuration = 0.5f;

	UPROPERTY(Config, EditAnywhere, Category = "Enemy|Death", meta = (ToolTip = "Seconds after death before actor is destroyed"))
	float DeathDestroyDelay = 3.0f;
};
