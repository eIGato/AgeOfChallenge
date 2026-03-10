#include "HealFlashWidget.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UHealFlashWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	FlashAlpha = FMath::FInterpTo(FlashAlpha, 0.0f, InDeltaTime, FadeSpeed);
}

int32 UHealFlashWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
	const int32 ResultLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (FlashAlpha <= KINDA_SMALL_NUMBER)
	{
		return ResultLayer;
	}

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));

	// Draw concentric green bands from screen edges inward (same pattern as damage overlay)
	const int32 BandCount = 8;
	const float MaxThickness = FMath::Min(Size.X, Size.Y) * 0.35f;
	const FLinearColor GreenColor(0.1f, 0.9f, 0.2f, 1.0f);
	int32 DrawLayer = ResultLayer;

	for (int32 i = 0; i < BandCount; ++i)
	{
		const float T0 = static_cast<float>(i) / static_cast<float>(BandCount);
		const float T1 = static_cast<float>(i + 1) / static_cast<float>(BandCount);
		const float InnerInset = MaxThickness * T0;
		const float OuterInset = MaxThickness * T1;
		const float BandThickness = OuterInset - InnerInset;
		const float Alpha = FlashAlpha * FMath::Pow(1.0f - T0, 1.5f) * 0.6f;
		const FLinearColor BandColor = GreenColor.CopyWithNewOpacity(Alpha);

		const float HWidth = FMath::Max(0.0f, Size.X - 2.0f * InnerInset);
		const float VHeight = FMath::Max(0.0f, Size.Y - 2.0f * OuterInset);

		if (HWidth > 0.0f && BandThickness > 0.0f)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(FVector2f(HWidth, BandThickness), FSlateLayoutTransform(FVector2f(InnerInset, InnerInset))),
			    WhiteBrush, ESlateDrawEffect::None, BandColor);

			FSlateDrawElement::MakeBox(OutDrawElements, ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(FVector2f(HWidth, BandThickness), FSlateLayoutTransform(FVector2f(InnerInset, Size.Y - OuterInset))),
			    WhiteBrush, ESlateDrawEffect::None, BandColor);
		}

		if (VHeight > 0.0f && BandThickness > 0.0f)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(FVector2f(BandThickness, VHeight), FSlateLayoutTransform(FVector2f(InnerInset, OuterInset))),
			    WhiteBrush, ESlateDrawEffect::None, BandColor);

			FSlateDrawElement::MakeBox(OutDrawElements, ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(FVector2f(BandThickness, VHeight), FSlateLayoutTransform(FVector2f(Size.X - OuterInset, OuterInset))),
			    WhiteBrush, ESlateDrawEffect::None, BandColor);
		}
	}

	return DrawLayer;
}

void UHealFlashWidget::TriggerHealFlash()
{
	FlashAlpha = 1.0f;
}
