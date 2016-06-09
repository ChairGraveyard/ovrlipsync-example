
#pragma once

#include "IPluginManager.h"
#include "TaskGraphInterfaces.h"
#include "VisemeGenerationWorker.h"
#include "VisemeGenerationActor.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVisemeGeneratedSignature, FString, Text);

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVisemeGeneratedSignature, int32, FrameNumber, int32, FrameDelay, TArray<float>, Visemes);


// Error codes that may return from Lip-Sync engine
enum ovrLipSyncError
{
	Unknown = -2200,	//< An unknown error has occurred
	CannotCreateContext = -2201, 	//< Unable to create a context
	InvalidParam = -2202,	//< An invalid parameter, e.g. NULL pointer or out of range
	BadSampleRate = -2203,	//< An unsupported sample rate was declared
	MissingDLL = -2204,	//< The DLL or shared library could not be found
	BadVersion = -2205,	//< Mismatched versions between header and libs
	UndefinedFunction = -2206	//< An undefined function 
};

// Various visemes
enum ovrLipSyncViseme
{
	sil,
	PP,
	FF,
	TH,
	DD,
	kk,
	CH,
	SS,
	nn,
	RR,
	aa,
	E,
	ih,
	oh,
	ou,
	VisemesCount
};

/// Flags
enum ovrLipSyncFlag
{
	None = 0x0000,
	DelayCompensateAudio = 0x0001

};

// Enum for sending lip-sync engine specific signals
enum ovrLipSyncSignals
{
	VisemeOn,
	VisemeOff,
	VisemeAmount,
	VisemeSmoothing,
	SignalCount
};

// Enum for provider context to create
enum ovrLipSyncContextProvider
{
	Main,
	Other
};

USTRUCT(BlueprintType)
struct FOVRLipSyncFrame
{
	GENERATED_BODY()

	FOVRLipSyncFrame()
	{
		FrameNumber = 0;
		FrameDelay = 0;
		Visemes.AddDefaulted((int)ovrLipSyncViseme::VisemesCount);		
	}

	void CopyInput(FOVRLipSyncFrame &input)
	{
		FrameNumber = input.FrameNumber;
		FrameDelay = input.FrameDelay;
		Visemes.Empty();
		Visemes.Append(input.Visemes);		
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = LipSync)
		int   	FrameNumber; 		// count from start of recognition

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = LipSync)
		int   	FrameDelay;  		// in ms

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = LipSync)
		TArray<float> 	Visemes;	// Array of floats for viseme frame. Size of Viseme Count, above
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVisemeGeneratedSignature, FOVRLipSyncFrame, LipSyncFrame);

UCLASS(BlueprintType, Blueprintable)
class OVRLIPSYNC_API AVisemeGenerationActor : public AActor
{
	GENERATED_UCLASS_BODY()

private:

	void* OVRDLLHandle;
	int sOVRLipSyncInit;


	unsigned int CurrentContext = 0;

	ovrLipSyncContextProvider ContextProvider = ovrLipSyncContextProvider::Main;

	
	FOVRLipSyncFrame LastFrame;

	int32 instanceCtr;
	
	FVisemeGenerationWorker* listenerThread;

	//static void VisemeGenerated_trigger(FVisemeGeneratedSignature delegate_method, FString text);
	//static void VisemeGenerated_trigger(FVisemeGeneratedSignature delegate_method, int32 FrameNumber, int32 FrameDelay, TArray<float> Visemes);
	static void VisemeGenerated_trigger(FVisemeGeneratedSignature delegate_method, FOVRLipSyncFrame LipSyncFrame);

public:
	
	AVisemeGenerationActor();

	const int ovrLipSyncSuccess = 0;
	FOVRLipSyncFrame CurrentFrame;

	// Basic functions 
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "Init", Keywords = "Viseme Generation Init"))		
	bool Init();

	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "Shutdown", Keywords = "Viseme Generation Shutdown"))
	bool Shutdown();

	//void VisemeGenerated_method(int32 FrameNumber, int32 FrameDelay, TArray<float> Visemes);
	UFUNCTION()
	void VisemeGenerated_method(FOVRLipSyncFrame LipSyncFrame);

	UPROPERTY(BlueprintAssignable, Category = "Audio")
	FVisemeGeneratedSignature OnVisemeGenerated;

	int IsInitialized() { return sOVRLipSyncInit; }

	int InitLipSync(int SampleRate, int BufferSize);
	void ShutdownLipSync();
	int CreateContext(unsigned int *Context, ovrLipSyncContextProvider Provider);
	int DestroyContext(unsigned int Context);
	int ResetContext(unsigned int Context);
	int SendSignal(unsigned int Context, ovrLipSyncSignals Signal, int Arg1, int Arg2);
	int ProcessFrame(unsigned int Context, float *AudioBuffer, ovrLipSyncFlag Flags, FOVRLipSyncFrame *Frame);
	int ProcessFrameInterleaved(unsigned int Context, float *AudioBuffer, ovrLipSyncFlag Flags, FOVRLipSyncFrame *Frame);

	int ProcessFrameExternal(float *AudioBuffer, ovrLipSyncFlag Flags);

	int CreateContextExternal();
	int DestroyContextExternal();

	void ClearCurrentFrame();

	int GetPhonemeFrame(FOVRLipSyncFrame *OutFrame);

	void GetFrameInfo(int *OutFrameNumber, int *OutFrameDelay, TArray<float> *OutVisemes);
};
