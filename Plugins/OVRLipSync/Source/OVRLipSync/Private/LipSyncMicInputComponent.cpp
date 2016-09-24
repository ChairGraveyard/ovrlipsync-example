#include "OVRLipSyncPrivatePCH.h"
#include "LipSyncMicInputComponent.h"
#include "OVRLipSyncContextComponent.h"

#include "VoiceCapture.h"
#include "VoiceModule.h"


ULipSyncMicInputComponent::ULipSyncMicInputComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
}

void ULipSyncMicInputComponent::BeginPlay()
{
    Super::BeginPlay();

    LipSyncContext = GetOwner()->FindComponentByClass<UOVRLipSyncContextComponent>();
    check(LipSyncContext != nullptr);

    VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();
    check(VoiceCapture.IsValid());
    VoiceCapture->Start();
}

void ULipSyncMicInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    VoiceCapture->Shutdown();
}

void ULipSyncMicInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // should be moved to background thread for better performance
    uint32 BytesAvailable = 0;
    EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(BytesAvailable);
    if (CaptureState == EVoiceCaptureState::Ok)
    {
        TempBuffer.SetNumUninitialized(BytesAvailable);
        VoiceCapture->GetVoiceData(TempBuffer.GetData(), BytesAvailable, BytesAvailable);
        VoiceBuffer.Append(TempBuffer);
    }
    if (VoiceBuffer.Num() >= VISEME_BUF_SIZE)
    {
        LipSyncContext->ProcessFrame(VoiceBuffer.GetData(), VISEME_BUF_SIZE);
        VoiceBuffer.RemoveAt(0, VISEME_BUF_SIZE);
    }
}
