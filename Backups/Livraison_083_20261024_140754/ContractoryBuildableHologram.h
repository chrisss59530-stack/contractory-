#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGGenericBuildableHologram.h"
#include "ContractoryBuildableHologram.generated.h"

class UChildActorComponent;

UCLASS()
class SATISFACTORYREVERSE_API AContractoryBuildableHologram
    : public AFGGenericBuildableHologram
{
    GENERATED_BODY()

public:
    AContractoryBuildableHologram();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UChildActorComponent> PreviewComponent;
};
