#include "Player/MyFPSGameMode.h"
#include "Player/MyFPSCharacter.h"

AMyFPSGameMode::AMyFPSGameMode()
{
	DefaultPawnClass = AMyFPSCharacter::StaticClass();
	UE_LOG(LogTemp, Log, TEXT("AMyFPSGameMode: DefaultPawnClass = AMyFPSCharacter"));
}
