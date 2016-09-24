#pragma once
#include "Components/ActorComponent.h"
#include "LipSyncMicInputComponent.generated.h"

// example micro input for lipsync
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API ULipSyncMicInputComponent : public UActorComponent
{
    GENERATED_UCLASS_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY(Transient)
    class UOVRLipSyncContextComponent* LipSyncContext;

    TSharedPtr<class IVoiceCapture> VoiceCapture;
    TArray<uint8> VoiceBuffer;
    TArray<uint8> TempBuffer;
};
