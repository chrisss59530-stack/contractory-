#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "ContractoryInteractiveBuildable.generated.h"

class UBoxComponent;

UCLASS()
class SATISFACTORYREVERSE_API AContractoryInteractiveBuildable : public AFGBuildable
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UBoxComponent> InteractionBox;
};
