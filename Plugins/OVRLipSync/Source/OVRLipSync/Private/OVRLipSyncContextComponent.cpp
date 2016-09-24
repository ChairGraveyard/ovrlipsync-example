#include "OVRLipSyncPrivatePCH.h"
#include "OVRLipSyncContextComponent.h"


UOVRLipSyncContextComponent::UOVRLipSyncContextComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    struct FConstructorStatics
    {
        TArray<FName> VisemeNames;

        FConstructorStatics()
        {
            VisemeNames.SetNum(ovrLipSyncViseme::VisemesCount);
            VisemeNames[ovrLipSyncViseme::sil] = "sil";
            VisemeNames[ovrLipSyncViseme::PP] = "PP";
            VisemeNames[ovrLipSyncViseme::FF] = "FF";
            VisemeNames[ovrLipSyncViseme::TH] = "TH";
            VisemeNames[ovrLipSyncViseme::DD] = "DD";
            VisemeNames[ovrLipSyncViseme::kk] = "kk";
            VisemeNames[ovrLipSyncViseme::CH] = "CH";
            VisemeNames[ovrLipSyncViseme::SS] = "SS";
            VisemeNames[ovrLipSyncViseme::nn] = "nn";
            VisemeNames[ovrLipSyncViseme::RR] = "RR";
            VisemeNames[ovrLipSyncViseme::aa] = "aa";
            VisemeNames[ovrLipSyncViseme::E] = "E";
            VisemeNames[ovrLipSyncViseme::ih] = "ih";
            VisemeNames[ovrLipSyncViseme::oh] = "oh";
            VisemeNames[ovrLipSyncViseme::ou] = "ou";
        }
    };
    static FConstructorStatics ConstructorStatics;
    VisemeNames = ConstructorStatics.VisemeNames;

    PrimaryComponentTick.bCanEverTick = true;
}

void UOVRLipSyncContextComponent::BeginPlay()
{
    Super::BeginPlay();

    FOVRLipSync::CreateContext(&CurrentContext, ContextProvider);
}

void UOVRLipSyncContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (CurrentContext)
    {
        FOVRLipSync::DestroyContext(CurrentContext);
    }
}

void UOVRLipSyncContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bDebugVisemes)
    {
        FOVRLipSyncFrame LipSyncFrame;
        if (ovrLipSyncSuccess == GetPhonemeFrame(&LipSyncFrame))
        {
            for (int i = 0; i < (int)ovrLipSyncViseme::VisemesCount; ++i)
            {
                float VisemeValue = LipSyncFrame.Visemes[i];
                FString OutputString = FString::Printf(TEXT("%s: %f"), *VisemeNames[i].ToString(), VisemeValue);
                GEngine->AddOnScreenDebugMessage((uint64)-1, DeltaTime, FColor::Green, OutputString);
            }
        }
    }
}

// maybe called in another thread
void UOVRLipSyncContextComponent::ProcessFrame(const uint8* AudioData, const int32 BufferSize)
{
    check(BufferSize >= VISEME_BUF_SIZE);
    if (CurrentContext == 0)
        return;

    for (uint32 i = 0; i < VISEME_SAMPLES; i++)
    {
        int16 Sample = (int16)(AudioData[i * 2 + 1] << 8 | AudioData[i * 2]);
        SampleBuffer[i * 2] = Sample / (float)SHRT_MAX;
        SampleBuffer[i * 2 + 1] = Sample / (float)SHRT_MAX;
    }
    FOVRLipSyncFrame TempFrame;
    FOVRLipSync::ProcessFrameInterleaved(CurrentContext, SampleBuffer, ovrLipSyncFlag::None, &TempFrame);

    // only lock the frame copy operation
    {
        FScopeLock LipSyncFrameLock(&LipSyncFrameCriticalSection);
        CurrentFrame = TempFrame;
    }

    // dispatch the event
    if (VisemeGenerated.IsBound())
    {
        FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
        {
            VisemeGenerated.Broadcast(CurrentFrame);
        }
        , TStatId(), nullptr, ENamedThreads::GameThread);
    }
}

int UOVRLipSyncContextComponent::GetPhonemeFrame(FOVRLipSyncFrame *OutFrame)
{
    if (FOVRLipSync::IsInitialized() != ovrLipSyncSuccess)
        return (int)ovrLipSyncError::Unknown;

    // only lock the frame copy operation
    {
        FScopeLock LipSyncFrameLock(&LipSyncFrameCriticalSection);
        *OutFrame = CurrentFrame;
    }

    return ovrLipSyncSuccess;
}
