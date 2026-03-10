#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "MyHUDWidget.generated.h"

class UAttributeComponent;
class UBorder;
class UProgressBar;
class UTextBlock;

UCLASS()
class AGEOFCHALLENGE_API UMyHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	void BindToAttributes(UAttributeComponent* InAttributes);
	void ShowRestartPrompt(bool bShow);

private:
	void BuildWidgetTreeIfNeeded();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHP, float MaxHP);

	UFUNCTION()
	void HandleManaChanged(float CurrentMP, float MaxMP);

	UFUNCTION()
	void HandleEXPChanged(int32 CurrentEXP, int32 EXPToNextLevel);

	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);

	UPROPERTY()
	TObjectPtr<UAttributeComponent> Attributes;

	UPROPERTY()
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> MPBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> EXPBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY()
	TObjectPtr<UTextBlock> MPText;

	UPROPERTY()
	TObjectPtr<UTextBlock> EXPText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TObjectPtr<UTextBlock> RestartText;
};
