// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OVRLipSync.h"
#include "ModuleManager.h"

class ILipSync : public IModuleInterface
{

	int32 instanceCtr;

public:

	UFUNCTION(BlueprintCallable, Category = "Audio")
	static inline ILipSync& Get()
	{
		return FModuleManager::LoadModuleChecked< ILipSync >("OVRLipSync");
	}

	UFUNCTION(BlueprintCallable, Category = "Audio")
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("OVRLipSync");
	}

	UFUNCTION(BlueprintCallable, Category = "Audio")
	int32 GetInstanceCounter()
	{
		return instanceCtr++;
	}

};

