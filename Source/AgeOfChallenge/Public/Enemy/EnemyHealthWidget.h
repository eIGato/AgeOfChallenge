#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "EnemyHealthWidget.generated.h"

class UTextBlock;
class UProgressBar;
class AEnemyBase;
class UAttributeComponent;

UCLASS()
class AGEOFCHALLENGE_API UEnemyHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeForEnemy(AEnemyBase* Enemy);

protected:
	UFUNCTION()
	void OnHealthChanged(float CurrentHP, float MaxHP);

private:
	void BuildWidgetTreeIfNeeded();

	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UAttributeComponent> CachedAttributeComponent;
};
