#pragma once

#include "OVRLipSyncContextComponent.h"
#include "LipSyncMorphTargetComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API ULipSyncMorphTargetComponent : public UOVRLipSyncContextComponent
{
    GENERATED_UCLASS_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
    UPROPERTY(Category = Visemes, EditDefaultsOnly, BlueprintReadOnly)
    FName MouseMeshTag;

private:
    UPROPERTY(Transient)
    class USkeletalMeshComponent* MouseMesh;
};
