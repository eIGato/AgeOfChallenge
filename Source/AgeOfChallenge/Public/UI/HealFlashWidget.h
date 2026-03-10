#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "HealFlashWidget.generated.h"

// Full-screen green flash on heal (analogous to DamageOverlayWidget's red flash).
UCLASS()
class AGEOFCHALLENGE_API UHealFlashWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(
	    const FPaintArgs& Args,
	    const FGeometry& AllottedGeometry,
	    const FSlateRect& MyCullingRect,
	    FSlateWindowElementList& OutDrawElements,
	    int32 LayerId,
	    const FWidgetStyle& InWidgetStyle,
	    bool bParentEnabled) const override;

	void TriggerHealFlash();

private:
	float FlashAlpha = 0.0f;
	static constexpr float FadeSpeed = 4.0f;
};
