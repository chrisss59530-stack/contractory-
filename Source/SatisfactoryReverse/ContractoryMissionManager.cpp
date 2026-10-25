#include "ContractoryMissionManager.h"

namespace
{
    bool FindDefinition(
        int32 MissionId,
        FContractoryMissionDefinition& Out
    )
    {
        for (
            const FContractoryMissionDefinition& Definition :
            UContractorySecondaryMissions::GetMissionCatalogue()
        )
        {
            if (Definition.MissionId == MissionId)
            {
                Out = Definition;
                return true;
            }
        }

        return false;
    }

    int32 FindActiveIndex(
        const FContractoryMissionSession& Session,
        int32 MissionId
    )
    {
        for (int32 i = 0; i < Session.ActiveMissions.Num(); ++i)
        {
            if (Session.ActiveMissions[i].MissionId == MissionId)
            {
                return i;
            }
        }

        return INDEX_NONE;
    }

    void EnsureProgressShape(
        FContractoryActiveMission& Active,
        const FContractoryMissionDefinition& Definition
    )
    {
        if (Active.DeliveredAmounts.Num() == Definition.Requirements.Num())
        {
            return;
        }

        TArray<int32> NewProgress;
        NewProgress.Init(0, Definition.Requirements.Num());

        // Migration douce d'une ancienne mission x1.
        if (NewProgress.Num() > 0 && Active.DeliveredAmount > 0)
        {
            NewProgress[0] = FMath::Min(
                Active.DeliveredAmount,
                Definition.Requirements[0].TargetAmount
            );
        }

        Active.DeliveredAmounts = MoveTemp(NewProgress);
    }
}

TArray<FContractoryMissionDefinition>
UContractoryMissionManager::GetFiveOffers(
    int32 UnlockedTier,
    int32 RotationIndex
)
{
    const TArray<FContractoryMissionDefinition> Pool =
        UContractorySecondaryMissions::GetAvailableMissions(UnlockedTier);

    TArray<FContractoryMissionDefinition> Offers;

    if (Pool.IsEmpty())
    {
        return Offers;
    }

    // Pas de rotation ni de limite de cinq offres : toutes les missions
    // debloquees sont visibles. La limite de trois concerne les ACTIVES.
    Offers = Pool;

    return Offers;
}

bool UContractoryMissionManager::AcceptMission(
    FContractoryMissionSession& Session,
    int32 MissionId,
    int32 UnlockedTier
)
{
    if (
        Session.ActiveMissions.Num() >= GetActiveMissionLimit() ||
        FindActiveIndex(Session, MissionId) != INDEX_NONE
    )
    {
        return false;
    }

    FContractoryMissionDefinition Definition;

    if (
        !FindDefinition(MissionId, Definition) ||
        Definition.MinimumTier > UnlockedTier ||
        Definition.Requirements.IsEmpty()
    )
    {
        return false;
    }

    FContractoryActiveMission NewMission;
    NewMission.MissionId = MissionId;
    NewMission.DeliveredAmount = 0;
    NewMission.DeliveredAmounts.Init(
        0,
        Definition.Requirements.Num()
    );

    Session.ActiveMissions.Add(MoveTemp(NewMission));
    return true;
}

int32 UContractoryMissionManager::GetDeliveredForRequirement(
    const FContractoryActiveMission& Active,
    int32 RequirementIndex
)
{
    if (Active.DeliveredAmounts.IsValidIndex(RequirementIndex))
    {
        return Active.DeliveredAmounts[RequirementIndex];
    }

    // Ancienne sauvegarde x1.
    return RequirementIndex == 0
        ? Active.DeliveredAmount
        : 0;
}

bool UContractoryMissionManager::IsMissionComplete(
    const FContractoryActiveMission& Active,
    const FContractoryMissionDefinition& Definition
)
{
    if (Definition.Requirements.IsEmpty())
    {
        return false;
    }

    for (int32 i = 0; i < Definition.Requirements.Num(); ++i)
    {
        if (
            GetDeliveredForRequirement(Active, i) <
            Definition.Requirements[i].TargetAmount
        )
        {
            return false;
        }
    }

    return true;
}

int32 UContractoryMissionManager::RecordVerifiedDeliveryForItem(
    FContractoryMissionSession& Session,
    int32 MissionId,
    const FString& ItemKey,
    int32 Quantity
)
{
    if (Quantity <= 0 || ItemKey.IsEmpty())
    {
        return 0;
    }

    const int32 ActiveIndex =
        FindActiveIndex(Session, MissionId);

    if (ActiveIndex == INDEX_NONE)
    {
        return 0;
    }

    FContractoryMissionDefinition Definition;

    if (
        !FindDefinition(MissionId, Definition) ||
        Definition.Requirements.IsEmpty()
    )
    {
        return 0;
    }

    FContractoryActiveMission& Active =
        Session.ActiveMissions[ActiveIndex];

    EnsureProgressShape(Active, Definition);

    int32 AcceptedTotal = 0;
    int32 RemainingInput = Quantity;

    for (
        int32 RequirementIndex = 0;
        RequirementIndex < Definition.Requirements.Num() &&
        RemainingInput > 0;
        ++RequirementIndex
    )
    {
        const FContractoryMissionRequirement& Requirement =
            Definition.Requirements[RequirementIndex];

        if (Requirement.ItemKey != ItemKey)
        {
            continue;
        }

        const int32 AlreadyDelivered =
            Active.DeliveredAmounts[RequirementIndex];

        const int32 RemainingNeed =
            FMath::Max(
                0,
                Requirement.TargetAmount - AlreadyDelivered
            );

        if (RemainingNeed <= 0)
        {
            continue;
        }

        const int32 Accepted =
            FMath::Min(RemainingNeed, RemainingInput);

        Active.DeliveredAmounts[RequirementIndex] += Accepted;
        AcceptedTotal += Accepted;
        RemainingInput -= Accepted;
    }

    // Ancien champ = progression de la premiere ligne.
    Active.DeliveredAmount =
        Active.DeliveredAmounts.IsValidIndex(0)
            ? Active.DeliveredAmounts[0]
            : 0;

    return AcceptedTotal;
}

bool UContractoryMissionManager::RecordVerifiedDelivery(
    FContractoryMissionSession& Session,
    int32 MissionId,
    int32 Quantity
)
{
    FContractoryMissionDefinition Definition;

    if (
        !FindDefinition(MissionId, Definition) ||
        Definition.Requirements.IsEmpty()
    )
    {
        return false;
    }

    return RecordVerifiedDeliveryForItem(
        Session,
        MissionId,
        Definition.Requirements[0].ItemKey,
        Quantity
    ) > 0;
}

bool UContractoryMissionManager::ClaimCompletedMission(
    FContractoryMissionSession& Session,
    int32 MissionId,
    int32& CreditsAwarded
)
{
    CreditsAwarded = 0;

    const int32 Index =
        FindActiveIndex(Session, MissionId);

    if (Index == INDEX_NONE)
    {
        return false;
    }

    FContractoryMissionDefinition Definition;

    if (
        !FindDefinition(MissionId, Definition) ||
        !IsMissionComplete(Session.ActiveMissions[Index], Definition) ||
        Definition.RewardCredits <= 0 ||
        Session.Credits > MAX_int32 - Definition.RewardCredits
    )
    {
        return false;
    }

    Session.Credits += Definition.RewardCredits;
    CreditsAwarded = Definition.RewardCredits;

    Session.ActiveMissions.RemoveAt(Index);
    Session.CompletedMissionIds.AddUnique(MissionId);

    return true;
}