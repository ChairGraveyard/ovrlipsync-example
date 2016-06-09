#include "OVRLipSyncPrivatePCH.h"
#include "VisemeGenerationWorker.h"

//General Log
//DEFINE_LOG_CATEGORY(OVRLipSyncPlugin);

FVisemeGenerationWorker::FVisemeGenerationWorker() {}


FVisemeGenerationWorker::~FVisemeGenerationWorker() {
	delete Thread;
	Thread = NULL;
}

void FVisemeGenerationWorker::ShutDown() {
	Stop();
	Thread->WaitForCompletion();
	delete Thread;

	Manager->DestroyContextExternal();
	Manager->ShutdownLipSync();
}

bool FVisemeGenerationWorker::Init() {


	if (!Manager) {
		ClientMessage(FString(TEXT("Speech Recognition Thread failed to start")));
		InitSuccess = false;
		return InitSuccess;
	}

	if (!VoiceCapture.IsValid() || !VoiceCapture->Start())
	{
		ClientMessage(FString(TEXT("Failed to open audio device")));
		InitSuccess = false;
		return InitSuccess;
	}


	Manager->InitLipSync(16000, VISEME_SAMPLES);
	Manager->CreateContextExternal();

	memset(buf, 0, VISEME_BUF_SIZE);
	memset(sampleBuf, 0, VISEME_SAMPLES * sizeof(float));

	InitSuccess = true;

	return InitSuccess;
}

uint32 FVisemeGenerationWorker::Run() {

	if (!Manager || Manager->ovrLipSyncSuccess < 0)
	{
		ClientMessage(FString(TEXT("Pending delete on Manager object")));
		return 1;
	}
	
	if (!VoiceCapture->Start())
	{
		ClientMessage(FString(TEXT("Failed to start recording")));
		return 2;
	}

	if (Manager->IsInitialized() != Manager->ovrLipSyncSuccess)
	{
		ClientMessage(FString(TEXT("OVR Lip Sync not initialized")));
		return 3;
	}

	while (StopTaskCounter.GetValue() == 0)
	{
		// Capturing samples:
		uint32 bytesAvailable = 0;
		EVoiceCaptureState::Type captureState = VoiceCapture->GetCaptureState(bytesAvailable);
		if (captureState == EVoiceCaptureState::Ok && bytesAvailable > 0)
		{
			memset(buf, 0, VISEME_BUF_SIZE);

			uint32 readBytes = 0;
			VoiceCapture->GetVoiceData(buf, VISEME_BUF_SIZE, readBytes);

			int16_t sample;
			for (uint32 i = 0; i < VISEME_SAMPLES; i++)
			{
				sample = (buf[i * 2 + 1] << 8) | buf[i * 2];
				sampleBuf[i] = float(sample) / 32768.0f;
			}

			// Do fun stuff here
			uint32 Flags = 0;

			Manager->ProcessFrameExternal(sampleBuf, (ovrLipSyncFlag)Flags);

			Manager->VisemeGenerated_method(Manager->CurrentFrame);
		}
	}

	VoiceCapture->Stop();

	return 0;
}

void FVisemeGenerationWorker::Stop() {
	StopTaskCounter.Increment();
}

bool FVisemeGenerationWorker::StartThread(AVisemeGenerationActor* manager) {
	Manager = manager;
	
	// Initialisation:
	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();
	if (!VoiceCapture.IsValid())
		InitSuccess = false;

	int32 threadIdx = ILipSync::Get().GetInstanceCounter();
	FString threadName = FString("FVisemeGenerationWorker:") + FString::FromInt(threadIdx);
	InitSuccess = true;
	Thread = FRunnableThread::Create(this, *threadName, 0U, TPri_Highest);

	return InitSuccess;
}

void FVisemeGenerationWorker::ClientMessage(FString text) {
	UE_LOG(OVRLipSyncPluginLog, Log, TEXT("%s"), *text);
}
