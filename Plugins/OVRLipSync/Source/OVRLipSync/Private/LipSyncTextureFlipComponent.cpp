#include "OVRLipSyncPrivatePCH.h"
#include "LipSyncTextureFlipComponent.h"

ULipSyncTextureFlipComponent::ULipSyncTextureFlipComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    struct FConstructorStatics
    {
        FName MouseTextureName;

        FConstructorStatics()
        {
            MouseTextureName = "Mouse";
        }
    };
    static FConstructorStatics ConstructorStatics;
    MouseTextureName = ConstructorStatics.MouseTextureName;
}

void ULipSyncTextureFlipComponent::BeginPlay()
{
    Super::BeginPlay();

    check(VisemeTextures.Num() == (int)ovrLipSyncViseme::VisemesCount);
}

void ULipSyncTextureFlipComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

}

void ULipSyncTextureFlipComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (MouseMaterail == nullptr)
        return;
    FOVRLipSyncFrame LipSyncFrame;
    if (ovrLipSyncSuccess == GetPhonemeFrame(&LipSyncFrame))
    {
        int MaxValueIndex = 0;
        float MaxValue = -FLT_MAX;
        for (int i = 0; i < (int)ovrLipSyncViseme::VisemesCount; ++i)
        {
            float VisemeValue = LipSyncFrame.Visemes[i];
            if (VisemeValue > MaxValue)
            {
                MaxValue = VisemeValue;
                MaxValueIndex = i;
            }
        }
        MouseMaterail->SetTextureParameterValue(MouseTextureName, VisemeTextures[MaxValueIndex]);
    }
}
