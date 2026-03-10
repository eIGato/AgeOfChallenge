#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamagingVolume.generated.h"

class UBoxComponent;
class UBillboardComponent;

UCLASS(Blueprintable)
class AGEOFCHALLENGE_API ADamagingVolume : public AActor
{
	GENERATED_BODY()

public:
	ADamagingVolume();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Volume")
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Volume")
	TObjectPtr<UBillboardComponent> Billboard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage Volume", meta = (ClampMin = "0.0"))
	float DamagePerPulse = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage Volume", meta = (ClampMin = "0.05"))
	float DamageInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage Volume")
	TSubclassOf<AActor> AffectedActorClass = AActor::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDrawDebugVolume = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (EditCondition = "bDrawDebugVolume"))
	FColor DebugColor = FColor(255, 70, 70);

private:
	void ApplyPulseDamage();

	FTimerHandle DamageTimerHandle;
};
