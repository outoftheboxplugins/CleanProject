// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <Logging/LogMacros.h>

DECLARE_LOG_CATEGORY_EXTERN(LogCleanProject, Log, All)

#define LOG_TRACE() UE_LOG(LogCleanProject, Log, TEXT("%s"), ANSI_TO_TCHAR(__FUNCTION__))
