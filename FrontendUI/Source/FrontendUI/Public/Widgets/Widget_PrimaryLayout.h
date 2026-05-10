
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Widget_PrimaryLayout.generated.h"

class UCommonActivatableWidgetContainerBase;

UCLASS(Abstract,BlueprintType,meta=(DisableNativeTick))
class FRONTENDUI_API UWidget_PrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonActivatableWidgetContainerBase* FindWidgetStackByTag(const FGameplayTag& InTag)const;

protected:
	UFUNCTION(BlueprintCallable)
	void RegisterWidgetStack(UPARAM(meta = (Categories = "Frontend.WidgetStack")) FGameplayTag InStackTag, UCommonActivatableWidgetContainerBase* InStack);

private:
	// Transient :
	// 휘발성. 객체가 저장되거나 로드될 때 변수 값이 디스크에 기록되지 않음.
	// 게임을 실행할 때마다 런타임 중에 동적으로 할당되고 관리되는 데이터에 주로 사용
	UPROPERTY(Transient)
	TMap<FGameplayTag,UCommonActivatableWidgetContainerBase*> RegisterWidgetStackMap;
};
