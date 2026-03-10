#include "Player/AttributeComponent.h"

#include "Misc/ConfigCacheIni.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxHP = 100.0f;
	CurrentHP = MaxHP;
	MaxMP = 60.0f;
	CurrentMP = MaxMP;
	CurrentEXP = 0;
	EXPToNextLevel = 100;
	Level = 1;
	HPRegenRate = 2.0f;
	MPRegenRate = 4.0f;
	EXPGrowthMultiplier = 1.5f;
	bIsAlive = true;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	LoadValuesFromConfig();
	BroadcastAll();
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAlive)
	{
		return;
	}

	const float PreviousHP = CurrentHP;
	const float PreviousMP = CurrentMP;

	CurrentHP = FMath::Clamp(CurrentHP + HPRegenRate * DeltaTime, 0.0f, MaxHP);
	CurrentMP = FMath::Clamp(CurrentMP + MPRegenRate * DeltaTime, 0.0f, MaxMP);

	if (!FMath::IsNearlyEqual(PreviousHP, CurrentHP))
	{
		OnHealthChanged.Broadcast(CurrentHP, MaxHP);
	}

	if (!FMath::IsNearlyEqual(PreviousMP, CurrentMP))
	{
		OnManaChanged.Broadcast(CurrentMP, MaxMP);
	}
}

float UAttributeComponent::TakeDamage(float Damage)
{
	if (!bIsAlive || Damage <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.0f, MaxHP);
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);

	const float AppliedDamage = PreviousHP - CurrentHP;
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return AppliedDamage;
}

void UAttributeComponent::Heal(float Amount)
{
	if (!bIsAlive || Amount <= 0.0f)
	{
		return;
	}

	CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

bool UAttributeComponent::SpendMP(float Amount)
{
	if (!bIsAlive || Amount <= 0.0f)
	{
		return false;
	}

	if (CurrentMP < Amount)
	{
		return false;
	}

	CurrentMP = FMath::Clamp(CurrentMP - Amount, 0.0f, MaxMP);
	OnManaChanged.Broadcast(CurrentMP, MaxMP);
	return true;
}

void UAttributeComponent::AddEXP(float Amount)
{
	if (!bIsAlive || Amount <= 0.0f)
	{
		return;
	}

	CurrentEXP += FMath::RoundToInt(Amount);

	while (CurrentEXP >= EXPToNextLevel)
	{
		CurrentEXP -= EXPToNextLevel;
		LevelUp();
	}

	OnEXPChanged.Broadcast(CurrentEXP, EXPToNextLevel);
}

void UAttributeComponent::Die()
{
	if (!bIsAlive)
	{
		return;
	}

	bIsAlive = false;
	UE_LOG(LogTemp, Warning, TEXT("UAttributeComponent: %s died"), *GetOwner()->GetName());
	OnDeath.Broadcast();
}

float UAttributeComponent::GetHealthRatio() const
{
	return (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
}

float UAttributeComponent::GetManaRatio() const
{
	return (MaxMP > 0.0f) ? (CurrentMP / MaxMP) : 0.0f;
}

float UAttributeComponent::GetEXPRatio() const
{
	return (EXPToNextLevel > 0) ? (static_cast<float>(CurrentEXP) / static_cast<float>(EXPToNextLevel)) : 0.0f;
}

void UAttributeComponent::LoadValuesFromConfig()
{
	const TCHAR* Section = TEXT("AgeOfChallenge.Attributes");

	GConfig->GetFloat(Section, TEXT("MaxHP"), MaxHP, GGameIni);
	GConfig->GetFloat(Section, TEXT("MaxMP"), MaxMP, GGameIni);
	GConfig->GetInt(Section, TEXT("Level"), Level, GGameIni);
	GConfig->GetInt(Section, TEXT("CurrentEXP"), CurrentEXP, GGameIni);
	GConfig->GetInt(Section, TEXT("EXPToNextLevel"), EXPToNextLevel, GGameIni);
	GConfig->GetFloat(Section, TEXT("HPRegenRate"), HPRegenRate, GGameIni);
	GConfig->GetFloat(Section, TEXT("MPRegenRate"), MPRegenRate, GGameIni);
	GConfig->GetFloat(Section, TEXT("EXPGrowthMultiplier"), EXPGrowthMultiplier, GGameIni);

	MaxHP = FMath::Max(1.0f, MaxHP);
	MaxMP = FMath::Max(1.0f, MaxMP);
	Level = FMath::Max(1, Level);
	EXPToNextLevel = FMath::Max(1, EXPToNextLevel);
	EXPGrowthMultiplier = FMath::Max(1.05f, EXPGrowthMultiplier);

	CurrentHP = MaxHP;
	CurrentMP = MaxMP;
	CurrentEXP = FMath::Clamp(CurrentEXP, 0, EXPToNextLevel - 1);
	bIsAlive = true;
}

void UAttributeComponent::BroadcastAll()
{
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
	OnManaChanged.Broadcast(CurrentMP, MaxMP);
	OnEXPChanged.Broadcast(CurrentEXP, EXPToNextLevel);
}

void UAttributeComponent::LevelUp()
{
	Level++;
	MaxHP += 20.0f;
	MaxMP += 10.0f;
	HPRegenRate += 0.5f;
	MPRegenRate += 0.3f;

	CurrentHP = MaxHP;
	CurrentMP = MaxMP;
	EXPToNextLevel = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EXPToNextLevel) * EXPGrowthMultiplier));

	UE_LOG(LogTemp, Log, TEXT("UAttributeComponent: Level up to %d"), Level);
	OnLevelUp.Broadcast(Level);
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
	OnManaChanged.Broadcast(CurrentMP, MaxMP);
}
