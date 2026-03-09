#include "MyFPSCharacter.h"
#include "AttributeComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DamageOverlayWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
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

	{
		static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(
		    TEXT("/Game/Input/IMC_Default"));
		if (IMC.Succeeded())
			DefaultMappingContext = IMC.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(
		    TEXT("/Game/Input/IA_Move"));
		if (IA.Succeeded())
			MoveAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(
		    TEXT("/Game/Input/IA_Look"));
		if (IA.Succeeded())
			LookAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(
		    TEXT("/Game/Input/IA_Jump"));
		if (IA.Succeeded())
			JumpAction = IA.Object;
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
	}

	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AMyFPSCharacter::HandleRestartRequested);
}

float AMyFPSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!AttributeComponent)
	{
		return 0.0f;
	}

	const float AppliedDamage = AttributeComponent->TakeDamage(DamageAmount);
	if (AppliedDamage > 0.0f && DamageOverlayWidget)
	{
		const float Intensity = FMath::Clamp(
		    AppliedDamage / FMath::Max(AttributeComponent->MaxHP * 0.35f, 1.0f),
		    0.15f,
		    1.0f);
		DamageOverlayWidget->TriggerDamageFlash(Intensity);
	}

	return AppliedDamage;
}

void AMyFPSCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDying)
	{
		return;
	}

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
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(Axis.X);
		AddControllerPitchInput(Axis.Y);
	}
}

void AMyFPSCharacter::HandleDeath()
{
	if (bIsDying)
	{
		return;
	}

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
	{
		DamageOverlayWidget->SetDeathOverlay();
	}

	UE_LOG(LogTemp, Warning, TEXT("AMyFPSCharacter: Player died"));
	GetWorldTimerManager().SetTimer(RestartPromptTimerHandle, this, &AMyFPSCharacter::ShowRestartPrompt, 3.0f, false);
}

void AMyFPSCharacter::HandleHealthChanged(float CurrentHP, float MaxHP)
{
	if (!DamageOverlayWidget)
	{
		return;
	}

	const float HealthRatio = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
	DamageOverlayWidget->SetCoverageFromHealthRatio(HealthRatio);
}

void AMyFPSCharacter::ShowRestartPrompt()
{
	bCanRestart = true;
	if (HUDWidget)
	{
		HUDWidget->ShowRestartPrompt(true);
	}
}

void AMyFPSCharacter::HandleRestartRequested()
{
	if (!bCanRestart)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Restart requested"));
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

void AMyFPSCharacter::TryInitializeLocalPlayerUIAndInput()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

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
			{
				HUDWidget->BindToAttributes(AttributeComponent);
			}
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
			{
				DamageOverlayWidget->SetCoverageFromHealthRatio(AttributeComponent->GetHealthRatio());
			}
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: Damage overlay widget created"));
		}
	}
}
