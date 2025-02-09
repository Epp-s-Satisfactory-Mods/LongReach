#pragma once

#include "CoreMinimal.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "LongReachConfigInCM.h"
#include "LongReachConfigurationStruct.h"

class LONGREACH_API LongReachDebugging
{
public:
    static void DumpConfigMap(FString prefix, TMap<AFGPlayerController*,FLongReachConfigInCM>& configByPlayer);
    static void DumpPlayer(FString prefix, const AFGCharacterPlayer* player);
    static void DumpPlayerController(FString prefix, const AFGPlayerController* player);
    static void DumpConfigInCM(FString prefix, FLongReachConfigInCM& config);
    static void DumpConfigStruct(FString prefix, FLongReachConfigurationStruct& config);

    static void EnsureColon(FString& prefix)
    {
        prefix = prefix.TrimEnd();

        if (!prefix.EndsWith(TEXT(":")))
        {
            prefix.Append(TEXT(":"));
        }
    }

    static FString GetNestedPrefix(FString& prefix)
    {
        return FString(prefix).Append(TEXT("    "));
    }
};
