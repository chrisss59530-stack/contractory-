#include "ContractoryMissionSubsystem.h"



#include "Resources/FGItemDescriptor.h"

#include "FGInventoryComponent.h"

#include "FGCharacterPlayer.h"

#include "ContractoryMissionSaveManager.h"

#include "ContractoryBoutiqueCatalog.h"

#include "FGSchematic.h"



#include "FGRecipeManager.h"

#include "FGSchematicManager.h"

#include "FGRecipe.h"

#include "FGPlayerState.h"

#include "ContractoryMissionManager.h"



#include "EngineUtils.h"

#include "Engine/World.h"

#include "TimerManager.h"

#include "UObject/UObjectGlobals.h"



AContractoryMissionSubsystem::AContractoryMissionSubsystem()

{

    ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;

}



void AContractoryMissionSubsystem::Init()

{

    Super::Init();



    UWorld* World = GetWorld();

    if (!World || !HasAuthority())

    {

        return;

    }



    // Recherche d'un manager dÃƒÆ’Ã‚Â©jÃƒÆ’Ã‚Â  prÃƒÆ’Ã‚Â©sent dans la sauvegarde.

    for (TActorIterator<AContractoryMissionSaveManager> It(World); It; ++It)

    {

        MissionSaveManager = *It;

        UE_LOG(

            LogTemp,

            Log,

            TEXT("[Contractory] Existing mission save manager found.")

        );

        break;

    }



    // CrÃƒÆ’Ã‚Â©ation uniquement s'il n'existe pas dÃƒÆ’Ã‚Â©jÃƒÆ’Ã‚Â .

    if (!MissionSaveManager)

    {

        FActorSpawnParameters SpawnParams;

        SpawnParams.Name = FName(TEXT("ContractoryMissionSaveManager"));

        SpawnParams.SpawnCollisionHandlingOverride =

            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;



        MissionSaveManager = World->SpawnActor<AContractoryMissionSaveManager>(

            AContractoryMissionSaveManager::StaticClass(),

            FTransform::Identity,

            SpawnParams

        );



        if (MissionSaveManager)

        {

            UE_LOG(

                LogTemp,

                Log,

                TEXT("[Contractory] Mission save manager created.")

            );

        }

        else

        {

            UE_LOG(

                LogTemp,

                Error,

                TEXT("[Contractory] Mission save manager creation failed.")

            );

        }

    }



    // VÃƒÆ’Ã‚Â©rification pÃƒÆ’Ã‚Â©riodique :

    // - permet aux anciennes sauvegardes Tier 2+ de rÃƒÆ’Ã‚Â©cupÃƒÆ’Ã‚Â©rer les machines ;

    // - permet aussi ÃƒÆ’Ã‚Â  une nouvelle partie de les recevoir lorsqu'elle atteint Tier 2.

    World->GetTimerManager().SetTimer(

        MachineUnlockRetryTimer,

        this,

        &AContractoryMissionSubsystem::TryUnlockContractoryMachines,

        5.0f,

        true,

        2.0f

    );



    // Premier essai immÃƒÆ’Ã‚Â©diat.

    TryUnlockContractoryMachines();

}



bool AContractoryMissionSubsystem::AcceptMissionForPlayer(

    AFGPlayerState* PlayerState,

    int32 MissionId

)

{

    if (!HasAuthority() ||

        !PlayerState ||

        !MissionSaveManager)

    {

        return false;

    }



    const FString PlayerKey =

        TEXT("CONTRACTORY_SHARED");



    if (PlayerKey.IsEmpty())

    {

        UE_LOG(

            LogTemp,

            Warning,

            TEXT("[Contractory] Mission refused: invalid player user id.")

        );



        return false;

    }



    FContractoryPlayerMissionState* PlayerMissionState =

        MissionSaveManager->FindOrAddServerPlayer(PlayerKey);



    if (!PlayerMissionState)

    {

        return false;

    }



    AFGSchematicManager* SchematicManager =

        AFGSchematicManager::Get(GetWorld());



    if (!SchematicManager)

    {

        return false;

    }



    const int32 HighestTier =

        SchematicManager->GetHighestAvailableTechTier();



    const TArray<FContractoryMissionDefinition> CurrentOffers =

        UContractoryMissionManager::GetFiveOffers(

            HighestTier,

            PlayerMissionState->RotationIndex

        );



    const bool bIsCurrentOffer =

        CurrentOffers.ContainsByPredicate(

            [MissionId](const FContractoryMissionDefinition& Offer)

            {

                return Offer.MissionId == MissionId;

            }

        );



    if (!bIsCurrentOffer)

    {

        UE_LOG(

            LogTemp,

            Warning,

            TEXT(

                "[Contractory] Mission %d refused: not in current offers."

            ),

            MissionId

        );



        return false;

    }



    // CONTRACTORY_SERVER_LIMIT_100
    if (UContractoryMissionManager::GetMissionCompletionCount(
            PlayerMissionState->Session, MissionId
        ) >= UContractoryMissionManager::GetMissionCompletionLimit(MissionId))
    {
        return false;
    }

    const bool bAccepted =

        UContractoryMissionManager::AcceptMission(

            PlayerMissionState->Session,

            MissionId,

            HighestTier

        );



    if (bAccepted)

    {

        MissionSaveManager->ForceNetUpdate();



        UE_LOG(

            LogTemp,

            Log,

            TEXT(

                "[Contractory] Mission %d accepted for trusted player."

            ),

            MissionId

        );

    }



    return bAccepted;

}



void AContractoryMissionSubsystem::TryUnlockContractoryMachines()

{

    UWorld* World = GetWorld();

    if (!World || !HasAuthority())

    {

        return;

    }



    AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(World);

    AFGRecipeManager* RecipeManager = AFGRecipeManager::Get(World);



    if (!SchematicManager || !RecipeManager)

    {

        UE_LOG(

            LogTemp,

            Verbose,

            TEXT("[Contractory] Waiting for SchematicManager / RecipeManager...")

        );

        return;

    }



    const int32 HighestTier =

        SchematicManager->GetHighestAvailableTechTier();



    if (HighestTier < 2)

    {

        return;

    }



    const TCHAR* RecipePaths[] =

    {

        TEXT("/SatisfactoryReverse/building/Blueprints/Recipe_Build_Contractory_Terminal.Recipe_Build_Contractory_Terminal_C"),

        TEXT("/SatisfactoryReverse/building/Blueprints/Recipe_Build_Contractory_Boutique.Recipe_Build_Contractory_Boutique_C"),

        TEXT("/SatisfactoryReverse/building/Blueprints/Recipe_Build_Contractory_Livraison.Recipe_Build_Contractory_Livraison_C")

    };



    int32 ValidRecipeCount = 0;

    int32 NewlyUnlockedCount = 0;



    for (const TCHAR* RecipePath : RecipePaths)

    {

        UClass* LoadedClass = LoadClass<UFGRecipe>(

            nullptr,

            RecipePath

        );



        if (!LoadedClass)

        {

            UE_LOG(

                LogTemp,

                Error,

                TEXT("[Contractory] Unable to load machine recipe: %s"),

                RecipePath

            );

            continue;

        }



        TSubclassOf<UFGRecipe> RecipeClass = LoadedClass;

        ++ValidRecipeCount;



        if (!RecipeManager->IsRecipeAvailable(RecipeClass))

        {

            RecipeManager->AddAvailableRecipe(RecipeClass);

            ++NewlyUnlockedCount;



            UE_LOG(

                LogTemp,

                Log,

                TEXT("[Contractory] Machine recipe unlocked: %s"),

                *LoadedClass->GetName()

            );

        }

    }



    // On arrÃƒÆ’Ã‚Âªte le contrÃƒÆ’Ã‚Â´le seulement quand les 3 classes ont bien ÃƒÆ’Ã‚Â©tÃƒÆ’Ã‚Â© chargÃƒÆ’Ã‚Â©es.

    if (ValidRecipeCount == 3)

    {

        World->GetTimerManager().ClearTimer(MachineUnlockRetryTimer);



        UE_LOG(

            LogTemp,

            Log,

            TEXT(

                "[Contractory] Machine unlock migration complete. "

                "Tier=%d NewRecipes=%d"

            ),

            HighestTier,

            NewlyUnlockedCount

        );

    }

}



bool AContractoryMissionSubsystem::GetMissionStateForPlayer(

    AFGPlayerState* PlayerState,

    int32& OutCredits,

    TArray<FContractoryMissionDefinition>& OutOffers,

    TArray<FContractoryActiveMission>& OutActiveMissions

)

{

    OutCredits = 0;

    OutOffers.Reset();

    OutActiveMissions.Reset();



    if (!HasAuthority() ||

        !PlayerState ||

        !MissionSaveManager)

    {

        return false;

    }



    const FString PlayerKey =

        TEXT("CONTRACTORY_SHARED");



    if (PlayerKey.IsEmpty())

    {

        UE_LOG(

            LogTemp,

            Warning,

            TEXT("[Contractory] Mission state refused: invalid player user id.")

        );



        return false;

    }



    FContractoryPlayerMissionState* PlayerMissionState =

        MissionSaveManager->FindOrAddServerPlayer(PlayerKey);



    if (!PlayerMissionState)

    {

        return false;

    }



    AFGSchematicManager* SchematicManager =

        AFGSchematicManager::Get(GetWorld());



    if (!SchematicManager)

    {

        return false;

    }



    const int32 HighestTier =

        SchematicManager->GetHighestAvailableTechTier();



    OutCredits =

        PlayerMissionState->Session.Credits;



    OutOffers =

        UContractoryMissionManager::GetFiveOffers(

            HighestTier,

            PlayerMissionState->RotationIndex

        );



    // CONTRACTORY_FILTER_EXHAUSTED_100
    OutOffers.RemoveAll(
        [PlayerMissionState](const FContractoryMissionDefinition& Mission)
        {
            return UContractoryMissionManager::GetMissionCompletionCount(
                PlayerMissionState->Session, Mission.MissionId
            ) >= UContractoryMissionManager::GetMissionCompletionLimit(
                Mission.MissionId
            );
        }
    );

    OutActiveMissions =

        PlayerMissionState->Session.ActiveMissions;



    return true;

}



bool AContractoryMissionSubsystem::PurchaseBoutiqueSchematicForPlayer(

    AFGPlayerState* PlayerState,

    const FString& SchematicClassPath,

    int32& OutRemainingCredits,

    FString& OutFailureReason

)

{

    OutRemainingCredits = 0;

    OutFailureReason.Reset();



    if (!HasAuthority() ||

        !PlayerState ||

        !MissionSaveManager)

    {

        OutFailureReason = TEXT("Serveur indisponible");

        return false;

    }



    const FString PlayerKey = TEXT("CONTRACTORY_SHARED");



    if (PlayerKey.IsEmpty())

    {

        OutFailureReason = TEXT("Joueur invalide");

        return false;

    }



    FContractoryBoutiqueEntry Entry;



    if (!UContractoryBoutiqueCatalog::FindBySchematicClassPath(

            SchematicClassPath,

            Entry))

    {

        OutFailureReason = TEXT("Recherche absente du catalogue");

        return false;

    }



    if (Entry.PriceCredits <= 0)

    {

        OutFailureReason = TEXT("Prix invalide");

        return false;

    }

    // CONTRACTORY_SERVER_TIER_100
    AFGSchematicManager* TierManager = AFGSchematicManager::Get(GetWorld());

    if (!TierManager ||
        TierManager->GetHighestAvailableTechTier() < Entry.MinimumTier)
    {
        OutFailureReason = TEXT("Palier insuffisant");
        return false;
    }



    FContractoryPlayerMissionState* PlayerMissionState =

        MissionSaveManager->FindOrAddServerPlayer(PlayerKey);



    if (!PlayerMissionState)

    {

        OutFailureReason = TEXT("Etat Contractory indisponible");

        return false;

    }



    OutRemainingCredits =

        PlayerMissionState->Session.Credits;



        // CONTRACTORY_HOVERPACK_DIRECT_INVENTORY

    if (

        Entry.SchematicClassPath ==

        TEXT("CONTRACTORY_ITEM_HOVERPACK")

    )

    {
        // CONTRACTORY_HOVERPACK_UNIQUE_100
        if (PlayerMissionState->Session.bHoverpackPurchased)
        {
            OutFailureReason = TEXT("Hoverpack deja achete");
            return false;
        }


        if (PlayerMissionState->Session.Credits < Entry.PriceCredits)

        {

            OutFailureReason = TEXT("Credits FICSIT insuffisants");

            return false;

        }



        AFGCharacterPlayer* PlayerCharacter =

            Cast<AFGCharacterPlayer>(PlayerState->GetPawn());



        if (!PlayerCharacter)

        {

            OutFailureReason = TEXT("Personnage joueur indisponible");

            return false;

        }



        UFGInventoryComponent* PlayerInventory =

            PlayerCharacter->GetInventory();



        if (!PlayerInventory)

        {

            OutFailureReason = TEXT("Inventaire joueur indisponible");

            return false;

        }



        TSubclassOf<UFGItemDescriptor> HoverPackClass =

            LoadClass<UFGItemDescriptor>(

                nullptr,

                TEXT(

                    "/Game/FactoryGame/Resource/Equipment/HoverPack/"

                    "BP_EquipmentDescriptorHoverPack."

                    "BP_EquipmentDescriptorHoverPack_C"

                )

            );



        if (!HoverPackClass)

        {

            OutFailureReason = TEXT("Descriptor Hoverpack introuvable");

            return false;

        }



        const FInventoryStack HoverPackStack(1,HoverPackClass);



        if (!PlayerInventory->HasEnoughSpaceForStack(HoverPackStack))

        {

            OutFailureReason =

                TEXT("Inventaire plein : liberez une place");

            return false;

        }



        const int32 Added =

            PlayerInventory->AddStack(HoverPackStack,false);



        if (Added != 1)

        {

            OutFailureReason =

                TEXT("Impossible d'ajouter le Hoverpack");

            return false;

        }



        // CONTRACTORY_HOVERPACK_SAVE_100
        PlayerMissionState->Session.bHoverpackPurchased = true;
        PlayerMissionState->Session.Credits -= Entry.PriceCredits;



        OutRemainingCredits =

            PlayerMissionState->Session.Credits;



        MissionSaveManager->ForceNetUpdate();



        UE_LOG(

            LogTemp,

            Log,

            TEXT(

                "[Contractory][Boutique] Hoverpack reel ajoute "

                "| Cout=%d | Credits restants=%d"

            ),

            Entry.PriceCredits,

            OutRemainingCredits

        );



        return true;

    }

    TSubclassOf<UFGSchematic> SchematicClass =

        LoadClass<UFGSchematic>(

            nullptr,

            *Entry.SchematicClassPath

        );



    if (!SchematicClass)

    {

        OutFailureReason = TEXT("Schematic vanilla introuvable");

        return false;

    }



    AFGSchematicManager* SchematicManager =

        AFGSchematicManager::Get(GetWorld());



    if (!SchematicManager)

    {

        OutFailureReason = TEXT("SchematicManager indisponible");

        return false;

    }



    if (SchematicManager->IsSchematicPurchased(SchematicClass))

    {

        OutFailureReason = TEXT("Recherche deja achetee");

        return false;

    }



    if (!UFGSchematic::CanGiveAccessToSchematic(

            SchematicClass,

            GetWorld()))

    {

        OutFailureReason = TEXT("Dependances non remplies");

        return false;

    }



    if (PlayerMissionState->Session.Credits < Entry.PriceCredits)

    {

        OutFailureReason = TEXT("Credits FICSIT insuffisants");

        return false;

    }



    //

    // IMPORTANT :

    // On debloque AVANT de debiter.

    // Si Satisfactory refuse le schematic pour une raison quelconque,

    // aucun credit Contractory n'est perdu.

    //

    SchematicManager->GiveAccessToSchematic(

        SchematicClass,

        nullptr,

        ESchematicUnlockFlags::None

    );



    if (!SchematicManager->IsSchematicPurchased(SchematicClass))

    {

        OutFailureReason = TEXT("Le deblocage vanilla a echoue");

        return false;

    }



    PlayerMissionState->Session.Credits -=

        Entry.PriceCredits;



    OutRemainingCredits =

        PlayerMissionState->Session.Credits;



    MissionSaveManager->ForceNetUpdate();



    UE_LOG(

        LogTemp,

        Log,

        TEXT(

            "[Contractory][Boutique] Achat OK: %s | Cout=%d | Credits restants=%d"

        ),

        *Entry.SchematicClassPath,

        Entry.PriceCredits,

        OutRemainingCredits

    );



    return true;

}

bool AContractoryMissionSubsystem::ClaimMissionForPlayer(

    AFGPlayerState* PlayerState,

    int32 MissionId,

    int32& OutCreditsAwarded

)

{

    OutCreditsAwarded = 0;



    if (!HasAuthority() ||

        !PlayerState ||

        !MissionSaveManager)

    {

        return false;

    }



    const FString PlayerKey =

        TEXT("CONTRACTORY_SHARED");



    if (PlayerKey.IsEmpty())

    {

        return false;

    }



    FContractoryPlayerMissionState* PlayerMissionState =

        MissionSaveManager->FindOrAddServerPlayer(PlayerKey);



    if (!PlayerMissionState)

    {

        return false;

    }



    const bool bClaimed =

        UContractoryMissionManager::ClaimCompletedMission(

            PlayerMissionState->Session,

            MissionId,

            OutCreditsAwarded

        );



    if (bClaimed)

    {

        MissionSaveManager->ForceNetUpdate();



        UE_LOG(

            LogTemp,

            Log,

            TEXT(

                "[Contractory] Mission %d claimed: +%d credits."

            ),

            MissionId,

            OutCreditsAwarded

        );

    }



    return bClaimed;

}



bool AContractoryMissionSubsystem::RefreshOffersForPlayer(

    AFGPlayerState* PlayerState

)

{
    // CONTRACTORY_NO_REFRESH_100
    return false;
}

