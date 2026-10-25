#pragma once

#include "CoreMinimal.h"
#include "Resources/FGBuildingDescriptor.h"
#include "ContractoryBuildingDescriptor.generated.h"

UCLASS()
class SATISFACTORYREVERSE_API UContractoryBuildingDescriptor
    : public UFGBuildingDescriptor
{
    GENERATED_BODY()

protected:
    virtual FText GetItemNameInternal() const override;
    virtual FText GetItemDescriptionInternal() const override;
};
