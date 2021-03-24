// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace CPMenuExtensions
{
	TSharedPtr<FExtender> CreateMenuExtender();

	void AddMenuExtension(FMenuBuilder& MenuBuilder);
}