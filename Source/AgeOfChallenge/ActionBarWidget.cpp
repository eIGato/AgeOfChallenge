#include "ActionBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

const TCHAR* UActionBarWidget::SlotKeyLabel(int32 i)
{
	switch (i)
	{
	case 0:
		return TEXT("1");
	case 1:
		return TEXT("2");
	case 2:
		return TEXT("3");
	case 3:
		return TEXT("4");
	default:
		return TEXT("?");
	}
}

const TCHAR* UActionBarWidget::SlotIconLabel(int32 i)
{
	switch (i)
	{
	case 0:
		return TEXT("+HP"); // Heal
	case 1:
		return TEXT("FB"); // Fireball
	case 2:
		return TEXT("ML"); // Melee
	case 3:
		return TEXT("RG"); // Ranged
	default:
		return TEXT("?");
	}
}

FLinearColor UActionBarWidget::SlotIconColor(int32 i)
{
	switch (i)
	{
	case 0:
		return FLinearColor(0.2f, 1.0f, 0.3f, 1.0f); // Green — Heal
	case 1:
		return FLinearColor(1.0f, 0.5f, 0.1f, 1.0f); // Orange — Fireball
	case 2:
		return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White  — Melee
	case 3:
		return FLinearColor(0.4f, 0.8f, 1.0f, 1.0f); // Cyan   — Ranged
	default:
		return FLinearColor::White;
	}
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UActionBarWidget::SetCooldown(int32 SlotIndex, float Remaining, float Total)
{
	if (SlotIndex < 0 || SlotIndex >= 4)
	{
		return;
	}
	CooldownRemaining[SlotIndex] = FMath::Max(0.0f, Remaining);
	CooldownTotal[SlotIndex] = FMath::Max(0.0f, Total);
}

void UActionBarWidget::SetActiveSlot(int32 SlotIndex)
{
	ActiveSlot = SlotIndex;
}

// ---------------------------------------------------------------------------
// Widget tree — blank canvas, all drawing is done in NativePaint
// ---------------------------------------------------------------------------

TSharedRef<SWidget> UActionBarWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------------------
// Pie cooldown helper — draws a clockwise gray sector from 12 o'clock
// ---------------------------------------------------------------------------

void UActionBarWidget::DrawPieCooldown(
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FGeometry& AllottedGeometry,
    FVector2D Center,
    float Radius,
    float Fraction) const
{
	if (Fraction <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	Fraction = FMath::Clamp(Fraction, 0.0f, 1.0f);

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*WhiteBrush);

	const FSlateRenderTransform AccumTransform = AllottedGeometry.GetAccumulatedRenderTransform();

	// Triangle fan: center + ring vertices from -π/2 clockwise by Fraction * 2π
	const int32 Segments = 32;
	const int32 NumTriangles = FMath::CeilToInt(Fraction * static_cast<float>(Segments));
	const float AngleStart = -HALF_PI;
	const float AngleRange = Fraction * TWO_PI;

	TArray<FSlateVertex> Verts;
	TArray<SlateIndex> Indices;
	Verts.Reserve(NumTriangles + 2);
	Indices.Reserve(NumTriangles * 3);

	const FColor PieColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.65f).ToFColor(true);

	// Center vertex
	{
		FSlateVertex V = {};
		V.Position = FVector2f(AccumTransform.TransformPoint(Center));
		V.Color = PieColor;
		Verts.Add(V);
	}

	for (int32 i = 0; i <= NumTriangles; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(Segments);
		const float Angle = AngleStart + FMath::Min(t * TWO_PI, AngleRange);
		const FVector2D EdgeLocal(Center.X + FMath::Cos(Angle) * Radius, Center.Y + FMath::Sin(Angle) * Radius);

		FSlateVertex V = {};
		V.Position = FVector2f(AccumTransform.TransformPoint(EdgeLocal));
		V.Color = PieColor;
		Verts.Add(V);

		if (i > 0)
		{
			Indices.Add(0);
			Indices.Add(static_cast<SlateIndex>(i));
			Indices.Add(static_cast<SlateIndex>(i + 1));
		}
	}

	FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Verts, Indices, nullptr, 0, 0);
}

// ---------------------------------------------------------------------------
// NativePaint — draw the entire bar
// ---------------------------------------------------------------------------

int32 UActionBarWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
	int32 Layer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FVector2D ScreenSize = AllottedGeometry.GetLocalSize();
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);
	const FSlateFontInfo IconFont = FCoreStyle::GetDefaultFontStyle("Bold", 13);

	// Bottom-left origin for slot row
	const float OriginX = MarginX;
	const float OriginY = ScreenSize.Y - SlotSize - MarginY;

	for (int32 i = 0; i < 4; ++i)
	{
		const float SlotX = OriginX + i * (SlotSize + SlotGap);
		const float SlotY = OriginY;

		// ---- Background ----
		const FLinearColor BgColor(0.05f, 0.05f, 0.05f, 0.75f);
		FSlateDrawElement::MakeBox(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(SlotSize, SlotSize), FSlateLayoutTransform(FVector2f(SlotX, SlotY))),
		    WhiteBrush, ESlateDrawEffect::None, BgColor);

		// ---- Border ----
		const bool bActive = (i == ActiveSlot);
		const float BorderThickness = bActive ? 3.0f : 1.5f;
		const FLinearColor BorderColor = bActive
		    ? FLinearColor(1.0f, 1.0f, 0.2f, 1.0f) // yellow when active
		    : FLinearColor(0.5f, 0.5f, 0.5f, 0.8f); // gray otherwise

		// Top edge
		FSlateDrawElement::MakeBox(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(SlotSize, BorderThickness), FSlateLayoutTransform(FVector2f(SlotX, SlotY))),
		    WhiteBrush, ESlateDrawEffect::None, BorderColor);
		// Bottom edge
		FSlateDrawElement::MakeBox(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(SlotSize, BorderThickness), FSlateLayoutTransform(FVector2f(SlotX, SlotY + SlotSize - BorderThickness))),
		    WhiteBrush, ESlateDrawEffect::None, BorderColor);
		// Left edge
		FSlateDrawElement::MakeBox(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(BorderThickness, SlotSize), FSlateLayoutTransform(FVector2f(SlotX, SlotY))),
		    WhiteBrush, ESlateDrawEffect::None, BorderColor);
		// Right edge
		FSlateDrawElement::MakeBox(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(BorderThickness, SlotSize), FSlateLayoutTransform(FVector2f(SlotX + SlotSize - BorderThickness, SlotY))),
		    WhiteBrush, ESlateDrawEffect::None, BorderColor);

		// ---- Icon label (centered) ----
		FSlateDrawElement::MakeText(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(SlotSize, SlotSize), FSlateLayoutTransform(FVector2f(SlotX, SlotY + SlotSize * 0.3f))),
		    FText::FromString(SlotIconLabel(i)),
		    IconFont,
		    ESlateDrawEffect::None,
		    SlotIconColor(i));

		// ---- Key number (top-right corner, small) ----
		FSlateDrawElement::MakeText(OutDrawElements, ++Layer,
		    AllottedGeometry.ToPaintGeometry(FVector2f(SlotSize - 4.0f, 16.0f), FSlateLayoutTransform(FVector2f(SlotX + 4.0f, SlotY + 4.0f))),
		    FText::FromString(SlotKeyLabel(i)),
		    SmallFont,
		    ESlateDrawEffect::None,
		    FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));

		// ---- Pie cooldown overlay ----
		if (CooldownTotal[i] > KINDA_SMALL_NUMBER && CooldownRemaining[i] > KINDA_SMALL_NUMBER)
		{
			const float Fraction = CooldownRemaining[i] / CooldownTotal[i];
			const FVector2D SlotCenter(SlotX + SlotSize * 0.5f, SlotY + SlotSize * 0.5f);
			DrawPieCooldown(OutDrawElements, ++Layer, AllottedGeometry, SlotCenter, SlotSize * 0.5f - 2.0f, Fraction);
		}
	}

	return Layer;
}
