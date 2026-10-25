#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContractorySecondaryMissions.h"
#include "ContractoryMissionManager.generated.h"

USTRUCT(BlueprintType)
struct FContractoryActiveMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Contractory|Missions")
    int32 MissionId = 0;

    // Ancien champ conserve pour compatibilite sauvegarde.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Contractory|Missions")
    int32 DeliveredAmount = 0;

    // Nouveau : progression de chaque Requirement dans le meme ordre.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Contractory|Missions")
    TArray<int32> DeliveredAmounts;
};

USTRUCT(BlueprintType)
struct FContractoryMissionSession
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Contractory|Missions")
    TArray<FContractoryActiveMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Contractory|Missions")
    TArray<int32> CompletedMissionIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Contractory|Missions")
    int32 Credits = 0;
};

UCLASS()
class SATISFACTORYREVERSE_API UContractoryMissionManager : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Contractory|Missions secondaires")
    static TArray<FContractoryMissionDefinition> GetFiveOffers(
        int32 UnlockedTier,
        int32 RotationIndex
    );

    UFUNCTION(BlueprintCallable, Category="Contractory|Missions secondaires")
    static bool AcceptMission(
        UPARAM(ref) FContractoryMissionSession& Session,
        int32 MissionId,
        int32 UnlockedTier
    );

    // Compatibilite ancien appel : livre sur la premiere ressource.
    UFUNCTION(BlueprintCallable, Category="Contractory|Missions secondaires")
    static bool RecordVerifiedDelivery(
        UPARAM(ref) FContractoryMissionSession& Session,
        int32 MissionId,
        int32 Quantity
    );

    // Nouveau chemin multi-ressources.
    static int32 RecordVerifiedDeliveryForItem(
        FContractoryMissionSession& Session,
        int32 MissionId,
        const FString& ItemKey,
        int32 Quantity
    );

    static bool IsMissionComplete(
        const FContractoryActiveMission& Active,
        const FContractoryMissionDefinition& Definition
    );

    static int32 GetDeliveredForRequirement(
        const FContractoryActiveMission& Active,
        int32 RequirementIndex
    );

    UFUNCTION(BlueprintCallable, Category="Contractory|Missions secondaires")
    static bool ClaimCompletedMission(
        UPARAM(ref) FContractoryMissionSession& Session,
        int32 MissionId,
        int32& CreditsAwarded
    );

    UFUNCTION(BlueprintPure, Category="Contractory|Missions secondaires")
    static int32 GetActiveMissionLimit() { return 3; }
};