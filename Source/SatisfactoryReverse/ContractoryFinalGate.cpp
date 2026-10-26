#include "ContractoryFinalGate.h"

#include "FGSchematicManager.h"
#include "FGPlayerController.h"
#include "ContractoryMissionRemoteCallObject.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"


// ============================================================
// MARQUEUR PERSISTANT
// ============================================================

UContractoryFinalPortalTraversedSchematic::
UContractoryFinalPortalTraversedSchematic()
{
    mType = ESchematicType::EST_Custom;

    mDisplayName =
        FText::FromString(
            TEXT("Contractory Final Portal Traversed")
        );

    mDescription =
        FText::FromString(
            TEXT("Internal Contractory progression marker")
        );

    mTechTier = 9;

    mCost.Reset();
    mUnlocks.Reset();

    mTimeToComplete = 0.0f;
    mIsPlayerSpecific = false;
    mDependenciesBlocksSchematicAccess = false;
    mHiddenUntilDependenciesMet = true;
}


// ============================================================
// VERIFICATION DU MARQUEUR
// ============================================================

bool UContractoryFinalGateLibrary::IsFinalPortalTraversed(
    const UObject* WorldContextObject
)
{
    if (!WorldContextObject)
        return false;

    UWorld* World = WorldContextObject->GetWorld();

    if (!World)
        return false;

    AFGSchematicManager* Manager =
        AFGSchematicManager::Get(World);

    if (!Manager)
        return false;

    return Manager->IsSchematicPurchased(
        UContractoryFinalPortalTraversedSchematic::StaticClass()
    );
}


// ============================================================
// ENREGISTRER LA TRAVERSEE
// ============================================================

bool UContractoryFinalGateLibrary::MarkFinalPortalTraversed(
    const UObject* WorldContextObject
)
{
    if (!WorldContextObject)
        return false;

    UWorld* World = WorldContextObject->GetWorld();

    if (!World)
        return false;

    AFGSchematicManager* Manager =
        AFGSchematicManager::Get(World);

    if (!Manager || !Manager->HasAuthority())
        return false;

    if (!Manager->IsSchematicPurchased(
        UContractoryFinalPortalTraversedSchematic::StaticClass()
    ))
    {
        const ESchematicUnlockFlags Flags =
            ESchematicUnlockFlags::Force |
            ESchematicUnlockFlags::BlockTelemetry |
            ESchematicUnlockFlags::SuppressNarrativeMessages;

        Manager->GiveAccessToSchematic(
            UContractoryFinalPortalTraversedSchematic::StaticClass(),
            nullptr,
            Flags
        );
    }

    const bool bSuccess =
        Manager->IsSchematicPurchased(
            UContractoryFinalPortalTraversedSchematic::StaticClass()
        );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[CONTRACTORY FINAL] "
            "Portail final traverse : %s"
        ),
        bSuccess ? TEXT("OUI") : TEXT("NON")
    );

    return bSuccess;
}


// ============================================================
// DETECTION DE L'ECRAN FINAL
// ============================================================

static bool Contractory_IsFinalWidget(
    UUserWidget* Widget,
    UWorld* World
)
{
    if (!Widget || !World)
        return false;

    if (Widget->IsTemplate())
        return false;

    if (Widget->GetWorld() != World)
        return false;

    if (!Widget->IsInViewport())
        return false;

    const FString ClassPath =
        Widget->GetClass()->GetPathName();

    return ClassPath ==
        TEXT(
            "/SatisfactoryReverse/UI/"
            "WBP_Final_Contractory."
            "WBP_Final_Contractory_C"
        );
}


// ============================================================
// SURVEILLANCE DE LA SEQUENCE FINALE
// ============================================================

void UContractoryFinalSequenceSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();

    if (!World || World->GetNetMode() == NM_DedicatedServer)
        return;

    UUserWidget* Widget = ActiveFinalWidget.Get();
    const bool bWidgetVisible = Contractory_IsFinalWidget(Widget, World);

    // Detecter une nouvelle sequence finale.
    if (FinalWidgetSeenAt < 0.0)
    {
        Widget = nullptr;

        for (TObjectIterator<UUserWidget> It; It; ++It)
        {
            UUserWidget* Candidate = *It;

            if (!Contractory_IsFinalWidget(Candidate, World))
                continue;

            ActiveFinalWidget = Candidate;
            Widget = Candidate;
            FinalWidgetSeenAt = FPlatformTime::Seconds();
            bTraversalMarked = false;

            UE_LOG(LogTemp, Warning,
                TEXT("[CONTRACTORY FINAL] Ecran final detecte - compteur demarre"));

            break;
        }
    }

    // Aucune sequence en cours.
    if (FinalWidgetSeenAt < 0.0)
        return;

    // Le compteur continue meme si le widget disparait.
    constexpr double FinalScreenDuration = 15.0;

    const double Elapsed =
        FPlatformTime::Seconds() - FinalWidgetSeenAt;

    if (Elapsed < FinalScreenDuration)
        return;

    if (bTraversalMarked)
        return;

    APlayerController* PC = nullptr;

    if (Widget)
        PC = Widget->GetOwningPlayer();

    if (!PC)
        PC = UGameplayStatics::GetPlayerController(World, 0);

    AFGPlayerController* FGPC = Cast<AFGPlayerController>(PC);

    if (!FGPC)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CONTRACTORY FINAL] Controleur indisponible - nouvelle tentative"));
        return;
    }

    UContractoryMissionRemoteCallObject* RCO =
        Cast<UContractoryMissionRemoteCallObject>(
            FGPC->GetRemoteCallObjectOfClass(
                UContractoryMissionRemoteCallObject::StaticClass()
            )
        );

    if (!RCO)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CONTRACTORY FINAL] RCO indisponible - nouvelle tentative"));
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[CONTRACTORY FINAL] DELAI 15 SECONDES ATTEINT"));

    RCO->Server_MarkFinalPortalTraversed();
    bTraversalMarked = true;

    UE_LOG(LogTemp, Warning,
        TEXT("[CONTRACTORY FINAL] Demande de validation envoyee au serveur"));

    if (Contractory_IsFinalWidget(Widget, World))
        Widget->RemoveFromParent();

    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
        PC->ResetIgnoreMoveInput();
        PC->ResetIgnoreLookInput();
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[CONTRACTORY FINAL] Sequence terminee - controles rendus"));

    ActiveFinalWidget.Reset();
    FinalWidgetSeenAt = -1.0;
}
