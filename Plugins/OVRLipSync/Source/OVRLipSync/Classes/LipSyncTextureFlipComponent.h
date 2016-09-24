#pragma once

#include "OVRLipSyncContextComponent.h"
#include "LipSyncTextureFlipComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API ULipSyncTextureFlipComponent : public UOVRLipSyncContextComponent
{
    GENERATED_UCLASS_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
    UPROPERTY(Category = Visemes, EditDefaultsOnly, BlueprintReadOnly)
    TArray<class UTexture*> VisemeTextures;
    UPROPERTY(Category = Visemes, EditDefaultsOnly, BlueprintReadWrite)
    class UMaterialInstanceDynamic* MouseMaterail;
    UPROPERTY(Category = Visemes, EditDefaultsOnly, BlueprintReadOnly)
    FName MouseTextureName;
};
