#include "Test/DamagingVolume.h"
#include "Player/AttributeComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADamagingVolume::ADamagingVolume()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	SetRootComponent(DamageBox);
	DamageBox->SetBoxExtent(FVector(200.0f, 200.0f, 120.0f));
	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageBox->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	DamageBox->SetGenerateOverlapEvents(true);

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(DamageBox);
	Billboard->SetRelativeScale3D(FVector(1.4f));

	bReplicates = false;
}

void ADamagingVolume::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
	    DamageTimerHandle,
	    this,
	    &ADamagingVolume::ApplyPulseDamage,
	    FMath::Max(0.05f, DamageInterval),
	    true);
}

void ADamagingVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ADamagingVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bDrawDebugVolume)
	{
		return;
	}

	DrawDebugBox(
	    GetWorld(),
	    DamageBox->GetComponentLocation(),
	    DamageBox->GetScaledBoxExtent(),
	    DamageBox->GetComponentQuat(),
	    DebugColor,
	    false,
	    0.0f,
	    0,
	    2.0f);
}

void ADamagingVolume::ApplyPulseDamage()
{
	if (!DamageBox || DamagePerPulse <= 0.0f)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	DamageBox->GetOverlappingActors(OverlappingActors, AffectedActorClass);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!IsValid(OverlappingActor) || OverlappingActor == this)
		{
			continue;
		}

		UAttributeComponent* Attributes = OverlappingActor->FindComponentByClass<UAttributeComponent>();
		if (!Attributes || !Attributes->bIsAlive)
		{
			continue;
		}

		UGameplayStatics::ApplyDamage(
		    OverlappingActor,
		    DamagePerPulse,
		    nullptr,
		    this,
		    UDamageType::StaticClass());
	}
}
