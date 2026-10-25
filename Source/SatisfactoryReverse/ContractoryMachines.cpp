#include "CoreMinimal.h"
#include "FGSchematic.h"
#include "Registry/ModContentRegistry.h"

// Enregistre les trois recettes via leur schematic commun.
// A appeler uniquement lorsque le monde de jeu est disponible.
void ContractoryRegisterMachines(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return;
    }

    UModContentRegistry* Registry =
        UModContentRegistry::Get(WorldContext);

    if (!Registry)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[Contractory] Registre indisponible"));
        return;
    }

    const TCHAR* Path =
        TEXT("/SatisfactoryReverse/Schematics/"
             "Schematic_Contractory_Machines_Tier2."
             "Schematic_Contractory_Machines_Tier2_C");

    TSubclassOf<UFGSchematic> Machines =
        LoadClass<UFGSchematic>(nullptr, Path);

    if (!Machines)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[Contractory] Schematic machines introuvable"));
        return;
    }

    Registry->RegisterSchematic(
        FName(TEXT("SatisfactoryReverse")),
        Machines
    );

    UE_LOG(LogTemp, Log,
        TEXT("[Contractory] Schematic des 3 machines enregistre"));
}
