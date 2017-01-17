#include "OVRLipSyncPrivatePCH.h"
#include "LipSyncSoundWaveComponent.h"
#include "OVRLipSyncContextComponent.h"


ULipSyncSoundWaveComponent::ULipSyncSoundWaveComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULipSyncSoundWaveComponent::BeginPlay()
{
	Super::BeginPlay();

	LipSyncContext = GetOwner()->FindComponentByClass<UOVRLipSyncContextComponent>();
	check(LipSyncContext != nullptr);
}

void ULipSyncSoundWaveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ULipSyncSoundWaveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// should be moved to background thread for better performance
	uint32 BytesAvailable = 0;
	//EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(BytesAvailable);
	//if (CaptureState == EVoiceCaptureState::Ok)
	{
		TempBuffer.SetNumUninitialized(BytesAvailable);
		//VoiceCapture->GetVoiceData(TempBuffer.GetData(), BytesAvailable, BytesAvailable);
		SoundBuffer.Append(TempBuffer);
	}
	if (SoundBuffer.Num() >= VISEME_BUF_SIZE)
	{
		LipSyncContext->ProcessFrame(SoundBuffer.GetData(), VISEME_BUF_SIZE);
		SoundBuffer.RemoveAt(0, VISEME_BUF_SIZE);
	}
}
