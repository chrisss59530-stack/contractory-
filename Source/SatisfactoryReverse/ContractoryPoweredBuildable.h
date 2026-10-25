#pragma once

#include "CoreMinimal.h"
#include "ContractoryInteractiveBuildable.h"
#include "ContractoryPoweredBuildable.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UFGPowerConnectionComponent;
class UFGPowerInfoComponent;

UCLASS()
class SATISFACTORYREVERSE_API AContractoryPoweredBuildable
    : public AContractoryInteractiveBuildable
{
    GENERATED_BODY()

public:
    AContractoryPoweredBuildable();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category="Contractory|Power")
    bool HasContractoryPower() const;

    UFUNCTION(BlueprintPure, Category="Contractory|Power")
    UFGPowerConnectionComponent* GetContractoryPowerConnection() const
    {
        return PowerConnection;
    }

protected:
    virtual void BeginPlay() override;

    void CacheAnimationComponents();
    void UpdatePoweredVisuals(float DeltaSeconds, bool bPowered);
    void SetMachineLightsVisible(bool bVisible);

protected:

    // On déplace CE composant dans le Blueprint.
    // Le boîtier + la vraie connexion électrique suivront ensemble.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Contractory|Power")
    TObjectPtr<USceneComponent> PowerAnchor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Contractory|Power")
    TObjectPtr<UStaticMeshComponent> PowerSocketMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Contractory|Power")
    TObjectPtr<UFGPowerConnectionComponent> PowerConnection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Contractory|Power")
    TObjectPtr<UFGPowerInfoComponent> PowerInfo;

private:

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> TerminalScreenLines;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> DeliveryRollers;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USceneComponent>> DeliveryRollerPivots;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> DeliveryScannerBeams;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> DeliveryLights;

    bool bAnimationComponentsCached = false;
    bool bPreviousPoweredState = false;

    float AnimationClock = 0.0f;
};

