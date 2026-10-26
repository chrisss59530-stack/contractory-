#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "ContractoryMissionManager.h"
#include "ContractoryMissionRemoteCallObject.generated.h"

DECLARE_MULTICAST_DELEGATE(FContractoryMissionStateUpdated);

DECLARE_MULTICAST_DELEGATE_FourParams(
    FContractoryBoutiquePurchaseResult,
    bool,
    int32,
    const FString&,
    const FString&
);

UCLASS()
class SATISFACTORYREVERSE_API UContractoryMissionRemoteCallObject
    : public UFGRemoteCallObject
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UFUNCTION(Server, Reliable)
    void Server_MarkFinalPortalTraversed();
    UFUNCTION(Server, Reliable)
    void Server_RequestMissionState();

    UFUNCTION(Server, Reliable)
    void Server_AcceptMission(int32 MissionId);

    UFUNCTION(Server, Reliable)
    void Server_ClaimMission(int32 MissionId);

    UFUNCTION(Server, Reliable)
    void Server_RefreshOffers();

    UFUNCTION(Server, Reliable)
    void Server_PurchaseBoutiqueSchematic(
        const FString& SchematicClassPath
    );

    UFUNCTION(Client, Reliable)
    void Client_ReceiveMissionState(
        int32 Credits,
        const TArray<FContractoryMissionDefinition>& Offers,
        const TArray<FContractoryActiveMission>& ActiveMissions
    );

    UFUNCTION(Client, Reliable)
    void Client_ReceiveBoutiquePurchaseResult(
        bool bSuccess,
        int32 RemainingCredits,
        const FString& SchematicClassPath,
        const FString& FailureReason
    );
    int32 GetCachedCredits() const
    {
        return CachedCredits;
    }

    const TArray<FContractoryMissionDefinition>& GetCachedOffers() const
    {
        return CachedOffers;
    }

    const TArray<FContractoryActiveMission>& GetCachedActiveMissions() const
    {
        return CachedActiveMissions;
    }

    FContractoryMissionStateUpdated OnMissionStateUpdated;
    FContractoryBoutiquePurchaseResult OnBoutiquePurchaseResult;

private:
    UPROPERTY(Transient)
    int32 CachedCredits = 0;

    UPROPERTY(Transient)
    TArray<FContractoryMissionDefinition> CachedOffers;

    UPROPERTY(Transient)
    TArray<FContractoryActiveMission> CachedActiveMissions;

    UPROPERTY(Replicated, Meta=(NoAutoJson))
    bool mForceNetField_UContractoryMissionRemoteCallObject = false;
};
