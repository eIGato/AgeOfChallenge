#pragma once

#include "BombProjectile.h"
#include "CoreMinimal.h"
#include "FireballProjectile.generated.h"

// Fireball: straight-flying area-damage projectile (like BombProjectile but GravityScale=0).
// Caller sets Damage after spawning to apply level scaling.
UCLASS()
class AGEOFCHALLENGE_API AFireballProjectile : public ABombProjectile
{
	GENERATED_BODY()

public:
	AFireballProjectile();
};
