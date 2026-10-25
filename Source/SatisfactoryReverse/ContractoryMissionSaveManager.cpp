#include "ContractoryMissionSaveManager.h"

#include "Net/UnrealNetwork.h"

AContractoryMissionSaveManager::
AContractoryMissionSaveManager()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    SharedState.PlayerKey =
        TEXT("CONTRACTORY_SHARED");
}

void AContractoryMissionSaveManager::
GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(
        OutLifetimeProps
    );

    DOREPLIFETIME(
        AContractoryMissionSaveManager,
        SharedState
    );
}

void AContractoryMissionSaveManager::
PreSaveGame_Implementation(
    int32 SaveVersion,
    int32 GameVersion
)
{
}

void AContractoryMissionSaveManager::
PostSaveGame_Implementation(
    int32 SaveVersion,
    int32 GameVersion
)
{
}

void AContractoryMissionSaveManager::
PreLoadGame_Implementation(
    int32 SaveVersion,
    int32 GameVersion
)
{
}

void AContractoryMissionSaveManager::
PostLoadGame_Implementation(
    int32 SaveVersion,
    int32 GameVersion
)
{
    if (!HasAuthority())
    {
        return;
    }

    // Migration automatique depuis l'ancien systeme
    // ou chaque joueur avait son propre etat.
    if (
        PlayerStates.Num() > 0 &&
        SharedState.Session.ActiveMissions.Num() == 0 &&
        SharedState.Session.CompletedMissionIds.Num() == 0 &&
        SharedState.Session.Credits == 0 &&
        SharedState.RotationIndex == 0
    )
    {
        SharedState = PlayerStates[0];

        SharedState.PlayerKey =
            TEXT("CONTRACTORY_SHARED");

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[Contractory] Legacy player mission state "
                "migrated to shared cooperative state."
            )
        );
    }

    SharedState.PlayerKey =
        TEXT("CONTRACTORY_SHARED");

    // Une fois migre, l'ancien stockage ne sert plus.
    PlayerStates.Empty();

    ForceNetUpdate();
}

void AContractoryMissionSaveManager::
GatherDependencies_Implementation(
    TArray<UObject*>& OutDependentObjects
)
{
}

bool AContractoryMissionSaveManager::
ShouldSave_Implementation() const
{
    return true;
}

bool AContractoryMissionSaveManager::
NeedTransform_Implementation()
{
    return false;
}

FContractoryPlayerMissionState*
AContractoryMissionSaveManager::
FindOrAddServerPlayer(
    const FString& TrustedPlayerKey
)
{
    if (!HasAuthority())
    {
        return nullptr;
    }

    SharedState.PlayerKey =
        TEXT("CONTRACTORY_SHARED");

    return &SharedState;
}

const FContractoryPlayerMissionState*
AContractoryMissionSaveManager::
FindPlayer(
    const FString& PlayerKey
) const
{
    return &SharedState;
}
