#include "ContractoryBuildingDescriptor.h"

FText UContractoryBuildingDescriptor::GetItemNameInternal() const
{
    const FString ClassName = GetClass()->GetName();

    if (ClassName.Contains(TEXT("Terminal")))
        return FText::FromString(TEXT("Terminal Contractory"));

    if (ClassName.Contains(TEXT("Boutique")))
        return FText::FromString(TEXT("Boutique Contractory"));

    if (ClassName.Contains(TEXT("Livraison")))
        return FText::FromString(TEXT("Livraison Contractory"));

    return FText::FromString(TEXT("Machine Contractory"));
}

FText UContractoryBuildingDescriptor::GetItemDescriptionInternal() const
{
    const FString ClassName = GetClass()->GetName();

    if (ClassName.Contains(TEXT("Terminal")))
    {
        return FText::FromString(
            TEXT("Terminal principal des missions secondaires Contractory. Consulte les offres, accepte tes missions et suis tes crédits.")
        );
    }

    if (ClassName.Contains(TEXT("Boutique")))
    {
        return FText::FromString(
            TEXT("Échange tes crédits Contractory contre les récompenses disponibles.")
        );
    }

    if (ClassName.Contains(TEXT("Livraison")))
    {
        return FText::FromString(
            TEXT("Dépose ici les ressources demandées par tes missions Contractory.")
        );
    }

    return FText::FromString(TEXT("Machine du système Contractory."));
}
