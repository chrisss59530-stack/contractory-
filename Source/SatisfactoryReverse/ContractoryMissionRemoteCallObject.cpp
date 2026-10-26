#include "ContractoryMissionRemoteCallObject.h"

#include "ContractoryMissionSubsystem.h"
#include "ContractoryFinalGate.h"

#include "FGPlayerController.h"
#include "FGPlayerState.h"

#include "Subsystem/SubsystemActorManager.h"
#include "Net/UnrealNetwork.h"

namespace
{
    AContractoryMissionSubsystem* FindMissionSubsystem(
        UWorld* World
    )
    {
        if (!World)
        {
            return nullptr;
        }

        USubsystemActorManager* SubsystemActorManager =
            World->GetSubsystem<USubsystemActorManager>();

        if (!SubsystemActorManager)
        {
            return nullptr;
        }

        return SubsystemActorManager
            ->GetSubsystemActor<AContractoryMissionSubsystem>();
    }
}

void UContractoryMissionRemoteCallObject::
Server_MarkFinalPortalTraversed_Implementation()
{
    AFGPlayerController* PC = GetOwnerPlayerController();

    if (!PC || !PC->HasAuthority())
    {
        return;
    }

    const bool bSuccess =
        UContractoryFinalGateLibrary::
            MarkFinalPortalTraversed(PC);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[CONTRACTORY FINAL] Validation serveur : %s"),
        bSuccess ? TEXT("OUI") : TEXT("NON")
    );
}
void UContractoryMissionRemoteCallObject::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(
        UContractoryMissionRemoteCallObject,
        mForceNetField_UContractoryMissionRemoteCallObject
    );
}

void UContractoryMissionRemoteCallObject::
Server_RequestMissionState_Implementation()
{
    AFGPlayerController* PlayerController =
        GetOwnerPlayerController();

    if (!PlayerController)
    {
        return;
    }

    AFGPlayerState* PlayerState =
        PlayerController->GetPlayerState<AFGPlayerState>();

    if (!PlayerState)
    {
        return;
    }

    AContractoryMissionSubsystem* MissionSubsystem =
        FindMissionSubsystem(GetWorld());

    if (!MissionSubsystem)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory] Mission subsystem not found from RCO.")
        );

        return;
    }

    int32 Credits = 0;
    TArray<FContractoryMissionDefinition> Offers;
    TArray<FContractoryActiveMission> ActiveMissions;

    if (MissionSubsystem->GetMissionStateForPlayer(
            PlayerState,
            Credits,
            Offers,
            ActiveMissions
        ))
    {
        Client_ReceiveMissionState(
            Credits,
            Offers,
            ActiveMissions
        );
    }
}

void UContractoryMissionRemoteCallObject::
Server_AcceptMission_Implementation(int32 MissionId)
{
    AFGPlayerController* PlayerController =
        GetOwnerPlayerController();

    if (!PlayerController)
    {
        return;
    }

    AFGPlayerState* PlayerState =
        PlayerController->GetPlayerState<AFGPlayerState>();

    if (!PlayerState)
    {
        return;
    }

    AContractoryMissionSubsystem* MissionSubsystem =
        FindMissionSubsystem(GetWorld());

    if (!MissionSubsystem)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory] Mission subsystem not found from RCO.")
        );

        return;
    }

    MissionSubsystem->AcceptMissionForPlayer(
        PlayerState,
        MissionId
    );

    int32 Credits = 0;
    TArray<FContractoryMissionDefinition> Offers;
    TArray<FContractoryActiveMission> ActiveMissions;

    if (MissionSubsystem->GetMissionStateForPlayer(
            PlayerState,
            Credits,
            Offers,
            ActiveMissions
        ))
    {
        Client_ReceiveMissionState(
            Credits,
            Offers,
            ActiveMissions
        );
    }
}

void UContractoryMissionRemoteCallObject::
Client_ReceiveMissionState_Implementation(
    int32 Credits,
    const TArray<FContractoryMissionDefinition>& Offers,
    const TArray<FContractoryActiveMission>& ActiveMissions
)
{
    CachedCredits = Credits;
    CachedOffers = Offers;
    CachedActiveMissions = ActiveMissions;

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[Contractory] Mission state received: Credits=%d Offers=%d Active=%d"
        ),
        CachedCredits,
        CachedOffers.Num(),
        CachedActiveMissions.Num()
    );

    OnMissionStateUpdated.Broadcast();
}

void UContractoryMissionRemoteCallObject::
Server_ClaimMission_Implementation(int32 MissionId)
{
    AFGPlayerController* PlayerController =
        GetOwnerPlayerController();

    if (!PlayerController)
    {
        return;
    }

    AFGPlayerState* PlayerState =
        PlayerController->GetPlayerState<AFGPlayerState>();

    if (!PlayerState)
    {
        return;
    }

    AContractoryMissionSubsystem* MissionSubsystem =
        FindMissionSubsystem(GetWorld());

    if (!MissionSubsystem)
    {
        return;
    }

    int32 CreditsAwarded = 0;

    MissionSubsystem->ClaimMissionForPlayer(
        PlayerState,
        MissionId,
        CreditsAwarded
    );

    int32 Credits = 0;
    TArray<FContractoryMissionDefinition> Offers;
    TArray<FContractoryActiveMission> ActiveMissions;

    if (MissionSubsystem->GetMissionStateForPlayer(
            PlayerState,
            Credits,
            Offers,
            ActiveMissions
        ))
    {
        Client_ReceiveMissionState(
            Credits,
            Offers,
            ActiveMissions
        );
    }
}

void UContractoryMissionRemoteCallObject::
Server_RefreshOffers_Implementation()
{
    AFGPlayerController* PlayerController =
        GetOwnerPlayerController();

    if (!PlayerController)
    {
        return;
    }

    AFGPlayerState* PlayerState =
        PlayerController->GetPlayerState<AFGPlayerState>();

    if (!PlayerState)
    {
        return;
    }

    AContractoryMissionSubsystem* MissionSubsystem =
        FindMissionSubsystem(GetWorld());

    if (!MissionSubsystem)
    {
        return;
    }

    MissionSubsystem->RefreshOffersForPlayer(
        PlayerState
    );

    int32 Credits = 0;
    TArray<FContractoryMissionDefinition> Offers;
    TArray<FContractoryActiveMission> ActiveMissions;

    if (MissionSubsystem->GetMissionStateForPlayer(
            PlayerState,
            Credits,
            Offers,
            ActiveMissions
        ))
    {
        Client_ReceiveMissionState(
            Credits,
            Offers,
            ActiveMissions
        );
    }
}

void UContractoryMissionRemoteCallObject::
Server_PurchaseBoutiqueSchematic_Implementation(
    const FString& SchematicClassPath
)
{
    AFGPlayerController* PlayerController =
        GetOwnerPlayerController();

    if (!PlayerController)
    {
        return;
    }

    AFGPlayerState* PlayerState =
        PlayerController->GetPlayerState<AFGPlayerState>();

    if (!PlayerState)
    {
        Client_ReceiveBoutiquePurchaseResult(
            false,
            0,
            SchematicClassPath,
            TEXT("PlayerState indisponible")
        );

        return;
    }

    AContractoryMissionSubsystem* MissionSubsystem =
        FindMissionSubsystem(GetWorld());

    if (!MissionSubsystem)
    {
        Client_ReceiveBoutiquePurchaseResult(
            false,
            0,
            SchematicClassPath,
            TEXT("MissionSubsystem indisponible")
        );

        return;
    }

    int32 RemainingCredits = 0;
    FString FailureReason;

    const bool bSuccess =
        MissionSubsystem->PurchaseBoutiqueSchematicForPlayer(
            PlayerState,
            SchematicClassPath,
            RemainingCredits,
            FailureReason
        );

    Client_ReceiveBoutiquePurchaseResult(
        bSuccess,
        RemainingCredits,
        SchematicClassPath,
        FailureReason
    );

    //
    // Renvoie aussi l'etat Contractory actualise.
    // Le Terminal, la Livraison et la future Boutique
    // voient ainsi immédiatement le nouveau solde partagé.
    //
    int32 Credits = 0;
    TArray<FContractoryMissionDefinition> Offers;
    TArray<FContractoryActiveMission> ActiveMissions;

    if (MissionSubsystem->GetMissionStateForPlayer(
            PlayerState,
            Credits,
            Offers,
            ActiveMissions
        ))
    {
        Client_ReceiveMissionState(
            Credits,
            Offers,
            ActiveMissions
        );
    }
}

void UContractoryMissionRemoteCallObject::
Client_ReceiveBoutiquePurchaseResult_Implementation(
    bool bSuccess,
    int32 RemainingCredits,
    const FString& SchematicClassPath,
    const FString& FailureReason
)
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[Contractory][Boutique] Resultat achat: Success=%s Credits=%d Schematic=%s Reason=%s"
        ),
        bSuccess ? TEXT("true") : TEXT("false"),
        RemainingCredits,
        *SchematicClassPath,
        *FailureReason
    );

    OnBoutiquePurchaseResult.Broadcast(
        bSuccess,
        RemainingCredits,
        SchematicClassPath,
        FailureReason
    );
}