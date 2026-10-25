#include "ContractoryTerminalLibrary.h"

#include "Engine/World.h"
#include "FGSchematic.h"
#include "FGSchematicManager.h"

TArray<FContractoryContractStatus> UContractoryTerminalLibrary::GetMandatoryContracts(
    const UObject* WorldContextObject)
{
    TArray<FContractoryContractStatus> Result;

    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World || !World->IsGameWorld())
    {
        return Result;
    }

    AFGSchematicManager* Manager = AFGSchematicManager::Get(World);
    if (!Manager)
    {
        return Result;
    }

    Result.Reserve(21);
    for (int32 Id = 1; Id <= 21; ++Id)
    {
        FContractoryContractStatus Status;
        Status.ContractId = Id;

        const FString Path = FString::Printf(
            TEXT("/SatisfactoryReverse/Schematics/Schematic_FICSIT_Contract_%03d.Schematic_FICSIT_Contract_%03d_C"),
            Id, Id);

        Status.Schematic = LoadClass<UFGSchematic>(nullptr, *Path);
        Status.bSchematicFound = (Status.Schematic != nullptr);
        Status.bCompleted = Status.bSchematicFound && Manager->IsSchematicPurchased(Status.Schematic);

        Result.Add(Status);
    }

    return Result;
}
