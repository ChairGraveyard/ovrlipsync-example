// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "OVRLipSyncPrivatePCH.h"
#include "Voice.h"

//DLL import definition
typedef int(*dll_ovrlipsyncInitialize)(int, int);
typedef void(*dll_ovrlipsyncShutdown)(void);
typedef int(*dll_ovrlipsyncGetVersion)(int*, int*, int*);
typedef int(*dll_ovrlipsyncCreateContext)(unsigned int*, int);
typedef int(*dll_ovrlipsyncDestroyContext)(unsigned int);
typedef int(*dll_ovrlipsyncResetContext)(unsigned int);
typedef int(*dll_ovrlipsyncSendSignal)(unsigned int, int, int, int);
typedef int(*dll_ovrlipsyncProcessFrame)(unsigned int, float*, int, int*, int*, float*, int);
typedef int(*dll_ovrlipsyncProcessFrameInterleaved)(unsigned int, float*, int, int*, int*, float*, int);

dll_ovrlipsyncInitialize OVRLipSyncInitialize;
dll_ovrlipsyncShutdown OVRLipSyncShutdown;
dll_ovrlipsyncGetVersion OVRLipSyncGetVersion;
dll_ovrlipsyncCreateContext OVRLipSyncCreateContext;
dll_ovrlipsyncDestroyContext OVRLipSyncDestroyContext;
dll_ovrlipsyncResetContext OVRLipSyncResetContext;
dll_ovrlipsyncSendSignal OVRLipSyncSendSignal;
dll_ovrlipsyncProcessFrame OVRLipSyncProcessFrame;
dll_ovrlipsyncProcessFrameInterleaved OVRLipSyncProcessFrameInterleaved;

#define LOCTEXT_NAMESPACE "FOVRLipSyncModule"

void FOVRLipSyncModule::StartupModule()
{
    //Define Paths for direct dll bind
    FString BinariesRoot = FPaths::Combine(*FPaths::GameDir(), TEXT("Binaries"));
    IPluginManager &plgnMgr = IPluginManager::Get();
    TSharedPtr<IPlugin> plugin = plgnMgr.FindPlugin("OVRLipSync");
    if (!plugin.IsValid())
    {
        UE_LOG(OVRLipSyncPluginLog, Error, TEXT("Plugin not found."));
        return;
    }

    FString PluginRoot = plugin->GetBaseDir();
    FString PlatformString;
    FString OVRDLLString;

#if PLATFORM_WINDOWS
# if _WIN64
    //64bit
    OVRDLLString = FString(TEXT("OVRLipSync_x64.dll"));
    PlatformString = FString(TEXT("Win64"));
# else
    //32bit
    OVRDLLString = FString(TEXT("OVRLipSync.dll"));
    PlatformString = FString(TEXT("Win32"));
# endif

    FString DllFilepath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*PluginRoot, TEXT("Binaries"), *PlatformString, *OVRDLLString));

    UE_LOG(OVRLipSyncPluginLog, Log, TEXT("Fetching dll from %s"), *DllFilepath);

    //Check if the file exists, if not, give a detailed log entry why
    if (!FPaths::FileExists(DllFilepath))
    {
        //UE_LOG(OVRLipSyncPluginLog, Error, TEXT("%s File is missing (Did you copy Binaries into project root?)! Hydra Unavailable."), *OVRDLLString);
        return;
    }

    void* OVRDLLHandle = FPlatformProcess::GetDllHandle(*DllFilepath);

    if (!OVRDLLHandle)
    {
        UE_LOG(OVRLipSyncPluginLog, Error, TEXT("GetDllHandle failed, Hydra Unavailable."));
        UE_LOG(OVRLipSyncPluginLog, Error, TEXT("Full path debug: %s."), *DllFilepath);
        return;
    }

    OVRLipSyncInitialize = (dll_ovrlipsyncInitialize)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_Initialize"));
    OVRLipSyncShutdown = (dll_ovrlipsyncShutdown)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_Shutdown"));
    OVRLipSyncGetVersion = (dll_ovrlipsyncGetVersion)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_GetVersion"));
    OVRLipSyncCreateContext = (dll_ovrlipsyncCreateContext)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_CreateContext"));
    OVRLipSyncDestroyContext = (dll_ovrlipsyncDestroyContext)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_DestroyContext"));
    OVRLipSyncResetContext = (dll_ovrlipsyncResetContext)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_ResetContext"));
    OVRLipSyncSendSignal = (dll_ovrlipsyncSendSignal)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_SendSignal"));
    OVRLipSyncProcessFrame = (dll_ovrlipsyncProcessFrame)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_ProcessFrame"));
    OVRLipSyncProcessFrameInterleaved = (dll_ovrlipsyncProcessFrameInterleaved)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_ProcessFrameInterleaved"));

    FOVRLipSync::Initialize();
#else
    UE_LOG(OVRLipSyncPluginLog, Error, TEXT("Unsupported Platform. OVRLipSync Unavailable."));
#endif // PLATFORM_WINDOWS
}

void FOVRLipSyncModule::ShutdownModule()
{
    FOVRLipSync::Shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOVRLipSyncModule, OVRLipSync)

/////////////////////////////////////////////////////////////////////////////////////////////

int FOVRLipSync::sOVRLipSyncInit = -1;

void FOVRLipSync::Initialize()
{
    sOVRLipSyncInit = OVRLipSyncInitialize(VOICE_SAMPLE_RATE, VISEME_SAMPLES);
}

int FOVRLipSync::IsInitialized()
{
    return sOVRLipSyncInit;
}

void FOVRLipSync::Shutdown()
{
    OVRLipSyncShutdown();
}

int FOVRLipSync::CreateContext(unsigned int* Context, ovrLipSyncContextProvider Provider)
{
    if (IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::CannotCreateContext;

    return OVRLipSyncCreateContext(Context, Provider);
}

int FOVRLipSync::DestroyContext(unsigned int Context)
{
    if (IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::Unknown;

    return OVRLipSyncDestroyContext(Context);
}

int FOVRLipSync::ResetContext(unsigned int Context)
{
    if (IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::Unknown;

    return OVRLipSyncResetContext(Context);
}

int FOVRLipSync::SendSignal(unsigned int Context, ovrLipSyncSignals Signal, int Arg1, int Arg2)
{
    if (IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::Unknown;

    return OVRLipSyncSendSignal(Context, Signal, Arg1, Arg2);
}

int FOVRLipSync::ProcessFrame(unsigned int Context, float* AudioBuffer, ovrLipSyncFlag Flags, FOVRLipSyncFrame* Frame)
{
    if (IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::Unknown;

    // this function has some bugs(version 1.0.1), so use OVRLipSyncProcessFrameInterleaved instead
    return OVRLipSyncProcessFrame(Context, AudioBuffer, Flags, &Frame->FrameNumber, &Frame->FrameDelay, Frame->Visemes.GetData(), Frame->Visemes.Num());
}

int FOVRLipSync::ProcessFrameInterleaved(unsigned int Context, float* AudioBuffer, ovrLipSyncFlag Flags, FOVRLipSyncFrame* Frame)
{
    if (IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::Unknown;

    return OVRLipSyncProcessFrameInterleaved(Context, AudioBuffer, Flags, &Frame->FrameNumber, &Frame->FrameDelay, Frame->Visemes.GetData(), Frame->Visemes.Num());
}
