#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "TimerManager.h"
#include "ContractoryMissionManager.h"
#include "ContractoryMissionSubsystem.generated.h"

class AContractoryMissionSaveManager;
class AFGPlayerState;

UCLASS()
class SATISFACTORYREVERSE_API AContractoryMissionSubsystem : public AModSubsystem
{
    GENERATED_BODY()

public:
    AContractoryMissionSubsystem();

    // Trusted server-side call only.
    bool AcceptMissionForPlayer(
        AFGPlayerState* PlayerState,
        int32 MissionId
    );

    // Builds only the state belonging to the trusted server PlayerState.
    bool ClaimMissionForPlayer(
        AFGPlayerState* PlayerState,
        int32 MissionId,
        int32& OutCreditsAwarded
    );

    bool RefreshOffersForPlayer(
        AFGPlayerState* PlayerState
    );

    bool GetMissionStateForPlayer(
        AFGPlayerState* PlayerState,
        int32& OutCredits,
        TArray<FContractoryMissionDefinition>& OutOffers,
        TArray<FContractoryActiveMission>& OutActiveMissions
    );


    // Server only. Purchases one validated Boutique schematic
    // using the shared Contractory credit balance.
    bool PurchaseBoutiqueSchematicForPlayer(
        AFGPlayerState* PlayerState,
        const FString& SchematicClassPath,
        int32& OutRemainingCredits,
        FString& OutFailureReason
    );
protected:
    virtual void Init() override;

private:
    void TryUnlockContractoryMachines();

    UPROPERTY(Transient)
    TObjectPtr<AContractoryMissionSaveManager> MissionSaveManager;

    FTimerHandle MachineUnlockRetryTimer;
};

