#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContractoryBoutiqueCatalog.generated.h"

USTRUCT(BlueprintType)
struct FContractoryBoutiqueEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Boutique")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Boutique")
    FString SchematicClassPath;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Boutique")
    int32 PriceCredits = 0;

    UPROPERTY(BlueprintReadOnly, Category="Contractory|Boutique")
    int32 MinimumTier = 0;
};

UCLASS()
class SATISFACTORYREVERSE_API UContractoryBoutiqueCatalog : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Contractory|Boutique")
    static TArray<FContractoryBoutiqueEntry> GetCatalogue();

    UFUNCTION(BlueprintPure, Category="Contractory|Boutique")
    static bool FindBySchematicClassPath(
        const FString& SchematicClassPath,
        FContractoryBoutiqueEntry& OutEntry
    );

    UFUNCTION(BlueprintPure, Category="Contractory|Boutique")
    static int32 GetTotalCatalogueCost();
};

