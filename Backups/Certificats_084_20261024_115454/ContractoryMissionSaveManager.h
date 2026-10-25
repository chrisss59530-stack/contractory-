#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGSaveInterface.h"
#include "ContractoryMissionManager.h"
#include "ContractoryMissionSaveManager.generated.h"

// Structure conservee pour compatibilite avec le code existant.
// En mode partage, PlayerKey vaut CONTRACTORY_SHARED.
USTRUCT(BlueprintType)
struct FContractoryPlayerMissionState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, VisibleAnywhere, Category="Contractory|Missions")
    FString PlayerKey;

    UPROPERTY(SaveGame, VisibleAnywhere, Category="Contractory|Missions")
    FContractoryMissionSession Session;

    UPROPERTY(SaveGame, VisibleAnywhere, Category="Contractory|Missions")
    int32 RotationIndex = 0;
};

UCLASS(NotPlaceable)
class SATISFACTORYREVERSE_API AContractoryMissionSaveManager
    : public AActor,
      public IFGSaveInterface
{
    GENERATED_BODY()

public:
    AContractoryMissionSaveManager();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    virtual void PreSaveGame_Implementation(
        int32 SaveVersion,
        int32 GameVersion
    ) override;

    virtual void PostSaveGame_Implementation(
        int32 SaveVersion,
        int32 GameVersion
    ) override;

    virtual void PreLoadGame_Implementation(
        int32 SaveVersion,
        int32 GameVersion
    ) override;

    virtual void PostLoadGame_Implementation(
        int32 SaveVersion,
        int32 GameVersion
    ) override;

    virtual void GatherDependencies_Implementation(
        TArray<UObject*>& OutDependentObjects
    ) override;

    virtual bool ShouldSave_Implementation() const override;
    virtual bool NeedTransform_Implementation() override;

    // Compatibilite avec le backend existant.
    // TrustedPlayerKey est volontairement ignore :
    // tous les joueurs utilisent le meme etat Contractory.
    FContractoryPlayerMissionState* FindOrAddServerPlayer(
        const FString& TrustedPlayerKey
    );

    const FContractoryPlayerMissionState* FindPlayer(
        const FString& PlayerKey
    ) const;

private:

    // Etat unique partage entre tous les joueurs de la partie.
    UPROPERTY(SaveGame, Replicated)
    FContractoryPlayerMissionState SharedState;

    // Ancien stockage par joueur.
    // Conserve temporairement pour migrer les sauvegardes existantes.
    UPROPERTY(SaveGame)
    TArray<FContractoryPlayerMissionState> PlayerStates;
};
