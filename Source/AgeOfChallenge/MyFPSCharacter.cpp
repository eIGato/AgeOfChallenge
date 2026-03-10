#include "MyFPSCharacter.h"

#include "ActionBarWidget.h"
#include "AttributeComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DamageOverlayWidget.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FireballProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HealFlashWidget.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KnifeProjectile.h"
#include "MyHUDWidget.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AMyFPSCharacter::AMyFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	HUDWidgetClass = UMyHUDWidget::StaticClass();
	DamageOverlayWidgetClass = UDamageOverlayWidget::StaticClass();
	ActionBarWidgetClass = UActionBarWidget::StaticClass();
	HealFlashWidgetClass = UHealFlashWidget::StaticClass();
	KnifeProjectileClass = AKnifeProjectile::StaticClass();
	FireballProjectileClass = AFireballProjectile::StaticClass();
	{
		static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(TEXT("/Game/Input/IMC_Default"));
		if (IMC.Succeeded())
			DefaultMappingContext = IMC.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_Move"));
		if (IA.Succeeded())
			MoveAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_Look"));
		if (IA.Succeeded())
			LookAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_Jump"));
		if (IA.Succeeded())
			JumpAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_Attack"));
		if (IA.Succeeded())
			AttackAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_SwitchMelee"));
		if (IA.Succeeded())
			SwitchMeleeAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_SwitchRanged"));
		if (IA.Succeeded())
			SwitchRangedAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_Heal"));
		if (IA.Succeeded())
			HealAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/Input/IA_Fireball"));
		if (IA.Succeeded())
			FireballAction = IA.Object;
	}
}

void AMyFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter::BeginPlay"));
	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(this, &AMyFPSCharacter::HandleDeath);
		AttributeComponent->OnHealthChanged.AddDynamic(this, &AMyFPSCharacter::HandleHealthChanged);
	}

	TryInitializeLocalPlayerUIAndInput();
}

void AMyFPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if ((!HUDWidget || !DamageOverlayWidget || !bInputContextAdded) && !bIsDying)
	{
		TryInitializeLocalPlayerUIAndInput();
	}

	// Tick cooldowns and push to ActionBar
	if (ActionBarWidget)
	{
		for (int32 i = 0; i < 4; ++i)
		{
			if (SlotCooldownRemaining[i] > 0.0f)
			{
				SlotCooldownRemaining[i] = FMath::Max(0.0f, SlotCooldownRemaining[i] - DeltaSeconds);
				ActionBarWidget->SetCooldown(i, SlotCooldownRemaining[i], SlotCooldownTotal[i]);
			}
		}
	}

	if (bIsDying)
	{
		DeathAnimationElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(DeathAnimationElapsed / FMath::Max(DeathAnimationDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
		const float EasedAlpha = FMath::InterpEaseIn(0.0f, 1.0f, Alpha, 2.4f);

		FirstPersonCamera->SetRelativeLocation(FMath::Lerp(DeathCameraStartLocation, DeathCameraTargetLocation, EasedAlpha));
		const FQuat BlendedRotation = FQuat::Slerp(
		    DeathCameraStartRotation.Quaternion(),
		    DeathCameraTargetRotation.Quaternion(),
		    EasedAlpha);
		FirstPersonCamera->SetRelativeRotation(BlendedRotation);
	}
}

void AMyFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyFPSCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyFPSCharacter::Look);
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		if (AttackAction)
			EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AMyFPSCharacter::PerformAttack);
		if (SwitchMeleeAction)
			EIC->BindAction(SwitchMeleeAction, ETriggerEvent::Started, this, &AMyFPSCharacter::SwitchToMelee);
		if (SwitchRangedAction)
			EIC->BindAction(SwitchRangedAction, ETriggerEvent::Started, this, &AMyFPSCharacter::SwitchToRanged);
		if (HealAction)
			EIC->BindAction(HealAction, ETriggerEvent::Started, this, &AMyFPSCharacter::CastHeal);
		if (FireballAction)
			EIC->BindAction(FireballAction, ETriggerEvent::Started, this, &AMyFPSCharacter::CastFireball);
	}

	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AMyFPSCharacter::HandleRestartRequested);
}

float AMyFPSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!AttributeComponent)
		return 0.0f;

	const float AppliedDamage = AttributeComponent->TakeDamage(DamageAmount);
	if (AppliedDamage > 0.0f && DamageOverlayWidget)
	{
		const float Intensity = FMath::Clamp(
		    AppliedDamage / FMath::Max(AttributeComponent->MaxHP * 0.35f, 1.0f),
		    0.15f, 1.0f);
		DamageOverlayWidget->TriggerDamageFlash(Intensity);
	}
	return AppliedDamage;
}

// ---------------------------------------------------------------------------
// Combat — mode switching
// ---------------------------------------------------------------------------

void AMyFPSCharacter::SwitchToMelee(const FInputActionValue& Value)
{
	if (bIsDying)
		return;
	CombatMode = EPlayerCombatMode::Melee;
	if (ActionBarWidget)
		ActionBarWidget->SetActiveSlot(2);
	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Switched to Melee"));
}

void AMyFPSCharacter::SwitchToRanged(const FInputActionValue& Value)
{
	if (bIsDying)
		return;
	CombatMode = EPlayerCombatMode::Ranged;
	if (ActionBarWidget)
		ActionBarWidget->SetActiveSlot(3);
	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Switched to Ranged"));
}

// ---------------------------------------------------------------------------
// Combat — attack dispatch
// ---------------------------------------------------------------------------

void AMyFPSCharacter::PerformAttack(const FInputActionValue& Value)
{
	if (bIsDying)
		return;

	if (CombatMode == EPlayerCombatMode::Melee)
	{
		DoMeleeAttack();
	}
	else
	{
		DoRangedAttack();
	}
}

void AMyFPSCharacter::DoMeleeAttack()
{
	// Slot index 2 = Melee
	if (SlotCooldownRemaining[2] > 0.0f)
		return;

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * MeleeRange;

	TArray<FHitResult> Hits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::SphereTraceMulti(
	    this, Start, End, MeleeRadius,
	    UEngineTypes::ConvertToTraceType(ECC_Pawn),
	    false, ActorsToIgnore,
	    EDrawDebugTrace::ForDuration, Hits, true,
	    FLinearColor::Red, FLinearColor::Green, 0.5f);

	bool bHitAny = false;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == this)
			continue;

		if (UAttributeComponent* Attr = Target->FindComponentByClass<UAttributeComponent>())
		{
			if (Attr->bIsAlive)
			{
				Attr->TakeDamage(MeleeDamage);
				UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Melee hit %s for %.1f"), *Target->GetName(), MeleeDamage);
				bHitAny = true;
			}
		}
	}

	SlotCooldownRemaining[2] = MeleeCooldown;
	SlotCooldownTotal[2] = MeleeCooldown;
	if (ActionBarWidget)
		ActionBarWidget->SetCooldown(2, MeleeCooldown, MeleeCooldown);

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Melee attack — hit %s"), bHitAny ? TEXT("something") : TEXT("nothing"));
}

void AMyFPSCharacter::DoRangedAttack()
{
	// Slot index 3 = Ranged
	if (SlotCooldownRemaining[3] > 0.0f)
		return;
	if (!KnifeProjectileClass)
		return;

	const FVector SpawnLoc = FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() * 80.0f;
	const FRotator SpawnRot = FirstPersonCamera->GetComponentRotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	AKnifeProjectile* Knife = GetWorld()->SpawnActor<AKnifeProjectile>(KnifeProjectileClass, SpawnLoc, SpawnRot, Params);
	if (Knife)
	{
		if (UProjectileMovementComponent* PM = Knife->FindComponentByClass<UProjectileMovementComponent>())
		{
			PM->Velocity = FirstPersonCamera->GetForwardVector() * PM->InitialSpeed;
		}
	}

	SlotCooldownRemaining[3] = KnifeCooldown;
	SlotCooldownTotal[3] = KnifeCooldown;
	if (ActionBarWidget)
		ActionBarWidget->SetCooldown(3, KnifeCooldown, KnifeCooldown);

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Knife thrown"));
}

// ---------------------------------------------------------------------------
// Combat — spells
// ---------------------------------------------------------------------------

void AMyFPSCharacter::CastHeal(const FInputActionValue& Value)
{
	if (bIsDying)
		return;
	// Slot index 0 = Heal
	if (SlotCooldownRemaining[0] > 0.0f)
		return;
	if (!AttributeComponent)
		return;

	const int32 Level = AttributeComponent->Level;
	const float MPCost = HealBaseMPCost + Level * HealMPCostPerLevel;
	if (!AttributeComponent->SpendMP(MPCost))
	{
		UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Heal failed — not enough MP (need %.1f)"), MPCost);
		return;
	}

	const float HealAmount = HealBaseAmount + Level * HealAmountPerLevel;
	AttributeComponent->Heal(HealAmount);

	if (HealFlashWidget)
		HealFlashWidget->TriggerHealFlash();

	SlotCooldownRemaining[0] = HealCooldown;
	SlotCooldownTotal[0] = HealCooldown;
	if (ActionBarWidget)
		ActionBarWidget->SetCooldown(0, HealCooldown, HealCooldown);

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Heal cast — +%.1f HP, -%.1f MP"), HealAmount, MPCost);
}

void AMyFPSCharacter::CastFireball(const FInputActionValue& Value)
{
	if (bIsDying)
		return;
	// Slot index 1 = Fireball
	if (SlotCooldownRemaining[1] > 0.0f)
		return;
	if (!AttributeComponent || !FireballProjectileClass)
		return;

	const int32 Level = AttributeComponent->Level;
	const float MPCost = FireballBaseMPCost + Level * FireballMPCostPerLevel;
	if (!AttributeComponent->SpendMP(MPCost))
	{
		UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Fireball failed — not enough MP (need %.1f)"), MPCost);
		return;
	}

	const FVector CamLoc = FirstPersonCamera->GetComponentLocation();
	const FVector CamFwd = FirstPersonCamera->GetForwardVector();
	const FVector SpawnLoc = CamLoc + CamFwd * 100.0f + FVector(0.0f, 0.0f, -10.0f);
	const FRotator SpawnRot = FirstPersonCamera->GetComponentRotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	AFireballProjectile* Fireball = GetWorld()->SpawnActor<AFireballProjectile>(FireballProjectileClass, SpawnLoc, SpawnRot, Params);
	if (Fireball)
	{
		const float ScaledDamage = FireballBaseDamage + Level * FireballDamagePerLevel;
		Fireball->Damage = ScaledDamage;

		if (UProjectileMovementComponent* PM = Fireball->FindComponentByClass<UProjectileMovementComponent>())
		{
			PM->Velocity = CamFwd * PM->InitialSpeed;
		}
	}

	SlotCooldownRemaining[1] = FireballCooldown;
	SlotCooldownTotal[1] = FireballCooldown;
	if (ActionBarWidget)
		ActionBarWidget->SetCooldown(1, FireballCooldown, FireballCooldown);

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Fireball cast — %.1f dmg, -%.1f MP"), FireballBaseDamage + Level * FireballDamagePerLevel, MPCost);
}

// ---------------------------------------------------------------------------
// Movement / look
// ---------------------------------------------------------------------------

void AMyFPSCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDying)
		return;
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), Axis.Y);
		AddMovementInput(GetActorRightVector(), Axis.X);
	}
}

void AMyFPSCharacter::Look(const FInputActionValue& Value)
{
	if (bIsDying)
		return;
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(Axis.X);
		AddControllerPitchInput(Axis.Y);
	}
}

// ---------------------------------------------------------------------------
// Death / restart
// ---------------------------------------------------------------------------

void AMyFPSCharacter::HandleDeath()
{
	if (bIsDying)
		return;
	bIsDying = true;
	DeathAnimationElapsed = 0.0f;

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	FirstPersonCamera->bUsePawnControlRotation = false;

	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	DeathCameraStartLocation = FirstPersonCamera->GetRelativeLocation();
	DeathCameraStartRotation = FirstPersonCamera->GetRelativeRotation();
	DeathCameraTargetLocation = FVector(DeathCameraStartLocation.X, DeathCameraStartLocation.Y, -CapsuleHalfHeight + 20.0f);

	const float RollSign = FMath::RandBool() ? 1.0f : -1.0f;
	const float RollAmount = FMath::FRandRange(30.0f, 90.0f) * RollSign;
	const float PitchAmount = FMath::FRandRange(8.0f, 18.0f);
	DeathCameraTargetRotation = DeathCameraStartRotation + FRotator(PitchAmount, 0.0f, RollAmount);

	if (DamageOverlayWidget)
		DamageOverlayWidget->SetDeathOverlay();

	UE_LOG(LogTemp, Warning, TEXT("AMyFPSCharacter: Player died"));
	GetWorldTimerManager().SetTimer(RestartPromptTimerHandle, this, &AMyFPSCharacter::ShowRestartPrompt, 3.0f, false);
}

void AMyFPSCharacter::HandleHealthChanged(float CurrentHP, float MaxHP)
{
	if (!DamageOverlayWidget)
		return;
	const float HealthRatio = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
	DamageOverlayWidget->SetCoverageFromHealthRatio(HealthRatio);
}

void AMyFPSCharacter::ShowRestartPrompt()
{
	bCanRestart = true;
	if (HUDWidget)
		HUDWidget->ShowRestartPrompt(true);
}

void AMyFPSCharacter::HandleRestartRequested()
{
	if (!bCanRestart)
		return;
	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Restart requested"));
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

// ---------------------------------------------------------------------------
// UI + input init
// ---------------------------------------------------------------------------

void AMyFPSCharacter::TryInitializeLocalPlayerUIAndInput()
{
	if (!IsLocallyControlled())
		return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	if (!bInputContextAdded)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			bInputContextAdded = true;
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: DefaultMappingContext registered"));
		}
	}

	if (!HUDWidget && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UMyHUDWidget>(PC, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(1);
			if (AttributeComponent)
				HUDWidget->BindToAttributes(AttributeComponent);
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: HUD widget created"));
		}
	}

	if (!DamageOverlayWidget && DamageOverlayWidgetClass)
	{
		DamageOverlayWidget = CreateWidget<UDamageOverlayWidget>(PC, DamageOverlayWidgetClass);
		if (DamageOverlayWidget)
		{
			DamageOverlayWidget->AddToViewport(2);
			if (AttributeComponent)
				DamageOverlayWidget->SetCoverageFromHealthRatio(AttributeComponent->GetHealthRatio());
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Damage overlay widget created"));
		}
	}

	if (!ActionBarWidget && ActionBarWidgetClass)
	{
		ActionBarWidget = CreateWidget<UActionBarWidget>(PC, ActionBarWidgetClass);
		if (ActionBarWidget)
		{
			ActionBarWidget->AddToViewport(1);
			ActionBarWidget->SetActiveSlot(CombatMode == EPlayerCombatMode::Melee ? 2 : 3);
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: ActionBar widget created"));
		}
	}

	if (!HealFlashWidget && HealFlashWidgetClass)
	{
		HealFlashWidget = CreateWidget<UHealFlashWidget>(PC, HealFlashWidgetClass);
		if (HealFlashWidget)
		{
			HealFlashWidget->AddToViewport(3);
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: HealFlash widget created"));
		}
	}
}
