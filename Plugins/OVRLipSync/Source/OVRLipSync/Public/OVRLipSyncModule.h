// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "ModuleManager.h"
#include "IPluginManager.h"
#include <ILipSync.h>


class FOVRLipSyncModule : public ILipSync
{
private:


public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

};