#include "MyFPSGameMode.h"
#include "MyFPSCharacter.h"

AMyFPSGameMode::AMyFPSGameMode()
{
	DefaultPawnClass = AMyFPSCharacter::StaticClass();
	UE_LOG(LogTemp, Log, TEXT("AMyFPSGameMode: DefaultPawnClass = AMyFPSCharacter"));
}
