#include "MyFPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "UObject/ConstructorHelpers.h"

AMyFPSCharacter::AMyFPSCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;
	bUseControllerRotationYaw   = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;
	{
		static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(
			TEXT("/Game/Input/IMC_Default"));
		if (IMC.Succeeded()) DefaultMappingContext = IMC.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(
			TEXT("/Game/Input/IA_Move"));
		if (IA.Succeeded()) MoveAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(
			TEXT("/Game/Input/IA_Look"));
		if (IA.Succeeded()) LookAction = IA.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA(
			TEXT("/Game/Input/IA_Jump"));
		if (IA.Succeeded()) JumpAction = IA.Object;
	}
}

void AMyFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter::BeginPlay"));

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Log, TEXT("AMyFPSCharacter: DefaultMappingContext registered"));
		}
	}
}

void AMyFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyFPSCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyFPSCharacter::Look);
		EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}

void AMyFPSCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), Axis.Y);
		AddMovementInput(GetActorRightVector(),   Axis.X);
	}
}

void AMyFPSCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(Axis.X);
		AddControllerPitchInput(Axis.Y);
	}
}
