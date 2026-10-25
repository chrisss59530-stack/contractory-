#include "ContractoryBuildableHologram.h"

#include "Components/ChildActorComponent.h"
#include "GameFramework/Actor.h"

AContractoryBuildableHologram::AContractoryBuildableHologram()
{
    PreviewComponent =
        CreateDefaultSubobject<UChildActorComponent>(
            TEXT("ContractoryPreview")
        );

    PreviewComponent->SetupAttachment(RootComponent);
    PreviewComponent->SetMobility(EComponentMobility::Movable);
    PreviewComponent->SetVisibility(true, true);
}

void AContractoryBuildableHologram::BeginPlay()
{
    Super::BeginPlay();

    TSubclassOf<AActor> BuildClass = GetBuildClass();

    if (!BuildClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Contractory] Hologram: BuildClass is NULL.")
        );
        return;
    }

    const FString BuildName = BuildClass->GetName();
    const TCHAR* PreviewPath = nullptr;

    if (BuildName.Contains(TEXT("Terminal")))
    {
        PreviewPath =
            TEXT("/SatisfactoryReverse/building/Blueprints/"
                 "BP_Contractory_Terminal_3D."
                 "BP_Contractory_Terminal_3D_C");
    }
    else if (BuildName.Contains(TEXT("Boutique")))
    {
        PreviewPath =
            TEXT("/SatisfactoryReverse/building/Blueprints/"
                 "BP_Contractory_Boutique_3D."
                 "BP_Contractory_Boutique_3D_C");
    }
    else if (BuildName.Contains(TEXT("Livraison")))
    {
        PreviewPath =
            TEXT("/SatisfactoryReverse/building/Blueprints/"
                 "BP_Contractory_Livraison_3D."
                 "BP_Contractory_Livraison_3D_C");
    }

    if (!PreviewPath)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Contractory] Hologram: unknown BuildClass %s"),
            *BuildName
        );
        return;
    }

    UClass* PreviewClass =
        LoadClass<AActor>(nullptr, PreviewPath);

    if (!PreviewClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Contractory] Hologram: unable to load preview for %s"),
            *BuildName
        );
        return;
    }

    PreviewComponent->SetChildActorClass(PreviewClass);
    PreviewComponent->SetVisibility(true, true);

    if (AActor* PreviewActor = PreviewComponent->GetChildActor())
    {
        PreviewActor->SetActorEnableCollision(false);
        PreviewActor->SetActorHiddenInGame(false);
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Contractory] Hologram preview created for %s -> %s"),
        *BuildName,
        *PreviewClass->GetName()
    );
}
