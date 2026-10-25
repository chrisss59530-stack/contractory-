#pragma once

#include "CoreMinimal.h"
#include "ContractoryPoweredBuildable.h"
#include "ContractoryDeliveryBuildable.generated.h"

class USceneComponent;
class UFGInventoryComponent;
class UFGFactoryConnectionComponent;
class AContractoryMissionSaveManager;
class AFGCharacterPlayer;

UCLASS()
class SATISFACTORYREVERSE_API AContractoryDeliveryBuildable
    : public AContractoryPoweredBuildable
{
    GENERATED_BODY()

public:
    AContractoryDeliveryBuildable();

    virtual bool IsUseable_Implementation() const override;

    virtual void OnUse_Implementation(
        class AFGCharacterPlayer* byCharacter,
        const FUseState& state
    ) override;

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category="Contractory|Livraison")
    UFGInventoryComponent* GetDeliveryInventory() const
    {
        return DeliveryInventory;
    }

    UFUNCTION(BlueprintPure, Category="Contractory|Livraison")
    UFGFactoryConnectionComponent* GetDeliveryInputConnection() const
    {
        return DeliveryInputConnection;
    }

protected:
    virtual void BeginPlay() override;

private:
    bool IsDeliveryItemAllowed(
        TSubclassOf<UObject> ItemClass,
        int32 Index
    );

    void ProcessDeliveryInventory();

    void WireEmbeddedConveyorStub();

    AContractoryMissionSaveManager*
    ResolveMissionSaveManager();

protected:

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Contractory|Livraison"
    )
    TObjectPtr<UFGInventoryComponent> DeliveryInventory;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Contractory|Livraison"
    )
    TObjectPtr<USceneComponent> DeliveryInputAnchor;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="Contractory|Livraison"
    )
    TObjectPtr<UFGFactoryConnectionComponent> DeliveryInputConnection;

private:

    UPROPERTY(Transient)
    TObjectPtr<AContractoryMissionSaveManager> MissionSaveManager;
};

