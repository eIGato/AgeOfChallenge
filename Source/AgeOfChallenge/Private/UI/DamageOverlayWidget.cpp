#include "UI/DamageOverlayWidget.h"

#include "Layout/Geometry.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UDamageOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDamageOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bDeathLocked)
	{
		CurrentIntensity = 1.0f;
		TargetIntensity = 1.0f;
		return;
	}

	CurrentIntensity = FMath::FInterpTo(CurrentIntensity, TargetIntensity, InDeltaTime, FadeSpeed);
}

int32 UDamageOverlayWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
	const int32 ResultLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	const float EffectiveCoverage = bDeathLocked ? 1.0f : Coverage;
	const float EffectiveIntensity = bDeathLocked ? 1.0f : CurrentIntensity;
	if (EffectiveCoverage <= KINDA_SMALL_NUMBER || EffectiveIntensity <= KINDA_SMALL_NUMBER)
	{
		return ResultLayer;
	}

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	if (Size.X <= 1.0f || Size.Y <= 1.0f)
	{
		return ResultLayer;
	}

	const float MaxThickness = FMath::Min(Size.X, Size.Y) * 0.45f * EffectiveCoverage;
	if (MaxThickness <= 0.0f)
	{
		return ResultLayer;
	}

	const int32 BandCount = 10;
	const FLinearColor RedColor(1.0f, 0.0f, 0.0f, 1.0f);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	int32 DrawLayer = ResultLayer;

	for (int32 BandIndex = 0; BandIndex < BandCount; ++BandIndex)
	{
		const float T0 = static_cast<float>(BandIndex) / static_cast<float>(BandCount);
		const float T1 = static_cast<float>(BandIndex + 1) / static_cast<float>(BandCount);
		const float InnerInset = MaxThickness * T0;
		const float OuterInset = MaxThickness * T1;
		const float BandThickness = OuterInset - InnerInset;
		const float Alpha = EffectiveIntensity * FMath::Pow(1.0f - T0, 1.2f) * 0.75f;
		const FLinearColor BandColor = RedColor.CopyWithNewOpacity(Alpha);

		const float HorizontalWidth = FMath::Max(0.0f, Size.X - 2.0f * InnerInset);
		const float VerticalHeight = FMath::Max(0.0f, Size.Y - 2.0f * OuterInset);
		if (HorizontalWidth > 0.0f && BandThickness > 0.0f)
		{
			FSlateDrawElement::MakeBox(
			    OutDrawElements,
			    ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(
			        FVector2f(HorizontalWidth, BandThickness),
			        FSlateLayoutTransform(FVector2f(InnerInset, InnerInset))),
			    WhiteBrush,
			    ESlateDrawEffect::None,
			    BandColor);

			FSlateDrawElement::MakeBox(
			    OutDrawElements,
			    ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(
			        FVector2f(HorizontalWidth, BandThickness),
			        FSlateLayoutTransform(FVector2f(InnerInset, Size.Y - OuterInset))),
			    WhiteBrush,
			    ESlateDrawEffect::None,
			    BandColor);
		}

		if (VerticalHeight > 0.0f && BandThickness > 0.0f)
		{
			FSlateDrawElement::MakeBox(
			    OutDrawElements,
			    ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(
			        FVector2f(BandThickness, VerticalHeight),
			        FSlateLayoutTransform(FVector2f(InnerInset, OuterInset))),
			    WhiteBrush,
			    ESlateDrawEffect::None,
			    BandColor);

			FSlateDrawElement::MakeBox(
			    OutDrawElements,
			    ++DrawLayer,
			    AllottedGeometry.ToPaintGeometry(
			        FVector2f(BandThickness, VerticalHeight),
			        FSlateLayoutTransform(FVector2f(Size.X - OuterInset, OuterInset))),
			    WhiteBrush,
			    ESlateDrawEffect::None,
			    BandColor);
		}
	}

	return DrawLayer;
}

void UDamageOverlayWidget::TriggerDamageFlash(float DamageIntensity)
{
	if (bDeathLocked)
	{
		return;
	}

	const float ClampedIntensity = FMath::Clamp(DamageIntensity, 0.0f, 1.0f);
	CurrentIntensity = FMath::Max(CurrentIntensity, ClampedIntensity);
	TargetIntensity = 0.0f;
}

void UDamageOverlayWidget::SetCoverageFromHealthRatio(float HealthRatio)
{
	if (bDeathLocked)
	{
		return;
	}

	const float SafeRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);
	Coverage = 1.0f - SafeRatio;
}

void UDamageOverlayWidget::SetDeathOverlay()
{
	bDeathLocked = true;
	Coverage = 1.0f;
	CurrentIntensity = 1.0f;
	TargetIntensity = 1.0f;
}
