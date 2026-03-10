#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "DamageOverlayWidget.generated.h"

UCLASS()
class AGEOFCHALLENGE_API UDamageOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(
	    const FPaintArgs& Args,
	    const FGeometry& AllottedGeometry,
	    const FSlateRect& MyCullingRect,
	    FSlateWindowElementList& OutDrawElements,
	    int32 LayerId,
	    const FWidgetStyle& InWidgetStyle,
	    bool bParentEnabled) const override;

	void TriggerDamageFlash(float DamageIntensity);
	void SetCoverageFromHealthRatio(float HealthRatio);
	void SetDeathOverlay();

private:
	float CurrentIntensity = 0.0f;
	float TargetIntensity = 0.0f;
	float FadeSpeed = 2.0f;
	float Coverage = 0.0f;
	bool bDeathLocked = false;
};
