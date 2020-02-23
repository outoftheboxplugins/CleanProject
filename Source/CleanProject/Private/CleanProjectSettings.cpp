// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectSettings.h"

UCleanProjectSettings::UCleanProjectSettings()
{
    PlatformsPaths = 
    { 
        "WindowsNoEditor",
        "Android",
        "IOS",
        "Mac",
        "Linux"
    };

    BlacklistFiles =
    { 
        "PakBlacklist-Debug.txt",
        "PakBlacklist-Development.txt",
        "PakBlacklist-Test.txt",
        "PakBlacklist-Shipping.txt",
    };
}

