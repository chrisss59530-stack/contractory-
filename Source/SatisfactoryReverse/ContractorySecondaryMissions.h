#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContractorySecondaryMissions.generated.h"

USTRUCT(BlueprintType)
struct FContractoryMissionRequirement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    FString ItemKey;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    int32 TargetAmount = 0;
};

// Catalogue independant : aucune reference aux 21 contrats HUB.
USTRUCT(BlueprintType)
struct FContractoryMissionDefinition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    int32 MissionId = 0;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    int32 MinimumTier = 0;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    FString Category;

    // Compatibilite avec l'ancien format : premiere ressource de la mission.
    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    FString TargetItemKey;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    int32 TargetAmount = 0;

    // Nouveau format : de 1 a 5 ressources par mission.
    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    TArray<FContractoryMissionRequirement> Requirements;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    int32 RewardCredits = 0;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Missions")
    FText Title;
};

UCLASS()
class SATISFACTORYREVERSE_API UContractorySecondaryMissions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Contractory|Missions secondaires")
    static TArray<FContractoryMissionDefinition> GetMissionCatalogue();

    UFUNCTION(BlueprintPure, Category="Contractory|Missions secondaires")
    static TArray<FContractoryMissionDefinition> GetAvailableMissions(int32 UnlockedTier);
};