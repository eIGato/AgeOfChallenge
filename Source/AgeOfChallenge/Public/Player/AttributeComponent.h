#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "AttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedSignature, float, CurrentMP, float, MaxMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEXPChangedSignature, int32, CurrentEXP, int32, EXPToNextLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpSignature, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AGEOFCHALLENGE_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float TakeDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool SpendMP(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void AddEXP(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void Die();

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnEXPChangedSignature OnEXPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnLevelUpSignature OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnDeathSignature OnDeath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Health")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Health")
	float CurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Mana")
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Mana")
	float CurrentMP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Progression")
	int32 CurrentEXP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Progression")
	int32 EXPToNextLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Progression")
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Regen")
	float HPRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Regen")
	float MPRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|State")
	bool bIsAlive;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetHealthRatio() const;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetManaRatio() const;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetEXPRatio() const;

private:
	UPROPERTY(EditAnywhere, Category = "Attributes|Progression")
	float EXPGrowthMultiplier;

	void LoadValuesFromConfig();
	void BroadcastAll();
	void LevelUp();
};
