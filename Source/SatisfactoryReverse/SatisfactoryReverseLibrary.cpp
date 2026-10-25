#include "SatisfactoryReverseLibrary.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Creature/FGCreature.h"
#include "FGFallingGiftBundle.h"
#include "FGItemPickup_Spawnable.h"
#include "Resources/FGItemDescriptor.h"
#include "FGInventoryComponent.h"
#include "FGCharacterPlayer.h"
#include "AvailabilityDependencies/FGAvailabilityDependency.h"
#include "FGSchematic.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "FGRecipe.h"
#include "UObject/UnrealType.h"

bool USatisfactoryReverseLibrary::ReverseIronPlateRecipe()
{
    UClass* RecipeClass = LoadClass<UFGRecipe>(
        nullptr,
        TEXT("/Game/FactoryGame/Recipes/Constructor/Recipe_IronPlate.Recipe_IronPlate_C")
    );

    if (!RecipeClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Recette introuvable"));
        return false;
    }

    UFGRecipe* Recipe = RecipeClass->GetDefaultObject<UFGRecipe>();

    FArrayProperty* IngredientsProperty =
        FindFProperty<FArrayProperty>(UFGRecipe::StaticClass(), TEXT("mIngredients"));

    FArrayProperty* ProductsProperty =
        FindFProperty<FArrayProperty>(UFGRecipe::StaticClass(), TEXT("mProduct"));

    if (!IngredientsProperty || !ProductsProperty)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Proprietes introuvables"));
        return false;
    }

    void* Ingredients = IngredientsProperty->ContainerPtrToValuePtr<void>(Recipe);
    void* Products = ProductsProperty->ContainerPtrToValuePtr<void>(Recipe);

        TArray<FItemAmount>* IngredientsArray =
        reinterpret_cast<TArray<FItemAmount>*>(Ingredients);

    TArray<FItemAmount>* ProductsArray =
        reinterpret_cast<TArray<FItemAmount>*>(Products);

    Swap(*IngredientsArray, *ProductsArray);

    UE_LOG(LogTemp, Log, TEXT("[SatisfactoryReverse] Recette plaque de fer inversee !"));
    return true;
}


#include "Registry/ModContentRegistry.h"

int32 USatisfactoryReverseLibrary::CountRegisteredRecipes(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Contexte invalide"));
        return -1;
    }

    UModContentRegistry* Registry = UModContentRegistry::Get(WorldContextObject);

    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Registre introuvable"));
        return -1;
    }

    const TArray<FGameObjectRegistration> Recipes = Registry->GetRegisteredRecipes();

    int32 VanillaCount = 0;
    int32 ModCount = 0;

    for (const FGameObjectRegistration& Entry : Recipes)
    {
        if (Entry.HasAnyFlags(EGameObjectRegistrationFlags::BuiltIn))
        {
            ++VanillaCount;
        }
        else
        {
            ++ModCount;
        }
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[SatisfactoryReverse] Recettes enregistrees : Total=%d, Vanilla=%d, Mods=%d"),
        Recipes.Num(), VanillaCount, ModCount);

    return Recipes.Num();
}



int32 USatisfactoryReverseLibrary::CountManufacturingRecipes(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return -1;
    }

    UModContentRegistry* Registry = UModContentRegistry::Get(WorldContextObject);

    if (!Registry)
    {
        return -1;
    }

    int32 ManufacturingCount = 0;
    int32 OtherCount = 0;

    for (const FGameObjectRegistration& Entry : Registry->GetRegisteredRecipes())
    {
        if (!Entry.HasAnyFlags(EGameObjectRegistrationFlags::BuiltIn) ||
            Entry.HasAnyFlags(EGameObjectRegistrationFlags::Removed))
        {
            continue;
        }

        UClass* RecipeClass = Cast<UClass>(Entry.RegisteredObject.Get());

        if (!RecipeClass || !RecipeClass->IsChildOf(UFGRecipe::StaticClass()))
        {
            continue;
        }

        bool bIsManufacturing = false;

        const TArray<TSubclassOf<UObject>> Producers =
            UFGRecipe::GetProducedIn(RecipeClass);

        for (const TSubclassOf<UObject>& Producer : Producers)
        {
            if (Producer.Get() &&
                Producer.Get()->IsChildOf(AFGBuildableManufacturer::StaticClass()))
            {
                bIsManufacturing = true;
                break;
            }
        }

        if (bIsManufacturing)
        {
            ++ManufacturingCount;
        }
        else
        {
            ++OtherCount;
        }
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[SatisfactoryReverse] Diagnostic fabrication : Machines=%d, Autres=%d"),
        ManufacturingCount, OtherCount);

    return ManufacturingCount;
}


int32 USatisfactoryReverseLibrary::ReverseManufacturingRecipes(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Inversion : contexte invalide"));
        return -1;
    }

    UModContentRegistry* Registry = UModContentRegistry::Get(WorldContextObject);

    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Inversion : registre introuvable"));
        return -1;
    }

    // Conserve les recettes deja inversees durant cette session.
    static TSet<UClass*> ReversedRecipes;

    FArrayProperty* IngredientsProperty =
        FindFProperty<FArrayProperty>(UFGRecipe::StaticClass(), TEXT("mIngredients"));

    FArrayProperty* ProductsProperty =
        FindFProperty<FArrayProperty>(UFGRecipe::StaticClass(), TEXT("mProduct"));

    if (!IngredientsProperty || !ProductsProperty)
    {
        UE_LOG(LogTemp, Error, TEXT("[SatisfactoryReverse] Inversion : proprietes introuvables"));
        return -1;
    }

    int32 ReversedCount = 0;
    int32 SkippedCount = 0;

    for (const FGameObjectRegistration& Entry : Registry->GetRegisteredRecipes())
    {
        // Ne traiter que les recettes du jeu original, non supprimees.
        if (!Entry.HasAnyFlags(EGameObjectRegistrationFlags::BuiltIn) ||
            Entry.HasAnyFlags(EGameObjectRegistrationFlags::Removed))
        {
            continue;
        }

        UClass* RecipeClass = Cast<UClass>(Entry.RegisteredObject.Get());

        if (!RecipeClass ||
            !RecipeClass->IsChildOf(UFGRecipe::StaticClass()) ||
            ReversedRecipes.Contains(RecipeClass))
        {
            continue;
        }

        // Verifier si la recette utilise une machine de fabrication.
        bool bIsManufacturing = false;

        const TArray<TSubclassOf<UObject>> Producers =
            UFGRecipe::GetProducedIn(RecipeClass);

        for (const TSubclassOf<UObject>& Producer : Producers)
        {
            if (Producer.Get() &&
                Producer.Get()->IsChildOf(AFGBuildableManufacturer::StaticClass()))
            {
                bIsManufacturing = true;
                break;
            }
        }

        if (!bIsManufacturing)
        {
            continue;
        }

        UFGRecipe* Recipe = RecipeClass->GetDefaultObject<UFGRecipe>();

        if (!Recipe)
        {
            ++SkippedCount;
            continue;
        }

        TArray<FItemAmount>* Ingredients =
            IngredientsProperty->ContainerPtrToValuePtr<TArray<FItemAmount>>(Recipe);

        TArray<FItemAmount>* Products =
            ProductsProperty->ContainerPtrToValuePtr<TArray<FItemAmount>>(Recipe);

        if (!Ingredients || !Products ||
            Ingredients->IsEmpty() || Products->IsEmpty())
        {
            ++SkippedCount;
            continue;
        }

        // Conserver les premieres transformations normales.
        const FString RecipeName = RecipeClass->GetName();

        if (RecipeName.StartsWith(TEXT("Recipe_Ingot")) ||
            RecipeName.StartsWith(TEXT("Recipe_Concrete")) ||
            RecipeName.StartsWith(TEXT("Recipe_Silica")) ||
            RecipeName.StartsWith(TEXT("Recipe_QuartzCrystal")))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[SatisfactoryReverse] Premiere transformation conservee : %s"),
                *RecipeName);
            continue;
        }

        Swap(*Ingredients, *Products);

        ReversedRecipes.Add(RecipeClass);
        ++ReversedCount;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[SatisfactoryReverse] Inversion automatique : %d inversees, %d ignorees"),
        ReversedCount, SkippedCount);

    return ReversedCount;
}




bool USatisfactoryReverseLibrary::ActiverHardcoreTier2(
    const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return false;
    }

    UModContentRegistry* Registry =
        UModContentRegistry::Get(WorldContextObject);

    FArrayProperty* ArrayProperty =
        FindFProperty<FArrayProperty>(
            UFGSchematic::StaticClass(),
            TEXT("mSchematicDependencies")
        );

    FObjectPropertyBase* ObjectProperty =
        ArrayProperty
        ? CastField<FObjectPropertyBase>(ArrayProperty->Inner)
        : nullptr;

    if (!Registry || !ArrayProperty || !ObjectProperty)
    {
        return false;
    }

    UClass* Dependencies[10] = {};

    for (int32 Tier = 2; Tier <= 9; ++Tier)
    {
        const FString Path = FString::Printf(
            TEXT("/SatisfactoryReverse/BP_FICSIT_Contrats_Obligatoires_Tier%d.BP_FICSIT_Contrats_Obligatoires_Tier%d_C"),
            Tier - 1, Tier - 1
        );

        Dependencies[Tier] =
            LoadClass<UFGAvailabilityDependency>(nullptr, *Path);

        if (!Dependencies[Tier])
        {
            UE_LOG(LogTemp, Error,
                TEXT("[FICSIT Hardcore] Dependance manquante : palier %d"),
                Tier - 1);
            return false;
        }
    }

    int32 Modified[10] = {};

    for (const FGameObjectRegistration& Entry :
         Registry->GetRegisteredSchematics())
    {
        if (!Entry.HasAnyFlags(EGameObjectRegistrationFlags::BuiltIn) ||
            Entry.HasAnyFlags(EGameObjectRegistrationFlags::Removed))
        {
            continue;
        }

        UClass* SchematicClass =
            Cast<UClass>(Entry.RegisteredObject.Get());

        if (!SchematicClass ||
            !SchematicClass->IsChildOf(UFGSchematic::StaticClass()))
        {
            continue;
        }

        // Ne modifier que les jalons de progression vanilla.
        if (!SchematicClass->GetPathName().StartsWith(
            TEXT("/Game/FactoryGame/Schematics/Progression/")))
        {
            continue;
        }

        const int32 Tier = UFGSchematic::GetTechTier(SchematicClass);

        if (Tier < 2 || Tier > 9)
        {
            continue;
        }

        UFGSchematic* Defaults =
            SchematicClass->GetDefaultObject<UFGSchematic>();

        FScriptArrayHelper Array(
            ArrayProperty,
            ArrayProperty->ContainerPtrToValuePtr<void>(Defaults)
        );

        bool AlreadyPresent = false;

        for (int32 Index = 0; Index < Array.Num(); ++Index)
        {
            UObject* Existing =
                ObjectProperty->GetObjectPropertyValue(
                    Array.GetRawPtr(Index)
                );

            if (Existing && Existing->IsA(Dependencies[Tier]))
            {
                AlreadyPresent = true;
                break;
            }
        }

        if (AlreadyPresent)
        {
            ++Modified[Tier];
            continue;
        }

        UFGAvailabilityDependency* Dependency =
            DuplicateObject<UFGAvailabilityDependency>(
                Dependencies[Tier]->GetDefaultObject<UFGAvailabilityDependency>(),
                Defaults
            );

        if (!Dependency)
        {
            continue;
        }

        Dependency->SetFlags(RF_Transient);

        const int32 Index = Array.AddValue();

        ObjectProperty->SetObjectPropertyValue(
            Array.GetRawPtr(Index),
            Dependency
        );

        ++Modified[Tier];
    }

    int32 Total = 0;

    for (int32 Tier = 2; Tier <= 9; ++Tier)
    {
        Total += Modified[Tier];

        UE_LOG(LogTemp, Warning,
            TEXT("[FICSIT Hardcore] Palier %d : %d jalons configures"),
            Tier, Modified[Tier]);
    }

    return Total > 0;
}


bool USatisfactoryReverseLibrary::DonnerCertificatFICSIT(
    TSubclassOf<UFGSchematic> Schematic,
    AFGCharacterPlayer* Joueur)
{
    if (!Joueur || !Joueur->HasAuthority() || !Schematic)
    {
        return false;
    }

    const FString Prefix = TEXT("Schematic_FICSIT_Contract_");

    FString Numero = Schematic.Get()->GetName();

    if (!Numero.RemoveFromStart(Prefix))
    {
        return false;
    }

    Numero.RemoveFromEnd(TEXT("_C"));

    if (Numero.Len() != 3 || !Numero.IsNumeric())
    {
        return false;
    }

    const int32 Id = FCString::Atoi(*Numero);

    if (Id < 1 || Id > 21)
    {
        return false;
    }

    const FString Chemin = FString::Printf(
        TEXT("/SatisfactoryReverse/Certificats/Desc_Certificat_FICSIT_%03d.Desc_Certificat_FICSIT_%03d_C"),
        Id, Id
    );

    TSubclassOf<UFGItemDescriptor> Certificat =
        LoadClass<UFGItemDescriptor>(nullptr, *Chemin);

    if (!Certificat)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[FICSIT] Certificat %03d introuvable"), Id);
        return false;
    }

    UFGInventoryComponent* Inventaire = Joueur->GetInventory();

    if (!Inventaire)
    {
        return false;
    }

    // Eviter un doublon deja present dans l'inventaire.
    if (Inventaire->GetNumItems(Certificat) > 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[FICSIT] Certificat %03d deja possede"), Id);
        return true;
    }

    // CONTRACTORY_DOGGO_035
    // Doggo offert uniquement lors du contrat 001.
    auto OffrirDoggoFICSIT = [&]()
    {
        if (Id != 1 || !Joueur || !Joueur->HasAuthority())
        {
            return;
        }

        UWorld* MondeDoggo = Joueur->GetWorld();

        if (!MondeDoggo)
        {
            return;
        }

        const FName MarqueDoggo(TEXT("Contractory_Doggo_001"));

        // Ne pas recreer un Doggo Contractory deja present.
        for (TActorIterator<AFGCreature> It(MondeDoggo); It; ++It)
        {
            if (IsValid(*It) && It->ActorHasTag(MarqueDoggo))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[FICSIT] Doggo 001 deja present"));
                return;
            }
        }

        UClass* ClasseDoggo = LoadClass<AFGCreature>(
            nullptr,
            TEXT("/Game/FactoryGame/Character/Creature/Wildlife/SpaceRabbit/Char_SpaceRabbit.Char_SpaceRabbit_C")
        );

        if (!ClasseDoggo)
        {
            UE_LOG(LogTemp, Error,
                TEXT("[FICSIT] Classe Doggo introuvable"));
            return;
        }

        const FVector PositionDoggo =
            Joueur->GetActorLocation() +
            Joueur->GetActorForwardVector() * 350.0f +
            FVector(0.0f, 0.0f, 100.0f);

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AFGCreature* Doggo = MondeDoggo->SpawnActor<AFGCreature>(
            ClasseDoggo,
            PositionDoggo,
            Joueur->GetActorRotation(),
            Params
        );

        if (IsValid(Doggo))
        {
            Doggo->Tags.AddUnique(MarqueDoggo);
            Doggo->SetIsPersistent(true);

            UE_LOG(LogTemp, Warning,
                TEXT("[FICSIT] DOGGO OFFERT - CONTRAT 001"));
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("[FICSIT] Echec apparition Doggo 001"));
        }
    };
    // CONTRACTORY_CERTIFICAT_INVENTAIRE_DIRECT
    // Le certificat est remis au joueur avant le largage des autres recompenses.
    FInventoryItem ObjetCertificat(Certificat);
    if (!Inventaire->HasEnoughSpaceForItem(ObjetCertificat))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[FICSIT] Inventaire plein : certificat %03d non remis"), Id);
        return false;
    }

    if (!Inventaire->AddItem(ObjetCertificat))
    {
        UE_LOG(LogTemp, Error,
            TEXT("[FICSIT] Echec ajout inventaire certificat %03d"), Id);
        return false;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[FICSIT] Certificat %03d ajoute directement a l'inventaire"), Id);
    // CONTRACTORY_REWARDS_080
    // Alpha 0.8.0 :
    // - certificat uniquement pour tous les contrats ;
    // - un Doggo vivant uniquement pour le contrat 001 ;
    // - aucune autre recompense materielle ;
    // - aucun largage / caisse / parachute.
    OffrirDoggoFICSIT();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[FICSIT] Contrat %03d : certificat remis - aucune recompense materielle"),
        Id
    );

    return true;
}




