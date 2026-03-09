#include "EnemyBase.h"

#include "AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyAIController.h"
#include "EnemyHealthWidget.h"
#include "EnemySettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));

	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(GetCapsuleComponent());
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	{
		// Load engine sphere as a capsule proxy.
		// Sphere BasicShape radius = 50 units → scale to match ACharacter default capsule
		// (HalfHeight = 96, Radius = 42): scale X/Y = 42/50, Z = 96/50
		static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));
		if (SphereFinder.Succeeded())
		{
			EnemyMesh->SetStaticMesh(SphereFinder.Object);
			EnemyMesh->SetRelativeScale3D(FVector(0.84f, 0.84f, 1.92f));
		}
	}

	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(GetCapsuleComponent());
	HealthWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthWidgetComponent->SetDrawSize(FVector2D(220.0f, 60.0f));
	HealthWidgetComponent->SetWidgetClass(UEnemyHealthWidget::StaticClass());

	// Derive display name from class name, stripping the "Enemy" suffix.
	// AWarriorEnemy → "Warrior", AMageEnemy → "Mage", etc.
	{
		FString ClassName = GetClass()->GetName();
		ClassName.RemoveFromEnd(TEXT("Enemy"));
		EnemyName = ClassName;
	}
	AttackDamage = 10.0f;
	AttackRange = 150.0f;
	AttackCooldown = 1.5f;
	EXPRewardPerLevel = 10.0f;
	DeathTiltDuration = 0.5f;
	DeathDestroyDelay = 3.0f;

	bIsDying = false;
	DeathAnimElapsed = 0.0f;
	DeathTiltDirection = 1.0f;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay(); // Components (including WidgetComponent) initialize here

	// Override defaults from centralized settings
	if (const UEnemySettings* Settings = GetDefault<UEnemySettings>())
	{
		EXPRewardPerLevel = Settings->EXPRewardPerLevel;
		DeathTiltDuration = Settings->DeathTiltDuration;
		DeathDestroyDelay = Settings->DeathDestroyDelay;
	}

	// Apply dynamic material color.
	// Force BasicShapeMaterial (guaranteed to have a "Color" vector param) before creating the DynMat,
	// so we don't depend on whatever material the Blueprint assigned.
	if (EnemyMesh)
	{
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(
		    nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (BaseMat)
		{
			EnemyMesh->SetMaterial(0, BaseMat);
		}

		if (EnemyMesh->GetNumMaterials() > 0)
		{
			UMaterialInstanceDynamic* DynMat = EnemyMesh->CreateAndSetMaterialInstanceDynamic(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("Color"), GetEnemyColor());
			}
		}
	}

	// Bind death event
	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);
	}

	// Initialize health widget with reference to this enemy
	if (HealthWidgetComponent)
	{
		if (UEnemyHealthWidget* HealthWidget = Cast<UEnemyHealthWidget>(HealthWidgetComponent->GetWidget()))
		{
			HealthWidget->InitializeForEnemy(this);
		}
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDying)
	{
		return;
	}

	DeathAnimElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(DeathAnimElapsed / DeathTiltDuration, 0.0f, 1.0f);
	// Ease-in (accelerating): squared alpha
	const float EasedAlpha = Alpha * Alpha;
	const float TiltAngle = DeathTiltDirection * 90.0f * EasedAlpha;

	FRotator NewRotation = DeathStartRotation;
	NewRotation.Roll += TiltAngle;
	SetActorRotation(NewRotation);
}

void AEnemyBase::SetLevel(int32 NewLevel)
{
	ApplyLevelScaling(NewLevel);
}

void AEnemyBase::ApplyLevelScaling(int32 NewLevel)
{
	if (!AttributeComponent)
	{
		return;
	}

	const UEnemySettings* Settings = GetDefault<UEnemySettings>();
	if (!Settings)
	{
		return;
	}

	const int32 LevelsAboveBase = FMath::Max(0, NewLevel - 1);
	AttributeComponent->Level = NewLevel;
	AttributeComponent->MaxHP = Settings->BaseMaxHP + Settings->HPPerLevel * LevelsAboveBase;
	AttributeComponent->CurrentHP = AttributeComponent->MaxHP;
	AttackDamage += Settings->DamagePerLevel * LevelsAboveBase;

	UE_LOG(LogTemp, Log, TEXT("AEnemyBase: %s set to level %d — MaxHP=%.1f, Damage=%.1f"),
	    *EnemyName, NewLevel, AttributeComponent->MaxHP, AttackDamage);
}

FLinearColor AEnemyBase::GetEnemyColor() const
{
	return FLinearColor::Gray;
}

void AEnemyBase::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("AEnemyBase: %s died"), *EnemyName);

	// Grant EXP to player (always, regardless of who killed it)
	if (ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		if (UAttributeComponent* PlayerAttr = PlayerChar->FindComponentByClass<UAttributeComponent>())
		{
			const float EXP = AttributeComponent ? AttributeComponent->Level * EXPRewardPerLevel : EXPRewardPerLevel;
			PlayerAttr->AddEXP(EXP);
			UE_LOG(LogTemp, Log, TEXT("AEnemyBase: Awarded %.1f EXP to player for killing %s"), EXP, *EnemyName);
		}
	}

	// Disable capsule collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Unpossess AI controller
	if (AController* Ctrl = GetController())
	{
		Ctrl->UnPossess();
	}

	// Stop movement
	GetCharacterMovement()->DisableMovement();

	// Start death tilt animation
	bIsDying = true;
	DeathAnimElapsed = 0.0f;
	DeathStartRotation = GetActorRotation();
	DeathTiltDirection = FMath::RandBool() ? 1.0f : -1.0f;

	// Schedule destruction
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AEnemyBase::DestroyEnemy, DeathDestroyDelay, false);
}

void AEnemyBase::DestroyEnemy()
{
	Destroy();
}
