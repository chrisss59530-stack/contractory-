#include "ContractorySecondaryMissions.h"
#include <initializer_list>

namespace
{
    FContractoryMissionRequirement Req(int32 Tier, int32 Amount)
    {
        FContractoryMissionRequirement R;
        R.ItemKey = FString::Printf(
            TEXT("Desc_Recyclage_FICSIT_Tier%d"),
            Tier
        );
        R.TargetAmount = Amount;
        return R;
    }
}

TArray<FContractoryMissionDefinition>
UContractorySecondaryMissions::GetMissionCatalogue()
{
    TArray<FContractoryMissionDefinition> Result;
    Result.Reserve(14);

    auto Add =
        [&Result](
            int32 Id,
            int32 MinimumTier,
            int32 Credits,
            const TCHAR* Name,
            std::initializer_list<FContractoryMissionRequirement> Requirements
        )
    {
        FContractoryMissionDefinition Mission;
        Mission.MissionId = Id;
        Mission.MinimumTier = MinimumTier;
        Mission.Category = TEXT("Recyclage");
        Mission.RewardCredits = Credits;
        Mission.Title = FText::FromString(Name);

        for (const FContractoryMissionRequirement& R : Requirements)
        {
            Mission.Requirements.Add(R);
        }

        if (!Mission.Requirements.IsEmpty())
        {
            Mission.TargetItemKey = Mission.Requirements[0].ItemKey;
            Mission.TargetAmount = Mission.Requirements[0].TargetAmount;
        }

        Result.Add(MoveTemp(Mission));
    };

    // Alpha 0.8.2 : deux missions par palier de recyclage.
    // MinimumTier est un index 0-based : T1=0, T2=1, etc.
    Add( 1, 0,  30, TEXT("Recyclage T1 - Mission A"), { Req(1, 100) });
    Add( 2, 0,  85, TEXT("Recyclage T1 - Mission B"), { Req(1, 250) });
    Add( 3, 1,  50, TEXT("Recyclage T2 - Mission A"), { Req(2, 100) });
    Add( 4, 1, 140, TEXT("Recyclage T2 - Mission B"), { Req(2, 250) });
    Add( 5, 2,  80, TEXT("Recyclage T3 - Mission A"), { Req(3, 100) });
    Add( 6, 2, 220, TEXT("Recyclage T3 - Mission B"), { Req(3, 250) });
    Add( 7, 3, 120, TEXT("Recyclage T4 - Mission A"), { Req(4, 100) });
    Add( 8, 3, 330, TEXT("Recyclage T4 - Mission B"), { Req(4, 250) });
    Add( 9, 4, 175, TEXT("Recyclage T5 - Mission A"), { Req(5, 100) });
    Add(10, 4, 480, TEXT("Recyclage T5 - Mission B"), { Req(5, 250) });
    Add(11, 5, 250, TEXT("Recyclage T6 - Mission A"), { Req(6, 100) });
    Add(12, 5, 680, TEXT("Recyclage T6 - Mission B"), { Req(6, 250) });
    Add(13, 6, 350, TEXT("Recyclage T7 - Mission A"), { Req(7, 100) });
    Add(14, 6, 950, TEXT("Recyclage T7 - Mission B"), { Req(7, 250) });

    return Result;
}

TArray<FContractoryMissionDefinition>
UContractorySecondaryMissions::GetAvailableMissions(int32 UnlockedTier)
{
    TArray<FContractoryMissionDefinition> Available;

    for (const FContractoryMissionDefinition& Mission : GetMissionCatalogue())
    {
        if (Mission.MinimumTier <= UnlockedTier)
        {
            Available.Add(Mission);
        }
    }

    return Available;
}