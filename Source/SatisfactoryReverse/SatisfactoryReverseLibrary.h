#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SatisfactoryReverseLibrary.generated.h"

UCLASS()
class SATISFACTORYREVERSE_API USatisfactoryReverseLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Satisfactory Reverse")
    static bool ReverseIronPlateRecipe();

    UFUNCTION(BlueprintCallable, Category = "Satisfactory Reverse", meta = (WorldContext = "WorldContextObject"))
    static int32 CountRegisteredRecipes(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Satisfactory Reverse", meta = (WorldContext = "WorldContextObject"))
    static int32 CountManufacturingRecipes(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Satisfactory Reverse", meta = (WorldContext = "WorldContextObject"))
    static int32 ReverseManufacturingRecipes(const UObject* WorldContextObject);
    UFUNCTION(BlueprintCallable, Category = "Satisfactory Reverse", meta = (WorldContext = "WorldContextObject"))
    static bool ActiverHardcoreTier2(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Satisfactory Reverse|FICSIT")
    static bool DonnerCertificatFICSIT(
        TSubclassOf<class UFGSchematic> Schematic,
        class AFGCharacterPlayer* Joueur);
};






