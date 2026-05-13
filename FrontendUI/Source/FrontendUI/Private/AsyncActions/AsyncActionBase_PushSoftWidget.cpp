
#include "AsyncActions/AsyncActionBase_PushSoftWidget.h"

UAsyncActionBase_PushSoftWidget* UAsyncActionBase_PushSoftWidget::PushSoftWidget(const UObject* WorldContextObject,
	APlayerController* OwningPlayerController, TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass,
	UPARAM(meta = (Categories = "Frontend.WidgetStack"))FGameplayTag InWidgetStackTag, bool bFocusOnNewlyPushedWidget)
{
	checkf(InSoftWidgetClass.IsNull(),TEXT("PushSoftWidgetToStack was passed a null soft widget class"));

	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			UAsyncActionBase_PushSoftWidget* Node = NewObject<UAsyncActionBase_PushSoftWidget>();
			Node->RegisterWithGameInstance(World);

			return Node;
		}	
	}
	
	return nullptr;
}
