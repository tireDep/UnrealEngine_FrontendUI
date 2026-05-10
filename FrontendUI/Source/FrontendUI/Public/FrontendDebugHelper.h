#pragma once

namespace Debug
{
	static void Print( const FString& Msg, int32 Inkey = -1, const FColor& InColor = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Inkey, 7.0f,InColor,Msg);

			UE_LOG(LogTemp,Warning,TEXT("%s"),*Msg);
		}
	}
}
