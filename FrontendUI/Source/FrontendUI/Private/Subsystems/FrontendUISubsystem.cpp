
#include "Subsystems/FrontendUISubsystem.h"
#include "engine/AssetManager.h"
#include "Widgets/Widget_PrimaryLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/Widget_ActivatableBase.h"

#include "FrontendDebugHelper.h"

UFrontendUISubsystem* UFrontendUISubsystem::Get(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::Assert);
		return UGameInstance::GetSubsystem<UFrontendUISubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

bool UFrontendUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> FoundClasses;
		GetDerivedClasses(GetClass(), FoundClasses);

		return FoundClasses.IsEmpty();
	}
	
	return false;
}

void UFrontendUISubsystem::RegisterCreatedPrimaryLayoutWidget(UWidget_PrimaryLayout* InCreatedWidget)
{
	check(InCreatedWidget);
	CreatedPrimaryLayout = InCreatedWidget;

	Debug::Print(TEXT("Primary layout widget stored"));
}

/**
 * 소프트 레퍼런스 위젯을 비동기적으로 로드하고 지정된 스택에 추가하는 함수
 *
 * 소프트 레퍼런스 :
 * 필요할때만 로드, 경로만 저장 해서 메모리 사용량 낮음, 로딩 필요, 실제 사용될 때만 포함
 * 선택적으로 사용되는 UI, 대용량 에셋, dlc 콘텐츠 등에서 사용
 * 
 * 하드 레퍼런스 :
 * 게임 시작 시 즉시 로드, 모든 에셋 메모리에 상주해서 높음, 메모리에 있어서 빠름, 패키징 무조건 포함
 *
 * 비동기 에셋 로딩 :
 * 게임 계속 실행됨. 부드러운 로딩. 스마트한 시분할 처리 + 부분적 멀티스레딩 같은 느낌
 *
 * 소프트 레퍼런스 + 비동기 로딩의 장점 :
 * 필요한 것만 로드해서 메모리 효율성 좋음, UI를 미리 로드하지 않아 게임 시작이 빠름, 게임이 멈추지 않음, 새로운 UI 쉽게 추가 가능
 * 콜백과 비동기 처리가 필요해 코드가 복잡함, 첫 사용 시 로딩으로 약간의 지연, 언제 언로드할지 메모리 관리 필요
 * 
 * @param InWidgetStackTag 타겟 위젯 스택을 식별하는 게임플레이 태그
 * @param InSoftWidgetClass 로드할 위젯 클래스의 소프트 레퍼런스 (메모리 효율성을 위해 사용)
 * @param AsyncPushStateCallback 로딩 및 푸시 상태를 추적하기 위한 콜백 함수
 * 
 * 동작 과정:
 * 1. 소프트 레퍼런스 유효성 검사
 * 2. UAssetManager를 통해 비동기 에셋 로딩 요청
 * 3. 로딩 완료 시 람다 함수 실행:
 *    - 로드된 클래스 획득
 *    - 타겟 위젯 스택 찾기
 *    - 위젯 생성 및 스택에 추가
 *    - 각 단계별 콜백 실행
 * 
 * 장점:
 * - 메모리 효율성: 필요할 때만 위젯 로드
 * - 성능: 게임 플레이 중단 없이 비동기 로딩
 * - 유연성: 콜백을 통한 상태 추적 및 커스터마이징
 */
void UFrontendUISubsystem::PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag,
	TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass,
	TFunction<void(EAsyncPushWidgetState, UWidget_ActivatableBase*)> AsyncPushStateCallback)
{
	// 1단계: 소프트 레퍼런스 유효성 검사
	// 전달받은 소프트 클래스 포인터가 null이 아닌지 확인
	check(!InSoftWidgetClass.IsNull());

	// 2단계: 비동기 에셋 로딩 요청
	// UAssetManager의 StreamableManager를 사용하여 백그라운드에서 위젯 클래스 로드
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(), // 로드할 에셋의 경로
		FStreamableDelegate::CreateLambda(
			[InSoftWidgetClass, this, InWidgetStackTag, AsyncPushStateCallback]()
			{
				// 3단계: 로딩 완료 후 실행되는 람다 함수
				
				// 3-1. 로드된 위젯 클래스 획득
				UClass* LoadedWidgetClass = InSoftWidgetClass.Get();
		
				// 3-2. 유효성 검사: 클래스가 성공적으로 로드되었고 PrimaryLayout이 존재하는지 확인
				check(LoadedWidgetClass && CreatedPrimaryLayout);

				// 3-3. 지정된 태그로 타겟 위젯 스택 찾기
				UCommonActivatableWidgetContainerBase* FoundWidgetStack = CreatedPrimaryLayout->FindWidgetStackByTag(InWidgetStackTag);

				// 3-4. 위젯 생성 및 스택에 추가
				UWidget_ActivatableBase* CreatedWidget = FoundWidgetStack->AddWidget<UWidget_ActivatableBase>(
					LoadedWidgetClass,
					[AsyncPushStateCallback](UWidget_ActivatableBase& CreatedWidgetInstance)
					{
						// 위젯 생성 완료, 스택 푸시 전 콜백 실행
						// 이 시점에서 위젯 초기화 작업을 수행할 수 있음
						AsyncPushStateCallback(EAsyncPushWidgetState::OnCreatedBeforePush, &CreatedWidgetInstance);
					}
				);

				// 3-5. 스택 푸시 완료 후 콜백 실행
				// 위젯이 완전히 활성화된 상태
				AsyncPushStateCallback(EAsyncPushWidgetState::AfterPush, CreatedWidget);
			}
		)
	); // UAssetManager::Get().GetStreamableManager().RequestAsyncLoad() 종료
}

