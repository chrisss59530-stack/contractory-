#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "ContractoryDeliveryConveyorStub.generated.h"

class UFGFactoryConnectionComponent;

/**
 * Petit convoyeur interne de la Livraison Contractory.
 *
 * IMPORTANT :
 * - derive du convoyeur Satisfactory ;
 * - ne modifie jamais une classe/instance vanilla ;
 * - adapte uniquement SA propre vitesse au convoyeur joueur connecte.
 */
UCLASS()
class SATISFACTORYREVERSE_API AContractoryDeliveryConveyorStub
    : public AFGBuildableConveyorBelt
{
    GENERATED_BODY()

public:
    AContractoryDeliveryConveyorStub();

    virtual void Tick(float DeltaSeconds) override;

    /** Recopie la vitesse du convoyeur branche sur l'entree du stub. */
    UFUNCTION(BlueprintCallable, Category="Contractory|Livraison")
    void SyncSpeedFromConnectedConveyor();

    UFUNCTION(BlueprintPure, Category="Contractory|Livraison")
    float GetContractoryStubSpeed() const
    {
        return GetSpeed();
    }

protected:
    virtual void BeginPlay() override;

private:
    float LastDetectedExternalSpeed = -1.0f;
};
