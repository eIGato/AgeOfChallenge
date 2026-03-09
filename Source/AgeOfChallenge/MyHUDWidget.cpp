#include "MyHUDWidget.h"

#include "AttributeComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

TSharedRef<SWidget> UMyHUDWidget::RebuildWidget()
{
	BuildWidgetTreeIfNeeded();
	return Super::RebuildWidget();
}

void UMyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	ShowRestartPrompt(false);
	UE_LOG(LogTemp, Log, TEXT("UMyHUDWidget: NativeConstruct completed"));
}

void UMyHUDWidget::BindToAttributes(UAttributeComponent* InAttributes)
{
	if (!InAttributes)
	{
		return;
	}

	Attributes = InAttributes;
	Attributes->OnHealthChanged.AddDynamic(this, &UMyHUDWidget::HandleHealthChanged);
	Attributes->OnManaChanged.AddDynamic(this, &UMyHUDWidget::HandleManaChanged);
	Attributes->OnEXPChanged.AddDynamic(this, &UMyHUDWidget::HandleEXPChanged);
	Attributes->OnLevelUp.AddDynamic(this, &UMyHUDWidget::HandleLevelUp);

	HandleHealthChanged(Attributes->CurrentHP, Attributes->MaxHP);
	HandleManaChanged(Attributes->CurrentMP, Attributes->MaxMP);
	HandleEXPChanged(Attributes->CurrentEXP, Attributes->EXPToNextLevel);
	HandleLevelUp(Attributes->Level);
}

void UMyHUDWidget::ShowRestartPrompt(bool bShow)
{
	if (RestartText)
	{
		RestartText->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UMyHUDWidget::BuildWidgetTreeIfNeeded()
{
	if (HPBar && MPBar && EXPBar && HPText && MPText && EXPText && LevelText && RestartText && WidgetTree->RootWidget)
	{
		return;
	}

	WidgetTree->RootWidget = nullptr;

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	UVerticalBox* TopLeftPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TopLeftPanel"));
	if (UCanvasPanelSlot* TopLeftSlot = Root->AddChildToCanvas(TopLeftPanel))
	{
		TopLeftSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		TopLeftSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		TopLeftSlot->SetPosition(FVector2D(24.0f, 24.0f));
		TopLeftSlot->SetAutoSize(true);
	}

	USizeBox* HPSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HPSizeBox"));
	HPSizeBox->SetWidthOverride(260.0f);
	HPSizeBox->SetHeightOverride(18.0f);
	if (UVerticalBoxSlot* BoxSlot = TopLeftPanel->AddChildToVerticalBox(HPSizeBox))
	{
		BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	}
	HPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HPBar"));
	HPBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.12f, 0.12f, 1.0f));
	HPBar->SetPercent(1.0f);
	HPSizeBox->AddChild(HPBar);

	HPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HPText"));
	HPText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	if (UVerticalBoxSlot* BoxSlot = TopLeftPanel->AddChildToVerticalBox(HPText))
	{
		BoxSlot->SetPadding(FMargin(2.0f, 0.0f, 0.0f, 8.0f));
	}

	USizeBox* MPSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MPSizeBox"));
	MPSizeBox->SetWidthOverride(260.0f);
	MPSizeBox->SetHeightOverride(18.0f);
	if (UVerticalBoxSlot* BoxSlot = TopLeftPanel->AddChildToVerticalBox(MPSizeBox))
	{
		BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	}
	MPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("MPBar"));
	MPBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.3f, 0.9f, 1.0f));
	MPBar->SetPercent(1.0f);
	MPSizeBox->AddChild(MPBar);

	MPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MPText"));
	MPText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	if (UVerticalBoxSlot* BoxSlot = TopLeftPanel->AddChildToVerticalBox(MPText))
	{
		BoxSlot->SetPadding(FMargin(2.0f, 0.0f, 0.0f, 10.0f));
	}

	UHorizontalBox* EXPRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("EXPRow"));
	if (UVerticalBoxSlot* BoxSlot = TopLeftPanel->AddChildToVerticalBox(EXPRow))
	{
		BoxSlot->SetPadding(FMargin(0.0f));
	}

	USizeBox* LevelBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LevelBox"));
	LevelBox->SetWidthOverride(34.0f);
	LevelBox->SetHeightOverride(34.0f);
	if (UHorizontalBoxSlot* BoxSlot = EXPRow->AddChildToHorizontalBox(LevelBox))
	{
		BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		BoxSlot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* LevelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LevelBorder"));
	FSlateBrush RoundedBrush;
	RoundedBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	RoundedBrush.TintColor = FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.9f));
	RoundedBrush.OutlineSettings.CornerRadii = FVector4(64.0f, 64.0f, 64.0f, 64.0f);
	LevelBorder->SetBrush(RoundedBrush);
	LevelBox->AddChild(LevelBorder);

	LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
	LevelText->SetJustification(ETextJustify::Center);
	LevelText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	LevelBorder->SetContent(LevelText);

	UVerticalBox* EXPPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EXPPanel"));
	if (UHorizontalBoxSlot* BoxSlot = EXPRow->AddChildToHorizontalBox(EXPPanel))
	{
		BoxSlot->SetVerticalAlignment(VAlign_Center);
	}

	USizeBox* EXPSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("EXPSizeBox"));
	EXPSizeBox->SetWidthOverride(218.0f);
	EXPSizeBox->SetHeightOverride(18.0f);
	if (UVerticalBoxSlot* BoxSlot = EXPPanel->AddChildToVerticalBox(EXPSizeBox))
	{
		BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	}
	EXPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("EXPBar"));
	EXPBar->SetFillColorAndOpacity(FLinearColor(0.95f, 0.75f, 0.15f, 1.0f));
	EXPBar->SetPercent(0.0f);
	EXPSizeBox->AddChild(EXPBar);

	EXPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EXPText"));
	EXPText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	EXPPanel->AddChildToVerticalBox(EXPText);

	RestartText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartText"));
	RestartText->SetText(FText::FromString(TEXT("Press Space to Restart")));
	RestartText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	RestartText->SetJustification(ETextJustify::Center);
	RestartText->SetVisibility(ESlateVisibility::Hidden);
	if (UCanvasPanelSlot* RestartSlot = Root->AddChildToCanvas(RestartText))
	{
		RestartSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		RestartSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		RestartSlot->SetPosition(FVector2D(0.0f, 0.0f));
		RestartSlot->SetAutoSize(true);
	}
}

void UMyHUDWidget::HandleHealthChanged(float CurrentHP, float MaxHP)
{
	if (HPBar)
	{
		HPBar->SetPercent((MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f);
	}

	if (HPText)
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("HP: %.0f/%.0f"), CurrentHP, MaxHP)));
	}
}

void UMyHUDWidget::HandleManaChanged(float CurrentMP, float MaxMP)
{
	if (MPBar)
	{
		MPBar->SetPercent((MaxMP > 0.0f) ? (CurrentMP / MaxMP) : 0.0f);
	}

	if (MPText)
	{
		MPText->SetText(FText::FromString(FString::Printf(TEXT("MP: %.0f/%.0f"), CurrentMP, MaxMP)));
	}
}

void UMyHUDWidget::HandleEXPChanged(int32 CurrentEXP, int32 EXPToNextLevel)
{
	if (EXPBar)
	{
		EXPBar->SetPercent((EXPToNextLevel > 0) ? (static_cast<float>(CurrentEXP) / static_cast<float>(EXPToNextLevel)) : 0.0f);
	}

	if (EXPText)
	{
		EXPText->SetText(FText::FromString(FString::Printf(TEXT("EXP: %d/%d"), CurrentEXP, EXPToNextLevel)));
	}
}

void UMyHUDWidget::HandleLevelUp(int32 NewLevel)
{
	if (LevelText)
	{
		LevelText->SetText(FText::AsNumber(NewLevel));
	}
}
