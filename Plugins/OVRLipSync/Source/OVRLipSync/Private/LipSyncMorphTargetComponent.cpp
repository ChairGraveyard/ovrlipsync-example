#include "OVRLipSyncPrivatePCH.h"
#include "LipSyncMorphTargetComponent.h"

ULipSyncMorphTargetComponent::ULipSyncMorphTargetComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    struct FConstructorStatics
    {
        FName MouseMeshTag;

        FConstructorStatics()
        {
            MouseMeshTag = "Mouse";
        }
    };
    static FConstructorStatics ConstructorStatics;
    MouseMeshTag = ConstructorStatics.MouseMeshTag;
}

void ULipSyncMorphTargetComponent::BeginPlay()
{
    Super::BeginPlay();

    auto Components = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), MouseMeshTag);
    if (Components.Num() > 0)
    {
        MouseMesh = (USkeletalMeshComponent*)(Components[0]);
        check(MouseMesh != nullptr);
    }
}

void ULipSyncMorphTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

}

void ULipSyncMorphTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (MouseMesh == nullptr)
        return;
    FOVRLipSyncFrame LipSyncFrame;
    if (ovrLipSyncSuccess == GetPhonemeFrame(&LipSyncFrame))
    {
        for (int i = 0; i < (int)ovrLipSyncViseme::VisemesCount; ++i)
        {
            float VisemeValue = LipSyncFrame.Visemes[i];
            MouseMesh->SetMorphTarget(VisemeNames[i], VisemeValue, false);
        }
    }
}
