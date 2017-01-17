#pragma once
#include "Components/ActorComponent.h"
#include "LipSyncSoundWaveComponent.generated.h"

// example micro input for lipsync
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API ULipSyncSoundWaveComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	class UOVRLipSyncContextComponent* LipSyncContext;

	// Reference to the SoundWave we are decompressing
	//USoundWave* CompressedSoundWaveRef;
	
	TArray<uint8> SoundBuffer;
	TArray<uint8> TempBuffer;
};
