#include "OVRLipSyncPrivatePCH.h"
#include "VisemeGenerationActor.h"

#define LIPSYNCPLUGIN ILIpSync::Get()
#define LOCTEXT_NAMESPACE "FOVRLipSyncModule"

DEFINE_LOG_CATEGORY_STATIC(OVRLipSyncPluginLog, Log, All);

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

dll_ovrlipsyncInitialize OVRLipSyncInit;
dll_ovrlipsyncShutdown OVRLipSyncShutdown;
dll_ovrlipsyncGetVersion OVRLipSyncGetVersion;
dll_ovrlipsyncCreateContext OVRLipSyncCreateContext;
dll_ovrlipsyncDestroyContext OVRLipSyncDestroyContext;
dll_ovrlipsyncResetContext OVRLipSyncResetContext;
dll_ovrlipsyncSendSignal OVRLipSyncSendSignal;
dll_ovrlipsyncProcessFrame OVRLipSyncProcessFrame;
dll_ovrlipsyncProcessFrameInterleaved OVRLipSyncProcessFrameInterleaved;


AVisemeGenerationActor::AVisemeGenerationActor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
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
#if _WIN64
	//64bit
	OVRDLLString = FString(TEXT("OVRLipSync_x64.dll"));
	PlatformString = FString(TEXT("Win64"));
#else
	//32bit
	OVRDLLString = FString(TEXT("OVRLipSync.dll"));
	PlatformString = FString(TEXT("Win32"));
#endif
#else
	UE_LOG(OVRLipSyncPluginLog, Error, TEXT("Unsupported Platform. Hydra Unavailable."));
#endif

	FString DllFilepath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*PluginRoot, TEXT("Binaries"), *PlatformString, *OVRDLLString));

#if PLATFORM_WINDOWS

	UE_LOG(OVRLipSyncPluginLog, Log, TEXT("Fetching dll from %s"), *DllFilepath);

	//Check if the file exists, if not, give a detailed log entry why
	if (!FPaths::FileExists(DllFilepath)) {
		UE_LOG(OVRLipSyncPluginLog, Error, TEXT("%s File is missing (Did you copy Binaries into project root?)! Hydra Unavailable."), *OVRDLLString);
		return;
	}

	OVRDLLHandle = NULL;
	OVRDLLHandle = FPlatformProcess::GetDllHandle(*DllFilepath);

	if (!OVRDLLHandle) {
		UE_LOG(OVRLipSyncPluginLog, Error, TEXT("GetDllHandle failed, Hydra Unavailable."));
		UE_LOG(OVRLipSyncPluginLog, Error, TEXT("Full path debug: %s."), *DllFilepath);
		return;
	}

	OVRLipSyncInit = (dll_ovrlipsyncInitialize)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_Initialize"));
	OVRLipSyncShutdown = (dll_ovrlipsyncShutdown)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_Shutdown"));
	OVRLipSyncGetVersion = (dll_ovrlipsyncGetVersion)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_GetVersion"));
	OVRLipSyncCreateContext = (dll_ovrlipsyncCreateContext)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_CreateContext"));
	OVRLipSyncDestroyContext = (dll_ovrlipsyncDestroyContext)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_DestroyContext"));
	OVRLipSyncResetContext = (dll_ovrlipsyncResetContext)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_ResetContext"));
	OVRLipSyncSendSignal = (dll_ovrlipsyncSendSignal)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_SendSignal"));
	OVRLipSyncProcessFrame = (dll_ovrlipsyncProcessFrame)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_ProcessFrame"));
	OVRLipSyncProcessFrameInterleaved = (dll_ovrlipsyncProcessFrameInterleaved)FPlatformProcess::GetDllExport(OVRDLLHandle, TEXT("ovrLipSyncDll_ProcessFrameInterleaved"));



#endif // PLATFORM_WINDOWS
}

bool AVisemeGenerationActor::Init()
{
	LastFrame.FrameDelay = LastFrame.FrameNumber = CurrentFrame.FrameDelay = CurrentFrame.FrameNumber = 0;

	LastFrame.Visemes.Empty();
	CurrentFrame.Visemes.Empty();

	LastFrame.Visemes.AddDefaulted((int)ovrLipSyncViseme::VisemesCount);
	CurrentFrame.Visemes.AddDefaulted((int)ovrLipSyncViseme::VisemesCount);

	// terminate any existing thread
	if (listenerThread != NULL)
		Shutdown();

	// start listener thread
	listenerThread = new FVisemeGenerationWorker();
	//TArray<FRecognitionKeyWord> dictionaryList;
	//listenerThread->SetLanguage(language);
	//listenerThread->AddWords(wordList);
	bool threadSuccess = listenerThread->StartThread(this);

	return threadSuccess;
}

bool AVisemeGenerationActor::Shutdown()
{	
	if (listenerThread != NULL) {
		listenerThread->ShutDown();
		listenerThread = NULL;
		return true;
	}
	else{
		return false;
	}
}

void AVisemeGenerationActor::VisemeGenerated_trigger(FVisemeGeneratedSignature delegate_method, FOVRLipSyncFrame LipSyncFrame)
{
	delegate_method.Broadcast(LipSyncFrame);
}

void AVisemeGenerationActor::VisemeGenerated_method(FOVRLipSyncFrame LipSyncFrame)
{
	FSimpleDelegateGraphTask::CreateAndDispatchWhenReady
		(
		FSimpleDelegateGraphTask::FDelegate::CreateStatic(&VisemeGenerated_trigger, OnVisemeGenerated, LipSyncFrame)
		, TStatId()
		, nullptr
		, ENamedThreads::GameThread
		);


}

int AVisemeGenerationActor::CreateContextExternal()
{
	int result = CreateContext(&CurrentContext, ContextProvider);
	if (result != ovrLipSyncSuccess)
		UE_LOG(OVRLipSyncPluginLog, Error, TEXT("Unable to create OVR LipSync Context"));

	return result;
}

int AVisemeGenerationActor::DestroyContextExternal()
{
	return DestroyContext(CurrentContext);
}

void AVisemeGenerationActor::ClearCurrentFrame()
{
	CurrentFrame.Visemes.Empty();
	CurrentFrame.Visemes.AddDefaulted((int)ovrLipSyncViseme::VisemesCount);
}

int AVisemeGenerationActor::InitLipSync(int SampleRate, int BufferSize)
{
	sOVRLipSyncInit = OVRLipSyncInit(SampleRate, BufferSize);
	return sOVRLipSyncInit;
}

void AVisemeGenerationActor::ShutdownLipSync()
{
	OVRLipSyncShutdown();
}

int AVisemeGenerationActor::CreateContext(unsigned int *Context, ovrLipSyncContextProvider Provider)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::CannotCreateContext;

	return OVRLipSyncCreateContext(Context, Provider);
}

int AVisemeGenerationActor::DestroyContext(unsigned int Context)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::Unknown;

	return OVRLipSyncDestroyContext(Context);
}

int AVisemeGenerationActor::ResetContext(unsigned int Context)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::Unknown;

	return OVRLipSyncResetContext(Context);
}

int AVisemeGenerationActor::SendSignal(unsigned int Context, ovrLipSyncSignals Signal, int Arg1, int Arg2)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::Unknown;

	return OVRLipSyncSendSignal(Context, Signal, Arg1, Arg2);
}

int AVisemeGenerationActor::ProcessFrame(unsigned int Context, float *AudioBuffer, ovrLipSyncFlag Flags, FOVRLipSyncFrame *Frame)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::Unknown;

	return OVRLipSyncProcessFrame(Context, AudioBuffer, Flags, &Frame->FrameNumber, &Frame->FrameDelay, Frame->Visemes.GetData(), Frame->Visemes.Num());
}

int AVisemeGenerationActor::ProcessFrameInterleaved(unsigned int Context, float *AudioBuffer, ovrLipSyncFlag Flags, FOVRLipSyncFrame *Frame)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::Unknown;

	return OVRLipSyncProcessFrameInterleaved(Context, AudioBuffer, Flags, &Frame->FrameNumber, &Frame->FrameDelay, Frame->Visemes.GetData(), Frame->Visemes.Num());
}

int AVisemeGenerationActor::ProcessFrameExternal(float *AudioBuffer, ovrLipSyncFlag Flags)
{
	//LastFrame.CopyInput(CurrentFrame);
	return ProcessFrame(CurrentContext, AudioBuffer, Flags, &CurrentFrame);
}

int AVisemeGenerationActor::GetPhonemeFrame(FOVRLipSyncFrame *OutFrame)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return (int)ovrLipSyncError::Unknown;

	OutFrame->CopyInput(CurrentFrame);

	return ovrLipSyncSuccess;
}

void AVisemeGenerationActor::GetFrameInfo(int *OutFrameNumber, int *OutFrameDelay, TArray<float> *OutVisemes)
{
	if (IsInitialized() != ovrLipSyncSuccess)
		return;

	*OutFrameNumber = CurrentFrame.FrameNumber;
	*OutFrameDelay = CurrentFrame.FrameDelay;
	OutVisemes->Append(CurrentFrame.Visemes);
}