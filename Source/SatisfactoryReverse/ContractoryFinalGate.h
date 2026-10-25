#pragma once

#include "CoreMinimal.h"
#include "FGSchematic.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "ContractoryFinalGate.generated.h"

class UUserWidget;


/*
 * Marqueur persistant :
 * le joueur a traversé le portail final Contractory.
 */
UCLASS()
class SATISFACTORYREVERSE_API UContractoryFinalPortalTraversedSchematic
    : public UFGSchematic
{
    GENERATED_BODY()

public:
    UContractoryFinalPortalTraversedSchematic();
};


/*
 * Fonctions utilisées par le verrou de la Phase 5.
 */
UCLASS()
class SATISFACTORYREVERSE_API UContractoryFinalGateLibrary
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(
        BlueprintCallable,
        Category="Contractory|Final",
        meta=(
            WorldContext="WorldContextObject",
            DisplayName="Marquer Portail Contractory Traverse"
        )
    )
    static bool MarkFinalPortalTraversed(
        const UObject* WorldContextObject
    );

    UFUNCTION(
        BlueprintPure,
        Category="Contractory|Final",
        meta=(
            WorldContext="WorldContextObject",
            DisplayName="Portail Contractory Deja Traverse"
        )
    )
    static bool IsFinalPortalTraversed(
        const UObject* WorldContextObject
    );
};


/*
 * Surveille uniquement l'écran final Contractory.
 *
 * Quand WBP_Final_Contractory apparaît :
 *   - la traversée est enregistrée ;
 *   - l'écran reste visible quelques secondes ;
 *   - le widget est fermé automatiquement ;
 *   - les contrôles sont rendus au joueur.
 */
UCLASS()
class SATISFACTORYREVERSE_API UContractoryFinalSequenceSubsystem
    : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:

    virtual void Tick(float DeltaTime) override;

    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(
            UContractoryFinalSequenceSubsystem,
            STATGROUP_Tickables
        );
    }

    virtual bool IsTickableInEditor() const override
    {
        return false;
    }

    virtual bool IsTickableWhenPaused() const override
    {
        return true;
    }

private:

    TWeakObjectPtr<UUserWidget> ActiveFinalWidget;

    double FinalWidgetSeenAt = -1.0;

    bool bTraversalMarked = false;
};
