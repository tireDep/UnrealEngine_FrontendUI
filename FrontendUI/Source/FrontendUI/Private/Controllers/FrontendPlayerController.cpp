
#include "Controllers/FrontendPlayerController.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

void AFrontendPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// Searching Camera
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ACameraActor::StaticClass(), FName("Default"),FoundCameras);

	if (!FoundCameras.IsEmpty())
	{
		SetViewTarget(*FoundCameras.begin());
	}
}
