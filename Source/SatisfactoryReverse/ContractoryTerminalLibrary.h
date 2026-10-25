#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContractoryTerminalLibrary.generated.h"

USTRUCT(BlueprintType)
struct FContractoryContractStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Contractory|Terminal")
    int32 ContractId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Contractory|Terminal")
    bool bSchematicFound = false;

    UPROPERTY(BlueprintReadOnly, Category = "Contractory|Terminal")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Contractory|Terminal")
    TSubclassOf<class UFGSchematic> Schematic;
};

UCLASS()
class SATISFACTORYREVERSE_API UContractoryTerminalLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Lecture seule : aucun changement des jalons, contrats ou recompenses existants.
    UFUNCTION(BlueprintCallable, Category = "Contractory|Terminal", meta = (WorldContext = "WorldContextObject"))
    static TArray<FContractoryContractStatus> GetMandatoryContracts(const UObject* WorldContextObject);
};
