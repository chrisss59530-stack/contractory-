#include "ContractoryBoutiqueCatalog.h"



TArray<FContractoryBoutiqueEntry> UContractoryBoutiqueCatalog::GetCatalogue()

{

    TArray<FContractoryBoutiqueEntry> Result;

    Result.Reserve(74);



        // CONTRACTORY_BOUTIQUE_TIER_RULES_V1
    auto GetMinimumTier =
        [](const FString& Category, const FString& Path) -> int32
    {
        // CONTRACTORY_HOVERPACK_TIER5_100
        if (Path == TEXT("CONTRACTORY_ITEM_HOVERPACK"))
        {
            return 5;
        }

        if (Category == TEXT("Nutriments"))
        {
            return 1;
        }

        if (Category == TEXT("Organismes extraterrestres"))
        {
            return 2;
        }

        if (Category == TEXT("Caterium"))
        {
            if (Path.Contains(TEXT("Research_Caterium_7")))
                return 6;

            if (Path.Contains(TEXT("Research_Caterium_6")) ||
                Path.Contains(TEXT("Research_Caterium_5")))
                return 5;

            if (Path.Contains(TEXT("Research_Caterium_4")))
                return 4;

            if (Path.Contains(TEXT("Research_Caterium_3")))
                return 3;

            return 2;
        }

        if (Category == TEXT("Mycelium"))
        {
            if (Path.Contains(TEXT("GasMask")))
                return 5;

            if (Path.Contains(TEXT("Research_Mycelia_7")) ||
                Path.Contains(TEXT("Research_Mycelia_8")))
                return 4;

            return 2;
        }

        if (Category == TEXT("Quartz"))
        {
            if (Path.Contains(TEXT("PriorityMerger")))
                return 6;

            if (Path.Contains(TEXT("Research_Quartz_4")))
                return 5;

            return 3;
        }

        if (Category == TEXT("Soufre"))
        {
            if (Path.Contains(TEXT("IonizedFuel")))
                return 8;

            if (Path.Contains(TEXT("RocketFuel")))
                return 7;

            if (Path.Contains(TEXT("ExperimentalPower")))
                return 6;

            if (Path.Contains(TEXT("TurboFuel")))
                return 5;

            if (Path.Contains(TEXT("Research_Sulfur_6")))
                return 6;

            if (Path.Contains(TEXT("Research_Sulfur_5")))
                return 5;

            if (Path.Contains(TEXT("Research_Sulfur_4")))
                return 4;

            return 3;
        }

        return 0;
    };

    auto Add =
        [&Result, &GetMinimumTier](
            const TCHAR* Category,
            const TCHAR* SchematicClassPath,
            int32 PriceCredits)
    {
        FContractoryBoutiqueEntry Entry;

        Entry.Category = Category;
        Entry.SchematicClassPath = SchematicClassPath;

        // CONTRACTORY_BOUTIQUE_PRICE_TABLE_V1
        auto ConvertBoutiquePrice =
            [](int32 BasePrice, const FString& Path) -> int32
        {
            if (Path.Contains(TEXT("IonizedFuel")))
                return 3500;

            if (Path.Contains(TEXT("RocketFuel")))
                return 2500;

            if (Path.Contains(TEXT("ExperimentalPower")))
                return 2000;

            if (Path.Contains(TEXT("TurboFuel")))
                return 1500;

            switch (BasePrice)
            {
                case 3:  return 100;
                case 5:  return 175;
                case 8:  return 300;
                case 10: return 450;
                case 12: return 600;
                case 15: return 850;
                case 20: return 1200;
                case 25: return 1600;
                default: return FMath::Max(BasePrice, 100);
            }
        };

        Entry.PriceCredits =
            ConvertBoutiquePrice(
                PriceCredits,
                Entry.SchematicClassPath
            );

        Entry.MinimumTier =
            GetMinimumTier(
                Entry.Category,
                Entry.SchematicClassPath
            );

        Result.Add(MoveTemp(Entry));
    };



    // Caterium

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_0.Research_Caterium_0_C"), 3);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_1.Research_Caterium_1_C"), 3);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_2.Research_Caterium_2_C"), 5);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_2_1.Research_Caterium_2_1_C"), 10);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_3.Research_Caterium_3_C"), 5);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_3_1.Research_Caterium_3_1_C"), 10);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_3_2.Research_Caterium_3_2_C"), 10);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_4_1.Research_Caterium_4_1_C"), 5);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_4_1_1.Research_Caterium_4_1_1_C"), 10);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_4_1_2.Research_Caterium_4_1_2_C"), 10);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_4_2.Research_Caterium_4_2_C"), 8);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_4_3.Research_Caterium_4_3_C"), 15);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_5.Research_Caterium_5_C"), 15);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_5_1.Research_Caterium_5_1_C"), 20);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_5_2.Research_Caterium_5_2_C"), 10);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_6_2.Research_Caterium_6_2_C"), 12);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_6_3.Research_Caterium_6_3_C"), 15);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_7_1.Research_Caterium_7_1_C"), 15);

    Add(TEXT("Caterium"), TEXT("/Game/FactoryGame/Schematics/Research/Caterium_RS/Research_Caterium_7_2.Research_Caterium_7_2_C"), 25);



    // Organismes extraterrestres

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_ACarapace_0.Research_ACarapace_0_C"), 3);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AOrgans_0.Research_AOrgans_0_C"), 3);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AO_Hatcher.Research_AO_Hatcher_C"), 3);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AO_Stinger.Research_AO_Stinger_C"), 3);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AO_DNACapsule.Research_AO_DNACapsule_C"), 3);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AO_Pre_Rebar.Research_AO_Pre_Rebar_C"), 5);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AOrganisms_2.Research_AOrganisms_2_C"), 5);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_ACarapace_2.Research_ACarapace_2_C"), 5);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AOrgans_2.Research_AOrgans_2_C"), 5);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_ACarapace_3.Research_ACarapace_3_C"), 5);

    Add(TEXT("Organismes extraterrestres"), TEXT("/Game/FactoryGame/Schematics/Research/AlienOrganisms_RS/Research_AOrgans_3.Research_AOrgans_3_C"), 5);



    // Mycelium

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_1.Research_Mycelia_1_C"), 3);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_2.Research_Mycelia_2_C"), 5);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_2_1.Research_Mycelia_2_1_C"), 10);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_3.Research_Mycelia_3_C"), 8);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_4.Research_Mycelia_4_C"), 5);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_5.Research_Mycelia_5_C"), 5);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_6.Research_Mycelia_6_C"), 10);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_7.Research_Mycelia_7_C"), 10);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_8.Research_Mycelia_8_C"), 10);

    Add(TEXT("Mycelium"), TEXT("/Game/FactoryGame/Schematics/Research/Mycelia_RS/Research_Mycelia_GasMask.Research_Mycelia_GasMask_C"), 10);



    // Nutriments

    Add(TEXT("Nutriments"), TEXT("/Game/FactoryGame/Schematics/Research/Nutrients_RS/Research_Nutrients_0.Research_Nutrients_0_C"), 3);

    Add(TEXT("Nutriments"), TEXT("/Game/FactoryGame/Schematics/Research/Nutrients_RS/Research_Nutrients_1.Research_Nutrients_1_C"), 3);

    Add(TEXT("Nutriments"), TEXT("/Game/FactoryGame/Schematics/Research/Nutrients_RS/Research_Nutrients_2.Research_Nutrients_2_C"), 3);

    Add(TEXT("Nutriments"), TEXT("/Game/FactoryGame/Schematics/Research/Nutrients_RS/Research_Nutrients_3.Research_Nutrients_3_C"), 5);

    Add(TEXT("Nutriments"), TEXT("/Game/FactoryGame/Schematics/Research/Nutrients_RS/Research_Nutrients_4.Research_Nutrients_4_C"), 10);



    // Power Slugs



    // Quartz

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_0.Research_Quartz_0_C"), 3);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_1_1.Research_Quartz_1_1_C"), 5);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_1_2.Research_Quartz_1_2_C"), 5);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_3.Research_Quartz_3_C"), 5);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_2.Research_Quartz_2_C"), 10);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_2_1.Research_Quartz_2_1_C"), 10);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_3_4.Research_Quartz_3_4_C"), 10);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_4_1.Research_Quartz_4_1_C"), 10);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_3_1.Research_Quartz_3_1_C"), 15);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_4.Research_Quartz_4_C"), 15);

    Add(TEXT("Quartz"), TEXT("/Game/FactoryGame/Schematics/Research/Quartz_RS/Research_Quartz_PriorityMerger.Research_Quartz_PriorityMerger_C"), 15);



    // Soufre

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_0.Research_Sulfur_0_C"), 3);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_1.Research_Sulfur_1_C"), 5);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_2.Research_Sulfur_2_C"), 5);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_3.Research_Sulfur_3_C"), 10);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_3_1.Research_Sulfur_3_1_C"), 10);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_4_2.Research_Sulfur_4_2_C"), 10);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_compactedCoal.Research_Sulfur_CompactedCoal_C"), 10);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_6.Research_Sulfur_6_C"), 10);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_5.Research_Sulfur_5_C"), 10);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_4.Research_Sulfur_4_C"), 15);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_4_1.Research_Sulfur_4_1_C"), 15);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_5_2.Research_Sulfur_5_2_C"), 15);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_ExperimentalPower.Research_Sulfur_ExperimentalPower_C"), 15);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_TurboFuel.Research_Sulfur_TurboFuel_C"), 15);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_5_1.Research_Sulfur_5_1_C"), 20);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_RocketFuel.Research_Sulfur_RocketFuel_C"), 20);

    Add(TEXT("Soufre"), TEXT("/Game/FactoryGame/Schematics/Research/Sulfur_RS/Research_Sulfur_IonizedFuel.Research_Sulfur_IonizedFuel_C"), 25);



    // Equipement special : vrai Hoverpack directement dans l'inventaire
    Add(
        TEXT("Equipement"),
        TEXT("CONTRACTORY_ITEM_HOVERPACK"),
        1500
    );

    return Result;

}



bool UContractoryBoutiqueCatalog::FindBySchematicClassPath(

    const FString& SchematicClassPath,

    FContractoryBoutiqueEntry& OutEntry

)

{

    for (const FContractoryBoutiqueEntry& Entry : GetCatalogue())

    {

        if (Entry.SchematicClassPath.Equals(SchematicClassPath, ESearchCase::CaseSensitive))

        {

            OutEntry = Entry;

            return true;

        }

    }



    return false;

}



int32 UContractoryBoutiqueCatalog::GetTotalCatalogueCost()

{

    int32 Total = 0;



    for (const FContractoryBoutiqueEntry& Entry : GetCatalogue())

    {

        Total += Entry.PriceCredits;

    }



    return Total;

}