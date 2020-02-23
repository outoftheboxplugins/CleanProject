// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectSettings.h"

UCleanProjectSettings::UCleanProjectSettings()
{
    PlatformsPaths = { "WindowsNoEditor" };
}

TArray<FString> UCleanProjectSettings::GetAllBlackLists() const
{
    TArray<FString> lists;

    if (!DebugBlackList.IsEmpty())
    {
        lists.Add(DebugBlackList);
    }

    if (!DevelopmentBlackList.IsEmpty())
    {
        lists.Add(DevelopmentBlackList);
    }

    if (!TestBlackList.IsEmpty())
    {
        lists.Add(TestBlackList);
    }

    if (!ShippingBlackList.IsEmpty())
    {
        lists.Add(ShippingBlackList);
    }

    return lists;
}
