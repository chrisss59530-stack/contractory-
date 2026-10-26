#include "ContractoryBoutiqueWidget.h"



#include "Resources/FGItemDescriptor.h"

#include "GameFramework/PlayerController.h"

#include "Blueprint/WidgetTree.h"

#include "Components/CanvasPanel.h"

#include "Components/CanvasPanelSlot.h"

#include "Components/Border.h"



#include "ContractoryBoutiqueCatalog.h"

#include "ContractoryMissionRemoteCallObject.h"



#include "FGPlayerController.h"

#include "FGSchematic.h"

#include "FGSchematicManager.h"

#include "Components/HorizontalBox.h"

#include "Components/VerticalBoxSlot.h"

#include "Components/Image.h"

#include "Components/ScrollBox.h"

// CONTRACTORY_BOUTIQUE_SEARCH_INCLUDE_V2

#include "Components/EditableTextBox.h"

#include "Components/SizeBox.h"

#include "Components/TextBlock.h"

#include "Components/VerticalBox.h"



#include "Engine/Texture2D.h"

#include "Engine/World.h"

#include "UObject/UObjectGlobals.h"





// ============================================================

// BOUTON D'ACHAT

// ============================================================



void UContractoryBoutiqueBuyButton::Configure(

    UContractoryMissionRemoteCallObject* InMissionRCO,

    const FString& InSchematicClassPath

)

{

    MissionRCO = InMissionRCO;

    SchematicClassPath = InSchematicClassPath;



    OnClicked.RemoveDynamic(

        this,

        &UContractoryBoutiqueBuyButton::HandleClicked

    );



    OnClicked.AddDynamic(

        this,

        &UContractoryBoutiqueBuyButton::HandleClicked

    );

}



void UContractoryBoutiqueBuyButton::HandleClicked()

{

    if (!MissionRCO ||

        SchematicClassPath.IsEmpty())

    {

        return;

    }



    MissionRCO->Server_PurchaseBoutiqueSchematic(

        SchematicClassPath

    );

}





// ============================================================

// WIDGET BOUTIQUE

// ============================================================



TSharedRef<SWidget> UContractoryBoutiqueWidget::RebuildWidget()

{

    UE_LOG(

        LogTemp,

        Warning,

        TEXT("[Contractory Boutique UI] RebuildWidget")

    );



    if (WidgetTree && !WidgetTree->RootWidget)

    {

        BuildBoutiqueInterface();

    }



    return Super::RebuildWidget();

}



void UContractoryBoutiqueWidget::SetInputMode_Implementation()

{

    Super::SetInputMode_Implementation();



    APlayerController* PlayerController = GetOwningPlayer();



    if (!PlayerController)

    {

        UE_LOG(

            LogTemp,

            Warning,

            TEXT("[Contractory Boutique UI] PlayerController NULL")

        );

        return;

    }



    PlayerController->bShowMouseCursor = true;

    PlayerController->SetIgnoreLookInput(true);

    PlayerController->SetIgnoreMoveInput(true);



    FInputModeGameAndUI InputMode;

    InputMode.SetWidgetToFocus(TakeWidget());

    InputMode.SetHideCursorDuringCapture(false);

    InputMode.SetLockMouseToViewportBehavior(

        EMouseLockMode::DoNotLock

    );



    PlayerController->SetInputMode(InputMode);



    UE_LOG(

        LogTemp,

        Warning,

        TEXT("[Contractory Boutique UI] SOURIS ACTIVE / CAMERA BLOQUEE")

    );

}



void UContractoryBoutiqueWidget::NativeConstruct()

{

    Super::NativeConstruct();



    if (!MainBox)

    {

        BuildBoutiqueInterface();

    }



    BindMissionRemoteCallObject();



    if (UWorld* World = GetWorld())

    {

        World->GetTimerManager().SetTimer(

            BoutiquePollTimer,

            this,

            &UContractoryBoutiqueWidget::PollBoutiqueState,

            0.5f,

            true

        );

    }



    RefreshBoutiqueInterface();

}



void UContractoryBoutiqueWidget::NativeDestruct()

{

    if (UWorld* World = GetWorld())

    {

        World->GetTimerManager().ClearTimer(

            BoutiquePollTimer

        );

    }



    if (MissionRCO)

    {

        MissionRCO->OnMissionStateUpdated.RemoveAll(this);

        MissionRCO->OnBoutiquePurchaseResult.RemoveAll(this);

    }



    Super::NativeDestruct();

}



void UContractoryBoutiqueWidget::BuildBoutiqueInterface()

{

    if (!WidgetTree)

    {

        return;

    }



    UCanvasPanel* RootCanvas =

        WidgetTree->ConstructWidget<UCanvasPanel>(

            UCanvasPanel::StaticClass(),

            TEXT("ContractoryBoutiqueRootCanvas")

        );



    USizeBox* RootSize =

        WidgetTree->ConstructWidget<USizeBox>(

            USizeBox::StaticClass(),

            TEXT("ContractoryBoutiqueRootSize")

        );



    RootSize->SetWidthOverride(700.0f);

    RootSize->SetHeightOverride(920.0f);



    MainBox =

        WidgetTree->ConstructWidget<UVerticalBox>(

            UVerticalBox::StaticClass(),

            TEXT("ContractoryBoutiqueMain")

        );



    // CONTRACTORY_SVG_FICSIT_2026 : panneau noir / bordure orange

    UBorder* BoutiqueFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BoutiqueFicsitFrame"));

    BoutiqueFrame->SetBrushColor(FLinearColor(0.949f, 0.549f, 0.157f, 1.0f));

    BoutiqueFrame->SetPadding(FMargin(5.0f));

    UBorder* BoutiqueBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BoutiqueFicsitBackground"));

    BoutiqueBackground->SetBrushColor(FLinearColor(0.067f, 0.094f, 0.125f, 1.0f));

    BoutiqueBackground->SetPadding(FMargin(18.0f, 12.0f));

    BoutiqueFrame->AddChild(BoutiqueBackground);

    BoutiqueBackground->AddChild(MainBox);

    RootSize->AddChild(BoutiqueFrame);



    WidgetTree->RootWidget = RootCanvas;



    if (UCanvasPanelSlot* RootSlot =

        RootCanvas->AddChildToCanvas(RootSize))

    {

        RootSlot->SetAnchors(

            FAnchors(0.5f, 0.5f)

        );



        RootSlot->SetAlignment(

            FVector2D(0.5f, 0.5f)

        );



        RootSlot->SetPosition(

            FVector2D(0.0f, 0.0f)

        );



        RootSlot->SetSize(FVector2D(700.0f, 920.0f));



        RootSlot->SetAutoSize(false);

    }



    // --------------------------------------------------------

    // TITRE

    // --------------------------------------------------------



    UTextBlock* Title =

        WidgetTree->ConstructWidget<UTextBlock>(

            UTextBlock::StaticClass(),

            TEXT("BoutiqueTitle")

        );



    Title->SetText(

        FText::FromString(

            TEXT("FICSIT // CONTRACTORY")

        )

    );



    UBorder* HeaderBanner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BoutiqueOrangeHeader"));

    HeaderBanner->SetBrushColor(FLinearColor(0.949f, 0.549f, 0.157f, 1.0f));

    HeaderBanner->SetPadding(FMargin(16.0f, 18.0f));

    HeaderBanner->AddChild(Title);

    MainBox->AddChildToVerticalBox(HeaderBanner);

    Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.067f, 0.094f, 0.125f, 1.0f)));

    { FSlateFontInfo Font = Title->GetFont(); Font.Size = 29; Title->SetFont(Font); }



    // --------------------------------------------------------

    // CREDITS

    // --------------------------------------------------------



    CreditsText =

        WidgetTree->ConstructWidget<UTextBlock>(

            UTextBlock::StaticClass(),

            TEXT("BoutiqueCredits")

        );



    CreditsText->SetText(

        FText::FromString(

            TEXT("CREDITS FICSIT : chargement...")

        )

    );



    UBorder* CreditsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BoutiqueCreditsPanel"));

    CreditsPanel->SetBrushColor(FLinearColor(0.161f, 0.212f, 0.251f, 1.0f));

    CreditsPanel->SetPadding(FMargin(12.0f, 12.0f));

    CreditsPanel->AddChild(CreditsText);

    if (UVerticalBoxSlot* CreditsSlot = MainBox->AddChildToVerticalBox(CreditsPanel))

        CreditsSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 6.0f));

    CreditsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.949f, 0.549f, 0.157f, 1.0f)));

    { FSlateFontInfo Font = CreditsText->GetFont(); Font.Size = 25; CreditsText->SetFont(Font); }



    // --------------------------------------------------------

    // MESSAGE RESULTAT

    // --------------------------------------------------------



    ResultText =

        WidgetTree->ConstructWidget<UTextBlock>(

            UTextBlock::StaticClass(),

            TEXT("BoutiqueResult")

        );



    ResultText->SetText(

        FText::FromString(

            TEXT("CATALOGUE OFFICIEL - Selectionnez une recherche")

        )

    );



    ResultText->SetAutoWrapText(true);



    MainBox->AddChildToVerticalBox(ResultText);

    ResultText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.82f, 0.84f, 1.0f)));

    // ========================================================

    // CONTRACTORY_BOUTIQUE_SEARCH_BOX_V2

    // ========================================================



    SearchBox =

        WidgetTree->ConstructWidget<UEditableTextBox>(

            UEditableTextBox::StaticClass(),

            TEXT("BoutiqueSearchBox")

        );



    SearchBox->SetHintText(

        FText::FromString(

            TEXT("Rechercher un article...")

        )

    );



    SearchBox->SetText(

        FText::GetEmpty()

    );



    SearchBox->OnTextChanged.AddDynamic(

        this,

        &UContractoryBoutiqueWidget::HandleSearchChanged

    );



    if (UVerticalBoxSlot* SearchSlot = MainBox->AddChildToVerticalBox(SearchBox))

        SearchSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 12.0f));



    // --------------------------------------------------------

    // SCROLL CATALOGUE

    // --------------------------------------------------------



    CatalogueScroll =

        WidgetTree->ConstructWidget<UScrollBox>(

            UScrollBox::StaticClass(),

            TEXT("BoutiqueCatalogue")

        );



    CatalogueScroll->SetOrientation(EOrientation::Orient_Vertical);

CatalogueScroll->SetScrollBarVisibility(ESlateVisibility::Visible);

CatalogueScroll->SetConsumeMouseWheel(EConsumeMouseWheel::Always);

CatalogueScroll->SetWheelScrollMultiplier(10.0f);



USizeBox* CatalogueArea =

    WidgetTree->ConstructWidget<USizeBox>(

        USizeBox::StaticClass(),

        TEXT("CatalogueArea")

    );



CatalogueArea->SetHeightOverride(640.0f);

CatalogueArea->AddChild(CatalogueScroll);



if (UVerticalBoxSlot* CatalogueSlot =

    MainBox->AddChildToVerticalBox(CatalogueArea))

{

    CatalogueSlot->SetSize(

        FSlateChildSize(ESlateSizeRule::Fill)

    );



    CatalogueSlot->SetPadding(

        FMargin(0.0f, 2.0f, 0.0f, 0.0f)

    );

}



    EntryStatusTexts.Reset();



    // CONTRACTORY_BOUTIQUE_SEARCH_RESET_V2

    EntryCards.Reset();

    EntrySearchTexts.Reset();

    CategoryTitles.Reset();

    CategoryNames.Reset();

    EntryCategories.Reset();

    EntryButtonTexts.Reset();

    BuyButtons.Reset();

    SchematicClasses.Reset();

    SchematicPaths.Reset();

    EntryPrices.Reset();



    const TArray<FContractoryBoutiqueEntry> Catalogue =

        UContractoryBoutiqueCatalog::GetCatalogue();



    FString LastCategory;



    for (int32 Index = 0;

         Index < Catalogue.Num();

         ++Index)

    {

        const FContractoryBoutiqueEntry& Entry =

            Catalogue[Index];



        // ----------------------------------------------------

        // TITRE DE CATEGORIE

        // ----------------------------------------------------



        if (Entry.Category != LastCategory)

        {

            LastCategory = Entry.Category;



            UTextBlock* CategoryTitle =

                WidgetTree->ConstructWidget<UTextBlock>(

                    UTextBlock::StaticClass()

                );



            CategoryTitle->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT("=== %s ==="),

                        *Entry.Category

                    )

                )

            );



            CatalogueScroll->AddChild(

                CategoryTitle

            );

            CategoryTitles.Add(CategoryTitle);

            CategoryNames.Add(Entry.Category);



            // CONTRACTORY_SEARCH_CATEGORY_084

            CategoryTitle->SetVisibility(ESlateVisibility::Visible);

        }



        // ----------------------------------------------------

        // CHARGEMENT DU VRAI SCHEMATIC VANILLA

        // ----------------------------------------------------

        // CONTRACTORY_HOVERPACK_UI_BUILD

        const bool bHoverpackItem =

            Entry.SchematicClassPath ==

            TEXT("CONTRACTORY_ITEM_HOVERPACK");



        TSubclassOf<UFGSchematic> SchematicClass =

            bHoverpackItem

                ? nullptr

                : LoadClass<UFGSchematic>(

                    nullptr,

                    *Entry.SchematicClassPath

                );



        SchematicClasses.Add(SchematicClass);

        SchematicPaths.Add(Entry.SchematicClassPath);

        EntryPrices.Add(Entry.PriceCredits);



        // ----------------------------------------------------

        // CARTE BOUTIQUE CONTRACTORY

        // Base visuelle inspiree de l'AWESOME Shop

        // ----------------------------------------------------



        UBorder* Card =

            WidgetTree->ConstructWidget<UBorder>(

                UBorder::StaticClass()

            );



        Card->SetPadding(FMargin(14.0f, 12.0f, 14.0f, 12.0f));



        Card->SetBrushColor(FLinearColor(0.133f, 0.176f, 0.212f, 1.0f));



        CatalogueScroll->AddChild(

            Card

        );



        // CONTRACTORY_BOUTIQUE_SEARCH_ENTRY_V2

        EntryCards.Add(

            Card

        );

        EntryCategories.Add(Entry.Category);



        EntrySearchTexts.Add(

            FString::Printf(

                TEXT("%s %s"),

                *Entry.Category,

                *Entry.SchematicClassPath

            )

        );



        UHorizontalBox* Row =

            WidgetTree->ConstructWidget<UHorizontalBox>(

                UHorizontalBox::StaticClass()

            );



        Card->AddChild(

            Row

        );



        // ----------------------------------------------------

        // ICONE REELLE DU SCHEMATIC MAM

        // ----------------------------------------------------



        USizeBox* IconSize =

            WidgetTree->ConstructWidget<USizeBox>(

                USizeBox::StaticClass()

            );



        IconSize->SetWidthOverride(72.0f);



        IconSize->SetHeightOverride(72.0f);



        UImage* IconImage =

            WidgetTree->ConstructWidget<UImage>(

                UImage::StaticClass()

            );



        IconSize->AddChild(

            IconImage

        );



        Row->AddChildToHorizontalBox(

            IconSize

        );



        USizeBox* IconGap =

            WidgetTree->ConstructWidget<USizeBox>(

                USizeBox::StaticClass()

            );



        IconGap->SetWidthOverride(

        10.0f

        );



        Row->AddChildToHorizontalBox(

            IconGap

        );



        // ----------------------------------------------------

        // INFORMATIONS

        // ----------------------------------------------------



        USizeBox* InformationSize =

            WidgetTree->ConstructWidget<USizeBox>(

                USizeBox::StaticClass()

            );



        InformationSize->SetWidthOverride(345.0f);



        UVerticalBox* Information =

            WidgetTree->ConstructWidget<UVerticalBox>(

                UVerticalBox::StaticClass()

            );



        InformationSize->AddChild(

            Information

        );



        Row->AddChildToHorizontalBox(

            InformationSize

        );



        // Nom de la recherche MAM



        UTextBlock* NameText =

            WidgetTree->ConstructWidget<UTextBlock>(

                UTextBlock::StaticClass()

            );



        NameText->SetAutoWrapText(

            true

        );



        Information->AddChildToVerticalBox(NameText);

        NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

        { FSlateFontInfo Font = NameText->GetFont(); Font.Size = 20; NameText->SetFont(Font); }



        // Description du schematic



        UTextBlock* DescriptionText =

            WidgetTree->ConstructWidget<UTextBlock>(

                UTextBlock::StaticClass()

            );



        DescriptionText->SetAutoWrapText(

            true

        );



        Information->AddChildToVerticalBox(DescriptionText);

        DescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.82f, 0.84f, 1.0f)));

        { FSlateFontInfo Font = DescriptionText->GetFont(); Font.Size = 13; DescriptionText->SetFont(Font); }



        // Prix / etat



        UTextBlock* StatusText =

            WidgetTree->ConstructWidget<UTextBlock>(

                UTextBlock::StaticClass()

            );



        StatusText->SetAutoWrapText(

            true

        );



        Information->AddChildToVerticalBox(StatusText);

        StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.949f, 0.549f, 0.157f, 1.0f)));

        { FSlateFontInfo Font = StatusText->GetFont(); Font.Size = 16; StatusText->SetFont(Font); }



        EntryStatusTexts.Add(

            StatusText

        );



        // ----------------------------------------------------

        // ESPACE AVANT BOUTON

        // ----------------------------------------------------



        USizeBox* ButtonGap =

            WidgetTree->ConstructWidget<USizeBox>(

                USizeBox::StaticClass()

            );



        ButtonGap->SetWidthOverride(

        10.0f

        );



        Row->AddChildToHorizontalBox(

            ButtonGap

        );



        // ----------------------------------------------------

        // BOUTON ACHETER

        // ----------------------------------------------------



        UContractoryBoutiqueBuyButton* BuyButton =

            WidgetTree->ConstructWidget<

                UContractoryBoutiqueBuyButton

            >(

                UContractoryBoutiqueBuyButton::StaticClass()

            );



        UTextBlock* BuyButtonText =

            WidgetTree->ConstructWidget<UTextBlock>(

                UTextBlock::StaticClass()

            );



        BuyButtonText->SetText(

            FText::FromString(

                TEXT("ACHETER")

            )

        );



        BuyButton->AddChild(BuyButtonText);

        BuyButton->SetBackgroundColor(FLinearColor(0.35f, 0.38f, 0.42f, 1.0f));

        BuyButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

        { FSlateFontInfo Font = BuyButtonText->GetFont(); Font.Size = 16; BuyButtonText->SetFont(Font); }



        Row->AddChildToHorizontalBox(

            BuyButton

        );



        BuyButtons.Add(

            BuyButton

        );



        EntryButtonTexts.Add(

            BuyButtonText

        );



        // ----------------------------------------------------

        // DONNEES VANILLA DU SCHEMATIC

        // ----------------------------------------------------



        if (bHoverpackItem)

        {

            // CONTRACTORY_HOVERPACK_CARD

            NameText->SetText(

                FText::FromString(TEXT("HOVERPACK"))

            );



            DescriptionText->SetText(

                FText::FromString(

                    TEXT("1 vrai Hoverpack directement dans l'inventaire.")

                )

            );



            TSubclassOf<UFGItemDescriptor> HoverPackDescriptor =

                LoadClass<UFGItemDescriptor>(

                    nullptr,

                    TEXT(

                        "/Game/FactoryGame/Resource/Equipment/HoverPack/"

                        "BP_EquipmentDescriptorHoverPack."

                        "BP_EquipmentDescriptorHoverPack_C"

                    )

                );



            if (HoverPackDescriptor)

            {

                if (UTexture2D* HoverPackIcon =

                    UFGItemDescriptor::GetSmallIcon(HoverPackDescriptor))

                {

                    IconImage->SetBrushFromTexture(HoverPackIcon,true);

                }

            }



            Card->SetToolTipText(

                FText::FromString(

                    TEXT("Objet reel : aucune recette Hoverpack n'est debloquee.")

                )

            );

        }

        else if (SchematicClass)

        {

            NameText->SetText(

                UFGSchematic::GetSchematicDisplayName(

                    SchematicClass

                )

            );



            DescriptionText->SetText(

                UFGSchematic::GetSchematicDescription(

                    SchematicClass

                )

            );



            const bool bRawOreResearch =

    Entry.SchematicClassPath.Contains(TEXT("Research_Caterium_0.")) ||

    Entry.SchematicClassPath.Contains(TEXT("Research_Quartz_0.")) ||

    Entry.SchematicClassPath.Contains(TEXT("Research_Sulfur_0."));



if (bRawOreResearch)

{

    UTexture2D* ScannerIcon = LoadObject<UTexture2D>(

        nullptr,

        TEXT("/SatisfactoryReverse/UI/Icons/T_Contractory_Scanner.T_Contractory_Scanner")

    );



    if (ScannerIcon)

    {

        IconImage->SetBrushFromTexture(ScannerIcon, true);

    }

    else

    {

        IconImage->SetBrush(UFGSchematic::GetItemIcon(SchematicClass));

    }

}

else

{

    IconImage->SetBrush(UFGSchematic::GetItemIcon(SchematicClass));

}



            Card->SetToolTipText(

                UFGSchematic::GetSchematicDescription(

                    SchematicClass

                )

            );

        }

        else

        {

            NameText->SetText(

                FText::FromString(

                    Entry.SchematicClassPath

                )

            );



            DescriptionText->SetText(

                FText::FromString(

                    TEXT("Schematic indisponible")

                )

            );

        }



        // CONTRACTORY_SEARCH_LOCALIZED_084

        // Ajouter le nom réellement affiché à la recherche.

        if (EntrySearchTexts.IsValidIndex(Index))

        {

            EntrySearchTexts[Index] += TEXT(" ");

            EntrySearchTexts[Index] += NameText->GetText().ToString();

        }



        StatusText->SetText(

            FText::FromString(

                FString::Printf(

                    TEXT("PRIX : %d CREDITS CONTRACTORY"),

                    Entry.PriceCredits

                )

            )

        );



        // Espace vertical entre deux cartes



        USizeBox* CardGap =

            WidgetTree->ConstructWidget<USizeBox>(

                USizeBox::StaticClass()

            );



        CardGap->SetHeightOverride(8.0f);



        CatalogueScroll->AddChild(

            CardGap

        );

    }

}





// ============================================================

// CONTRACTORY_BOUTIQUE_SEARCH_FUNCTION_V2

// ============================================================



void UContractoryBoutiqueWidget::HandleSearchChanged(

    const FText& SearchText

)

{

    FString Filter =

        SearchText.ToString();



    Filter.TrimStartAndEndInline();

    Filter.ToLowerInline();



    for (

        int32 Index = 0;

        Index < EntryCards.Num();

        ++Index

    )

    {

        UWidget* Card =

            EntryCards[Index];



        if (!Card)

        {

            continue;

        }



        if (Filter.IsEmpty())

        {

            Card->SetVisibility(

                ESlateVisibility::Visible

            );



            continue;

        }



        FString SearchableText;



        if (EntrySearchTexts.IsValidIndex(Index))

        {

            SearchableText =

                EntrySearchTexts[Index];

        }



        SearchableText.ToLowerInline();



        const bool bMatches =

            SearchableText.Contains(

                Filter,

                ESearchCase::IgnoreCase,

                ESearchDir::FromStart

            );



        Card->SetVisibility(

            bMatches

                ? ESlateVisibility::Visible

                : ESlateVisibility::Collapsed

        );

    }





    // CONTRACTORY_CATEGORY_FILTER_084

    for (int32 CategoryIndex = 0;

         CategoryIndex < CategoryTitles.Num();

         ++CategoryIndex)

    {

        bool bHasVisibleEntry = false;



        for (int32 EntryIndex = 0;

             EntryIndex < EntryCards.Num();

             ++EntryIndex)

        {

            if (EntryCategories.IsValidIndex(EntryIndex) &&

                CategoryNames.IsValidIndex(CategoryIndex) &&

                EntryCategories[EntryIndex] == CategoryNames[CategoryIndex] &&

                EntryCards[EntryIndex] &&

                EntryCards[EntryIndex]->GetVisibility() == ESlateVisibility::Visible)

            {

                bHasVisibleEntry = true;

                break;

            }

        }



        if (CategoryTitles[CategoryIndex])

        {

            CategoryTitles[CategoryIndex]->SetVisibility(

                bHasVisibleEntry

                    ? ESlateVisibility::Visible

                    : ESlateVisibility::Collapsed

            );

        }

    }



    if (CatalogueScroll)

    {

        CatalogueScroll->ScrollToStart();

    }

}



void UContractoryBoutiqueWidget::

BindMissionRemoteCallObject()

{

    AFGPlayerController* PlayerController =

        Cast<AFGPlayerController>(

            GetOwningPlayer()

        );



    if (!PlayerController)

    {

        return;

    }



    MissionRCO =

        Cast<UContractoryMissionRemoteCallObject>(

            PlayerController->GetRemoteCallObjectOfClass(

                UContractoryMissionRemoteCallObject::

                    StaticClass()

            )

        );



    if (!MissionRCO)

    {

        return;

    }



    MissionRCO->OnMissionStateUpdated.RemoveAll(this);

    MissionRCO->OnBoutiquePurchaseResult.RemoveAll(this);



    MissionRCO->OnMissionStateUpdated.AddUObject(

        this,

        &UContractoryBoutiqueWidget::

            RefreshBoutiqueInterface

    );



    MissionRCO->OnBoutiquePurchaseResult.AddUObject(

        this,

        &UContractoryBoutiqueWidget::

            HandleBoutiquePurchaseResult

    );



    for (int32 Index = 0;

         Index < BuyButtons.Num();

         ++Index)

    {

        if (!BuyButtons.IsValidIndex(Index) ||

            !SchematicPaths.IsValidIndex(Index))

        {

            continue;

        }



        if (BuyButtons[Index])

        {

            BuyButtons[Index]->Configure(

                MissionRCO,

                SchematicPaths[Index]

            );

        }

    }



    MissionRCO->Server_RequestMissionState();

}



void UContractoryBoutiqueWidget::

PollBoutiqueState()

{

    if (!MissionRCO)

    {

        BindMissionRemoteCallObject();

    }



    if (MissionRCO)

    {

        MissionRCO->Server_RequestMissionState();

    }



    RefreshBoutiqueInterface();

}



void UContractoryBoutiqueWidget::

RefreshBoutiqueInterface()

{

    const int32 Credits =

        MissionRCO

            ? MissionRCO->GetCachedCredits()

            : 0;



    if (CreditsText)

    {

        CreditsText->SetText(

            FText::FromString(

                FString::Printf(

                    TEXT("CREDITS FICSIT : %d"),

                    Credits

                )

            )

        );

    }



    AFGSchematicManager* SchematicManager =

        AFGSchematicManager::Get(

            GetWorld()

        );



    for (int32 Index = 0;

         Index < SchematicClasses.Num();

         ++Index)

    {

        if (!EntryStatusTexts.IsValidIndex(Index) ||

            !EntryButtonTexts.IsValidIndex(Index) ||

            !BuyButtons.IsValidIndex(Index) ||

            !EntryPrices.IsValidIndex(Index))

        {

            continue;

        }



        UTextBlock* StatusText =

            EntryStatusTexts[Index];



        UTextBlock* ButtonText =

            EntryButtonTexts[Index];



        UContractoryBoutiqueBuyButton* BuyButton =

            BuyButtons[Index];



        const int32 Price =

            EntryPrices[Index];



        TSubclassOf<UFGSchematic> SchematicClass =

            SchematicClasses[Index];



        if (!StatusText ||

            !ButtonText ||

            !BuyButton)

        {

            continue;

        }



                // CONTRACTORY_HOVERPACK_UI_REFRESH

        const bool bHoverpackItem =

            SchematicPaths.IsValidIndex(Index) &&

            SchematicPaths[Index] ==

                TEXT("CONTRACTORY_ITEM_HOVERPACK");



        if (bHoverpackItem)
        {
            // CONTRACTORY_HOVERPACK_UI_TIER_100
            if (!SchematicManager ||
                SchematicManager->GetHighestAvailableTechTier() < 5)
            {
                StatusText->SetText(FText::FromString(TEXT("DEBLOCAGE : PALIER 5")));
                ButtonText->SetText(FText::FromString(TEXT("VERROUILLE")));
                BuyButton->SetIsEnabled(false);
                continue;
            }

            if (!MissionRCO)
            {

                StatusText->SetText(

                    FText::FromString(

                        FString::Printf(

                            TEXT("Prix : %d | INDISPONIBLE"),

                            Price

                        )

                    )

                );

                ButtonText->SetText(

                    FText::FromString(TEXT("INDISPONIBLE"))

                );

                BuyButton->SetIsEnabled(false);

                continue;

            }



            if (Credits < Price)

            {

                StatusText->SetText(

                    FText::FromString(

                        FString::Printf(

                            TEXT("Prix : %d | Credits insuffisants"),

                            Price

                        )

                    )

                );

                ButtonText->SetText(

                    FText::FromString(TEXT("CREDITS"))

                );

                BuyButton->SetIsEnabled(false);

                continue;

            }



            StatusText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT("Prix : %d Credits FICSIT | OBJET REEL"),

                        Price

                    )

                )

            );

            ButtonText->SetText(

                FText::FromString(TEXT("ACHETER"))

            );

            BuyButton->SetIsEnabled(true);

            continue;

        }



        if (!MissionRCO ||

            !SchematicManager ||

            !SchematicClass)

        {

            StatusText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT("Prix : %d | INDISPONIBLE"),

                        Price

                    )

                )

            );



            ButtonText->SetText(

                FText::FromString(

                    TEXT("INDISPONIBLE")

                )

            );



            BuyButton->SetIsEnabled(false);

            continue;

        }



        const bool bPurchased =

            SchematicManager->IsSchematicPurchased(

                SchematicClass

            );



        if (bPurchased)

        {

            StatusText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT("Prix : %d | ACHETE"),

                        Price

                    )

                )

            );



            ButtonText->SetText(

                FText::FromString(

                    TEXT("ACHETE")

                )

            );



            BuyButton->SetIsEnabled(false);

            continue;

        }



        const bool bDependenciesMet =

            UFGSchematic::CanGiveAccessToSchematic(

                SchematicClass,

                GetWorld()

            );



        if (!bDependenciesMet)

        {

            StatusText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT("Prix : %d | VERROUILLE"),

                        Price

                    )

                )

            );



            ButtonText->SetText(

                FText::FromString(

                    TEXT("VERROUILLE")

                )

            );



            BuyButton->SetIsEnabled(false);

            continue;

        }



        if (Credits < Price)

        {

            StatusText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT(

                            "Prix : %d | Credits insuffisants"

                        ),

                        Price

                    )

                )

            );



            ButtonText->SetText(

                FText::FromString(

                    TEXT("CREDITS")

                )

            );



            BuyButton->SetIsEnabled(false);

            continue;

        }



        StatusText->SetText(

            FText::FromString(

                FString::Printf(

                    TEXT("Prix : %d Credits FICSIT"),

                    Price

                )

            )

        );



        ButtonText->SetText(

            FText::FromString(

                TEXT("ACHETER")

            )

        );



        BuyButton->SetIsEnabled(true);

    }

}



void UContractoryBoutiqueWidget::

HandleBoutiquePurchaseResult(

    bool bSuccess,

    int32 RemainingCredits,

    const FString& SchematicClassPath,

    const FString& FailureReason

)

{

    if (ResultText)

    {

        if (bSuccess)

        {

            TSubclassOf<UFGSchematic> PurchasedClass =

                LoadClass<UFGSchematic>(

                    nullptr,

                    *SchematicClassPath

                );



                        // CONTRACTORY_HOVERPACK_RESULT_NAME

            const FString DisplayName =

                SchematicClassPath ==

                    TEXT("CONTRACTORY_ITEM_HOVERPACK")

                ? TEXT("HOVERPACK")

                : (

                    PurchasedClass

                        ? UFGSchematic::GetSchematicDisplayName(

                            PurchasedClass

                          ).ToString()

                        : SchematicClassPath

                  );



            ResultText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT(

                            "ACHAT VALIDE : %s | Credits restants : %d"

                        ),

                        *DisplayName,

                        RemainingCredits

                    )

                )

            );

        }

        else

        {

            ResultText->SetText(

                FText::FromString(

                    FString::Printf(

                        TEXT(

                            "ACHAT REFUSE : %s"

                        ),

                        *FailureReason

                    )

                )

            );

        }

    }



    RefreshBoutiqueInterface();

}







