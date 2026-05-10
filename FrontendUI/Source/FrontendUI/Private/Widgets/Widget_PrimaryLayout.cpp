
#include "Widgets/Widget_PrimaryLayout.h"

UCommonActivatableWidgetContainerBase* UWidget_PrimaryLayout::FindWidgetStackByTag(const FGameplayTag& InTag) const
{
	checkf(RegisterWidgetStackMap.Contains(InTag), TEXT("Can not find the widget stack by the tag %s"),*InTag.ToString());

	return RegisterWidgetStackMap.FindRef(InTag);
}

void UWidget_PrimaryLayout::RegisterWidgetStack(UPARAM(meta = (Categories = "Frontend.WidgetStack"))FGameplayTag InStackTag, UCommonActivatableWidgetContainerBase* InStack)
{
	if (!IsDesignTime())
	{
		if (!RegisterWidgetStackMap.Contains(InStackTag))
		{
			RegisterWidgetStackMap.Add(InStackTag, InStack);
		}
	}
}
