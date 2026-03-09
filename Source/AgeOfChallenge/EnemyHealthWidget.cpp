#include "EnemyHealthWidget.h"

#include "AttributeComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EnemyBase.h"
#include "Styling/SlateBrush.h"

TSharedRef<SWidget> UEnemyHealthWidget::RebuildWidget()
{
	BuildWidgetTreeIfNeeded();
	return Super::RebuildWidget();
}

void UEnemyHealthWidget::BuildWidgetTreeIfNeeded()
{
	if (WidgetTree->RootWidget)
	{
		return;
	}

	// Root: dark semi-transparent background makes white text readable in any world lighting
	UBorder* BgBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BgBorder"));
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));
		Brush.OutlineSettings.CornerRadii = FVector4(4.0f, 4.0f, 4.0f, 4.0f);
		BgBorder->SetBrush(Brush);
		BgBorder->SetPadding(FMargin(5.0f, 3.0f));
	}
	WidgetTree->RootWidget = BgBorder;

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	BgBorder->SetContent(ContentBox);

	// Enemy name (top, centered)
	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
	NameText->SetText(FText::GetEmpty());
	NameText->SetJustification(ETextJustify::Center);
	NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	{
		FSlateFontInfo Font = NameText->GetFont();
		Font.Size = 10;
		NameText->SetFont(Font);
	}
	if (UVerticalBoxSlot* NameSlot = ContentBox->AddChildToVerticalBox(NameText))
	{
		NameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 3.0f));
		NameSlot->SetHorizontalAlignment(HAlign_Center);
	}

	// Bottom row: [Lv.X pill] [HP bar]
	UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
	if (UVerticalBoxSlot* BottomSlot = ContentBox->AddChildToVerticalBox(BottomRow))
	{
		BottomSlot->SetHorizontalAlignment(HAlign_Center);
	}

	// Level badge
	UBorder* LevelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LevelBorder"));
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(FLinearColor(0.15f, 0.15f, 0.4f, 1.0f));
		Brush.OutlineSettings.CornerRadii = FVector4(3.0f, 3.0f, 3.0f, 3.0f);
		LevelBorder->SetBrush(Brush);
		LevelBorder->SetPadding(FMargin(4.0f, 1.0f));
	}
	if (UHorizontalBoxSlot* LevelSlot = BottomRow->AddChildToHorizontalBox(LevelBorder))
	{
		LevelSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
		LevelSlot->SetVerticalAlignment(VAlign_Center);
	}

	LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
	LevelText->SetText(FText::FromString(TEXT("Lv.1")));
	LevelText->SetJustification(ETextJustify::Center);
	LevelText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	{
		FSlateFontInfo Font = LevelText->GetFont();
		Font.Size = 9;
		LevelText->SetFont(Font);
	}
	LevelBorder->SetContent(LevelText);

	// HP progress bar
	USizeBox* BarBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BarBox"));
	BarBox->SetWidthOverride(140.0f);
	BarBox->SetHeightOverride(10.0f);
	if (UHorizontalBoxSlot* BarSlot = BottomRow->AddChildToHorizontalBox(BarBox))
	{
		BarSlot->SetVerticalAlignment(VAlign_Center);
	}

	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
	HealthBar->SetPercent(1.0f);
	HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
	BarBox->AddChild(HealthBar);
}

void UEnemyHealthWidget::InitializeForEnemy(AEnemyBase* Enemy)
{
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEnemyHealthWidget::InitializeForEnemy: Enemy is null"));
		return;
	}

	BuildWidgetTreeIfNeeded();

	UE_LOG(LogTemp, Log, TEXT("UEnemyHealthWidget::InitializeForEnemy called for '%s' (NameText valid: %s)"),
	    *Enemy->EnemyName, NameText ? TEXT("yes") : TEXT("NO"));

	if (NameText)
	{
		NameText->SetText(FText::FromString(Enemy->EnemyName));
	}

	CachedAttributeComponent = Enemy->GetAttributeComponent();
	if (!CachedAttributeComponent)
	{
		return;
	}

	CachedAttributeComponent->OnHealthChanged.AddDynamic(this, &UEnemyHealthWidget::OnHealthChanged);
	OnHealthChanged(CachedAttributeComponent->CurrentHP, CachedAttributeComponent->MaxHP);

	if (LevelText)
	{
		LevelText->SetText(FText::Format(
		    FText::FromString(TEXT("Lv.{0}")),
		    FText::AsNumber(CachedAttributeComponent->Level)));
	}
}

void UEnemyHealthWidget::OnHealthChanged(float CurrentHP, float MaxHP)
{
	if (!HealthBar)
	{
		return;
	}

	const float Ratio = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
	HealthBar->SetPercent(Ratio);
}
