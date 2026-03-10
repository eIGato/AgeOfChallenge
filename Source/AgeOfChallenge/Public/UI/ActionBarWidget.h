#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "ActionBarWidget.generated.h"

// Hotbar widget drawn entirely in NativePaint.
// 4 slots anchored to the bottom-left of the screen:
//   Index 0 = key 1 (Heal), 1 = key 2 (Fireball), 2 = key 3 (Melee), 3 = key 4 (Ranged).
// Slots 2 and 3 show a bold border when active (combat mode).
// Cooldown is shown as a clockwise gray pie overlay that shrinks as the cooldown expires.
UCLASS()
class AGEOFCHALLENGE_API UActionBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// SlotIndex 0-based.  Remaining==0 clears the overlay.
	void SetCooldown(int32 SlotIndex, float Remaining, float Total);

	// SlotIndex 2 (Melee) or 3 (Ranged).  Pass -1 to clear.
	void SetActiveSlot(int32 SlotIndex);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual int32 NativePaint(
	    const FPaintArgs& Args,
	    const FGeometry& AllottedGeometry,
	    const FSlateRect& MyCullingRect,
	    FSlateWindowElementList& OutDrawElements,
	    int32 LayerId,
	    const FWidgetStyle& InWidgetStyle,
	    bool bParentEnabled) const override;

private:
	float CooldownRemaining[4] = {};
	float CooldownTotal[4] = {};
	int32 ActiveSlot = 2; // default Melee

	static constexpr float SlotSize = 64.0f;
	static constexpr float SlotGap = 8.0f;
	static constexpr float MarginX = 24.0f;
	static constexpr float MarginY = 24.0f;

	// Slot icon labels and key labels (compile-time, no allocation)
	static const TCHAR* SlotKeyLabel(int32 i);
	static const TCHAR* SlotIconLabel(int32 i);
	static FLinearColor SlotIconColor(int32 i);

	void DrawPieCooldown(
	    FSlateWindowElementList& OutDrawElements,
	    int32 LayerId,
	    const FGeometry& AllottedGeometry,
	    FVector2D Center,
	    float Radius,
	    float Fraction) const;
};
