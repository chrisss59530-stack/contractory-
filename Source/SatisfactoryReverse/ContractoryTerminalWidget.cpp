#include "ContractoryTerminalWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Resources/FGItemDescriptor.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

#include "ContractoryPoweredBuildable.h"
#include "ContractoryTerminalAnimatorComponent.h"
#include "ContractoryMissionRemoteCallObject.h"
#include "ContractorySecondaryMissions.h"

#include "FGPlayerController.h"

#include "Components/ChildActorComponent.h"
#include "GameFramework/Actor.h"

#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"


// CONTRACTORY_082_ICONS_ONLY: explicit texture fallback for recycled resources.
static UTexture2D* GetContractoryRecycleIcon(const FString& ItemKey)
{
    if (ItemKey.IsEmpty())
    {
        return nullptr;
    }

    const FString DescriptorPath = FString::Printf(
        TEXT("/SatisfactoryReverse/Recyclage/%s.%s_C"),
        *ItemKey, *ItemKey);

    UClass* DescriptorClass = LoadClass<UFGItemDescriptor>(nullptr, *DescriptorPath);
    if (DescriptorClass)
    {
        if (UTexture2D* Icon = UFGItemDescriptor::GetSmallIcon(DescriptorClass))
        {
            return Icon;
        }
    }

    // Only Contractory recycled material descriptors are eligible for this fallback.
    static const FString Prefix(TEXT("Desc_Recyclage_FICSIT_Tier"));
    if (!ItemKey.StartsWith(Prefix))
    {
        return nullptr;
    }

    const FString TierString = ItemKey.RightChop(Prefix.Len());
    if (TierString.IsEmpty() || !TierString.IsNumeric())
    {
        return nullptr;
    }

    const int32 Tier = FCString::Atoi(*TierString);
    if (Tier < 1 || Tier > 7)
    {
        return nullptr;
    }

    const FString IconPath = FString::Printf(
        TEXT("/SatisfactoryReverse/icons/Icon_Recyclage_FICSIT_Tier%d_64.Icon_Recyclage_FICSIT_Tier%d_64"),
        Tier, Tier);

    UTexture2D* Icon = LoadObject<UTexture2D>(nullptr, *IconPath);
    if (!Icon)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[Contractory Terminal UI] Missing recycled material icon: %s"),
            *IconPath);
    }
    return Icon;
}

TSharedRef<SWidget> UContractoryTerminalWidget::RebuildWidget()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] RebuildWidget ENTER Root=%s Offers=%d Active=%d"),
        (WidgetTree && WidgetTree->RootWidget) ? TEXT("OK") : TEXT("NULL"),
        OfferTexts.Num(),
        ActiveMissionTexts.Num()
    );

    if (WidgetTree &&
        (!WidgetTree->RootWidget ||
         OfferTexts.Num() != 5 ||
         ActiveMissionTexts.Num() != 3))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory Terminal UI] RebuildWidget -> BuildTerminalInterface BEFORE Slate")
        );

        BuildTerminalInterface();
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] RebuildWidget -> Super Root=%s"),
        (WidgetTree && WidgetTree->RootWidget) ? TEXT("OK") : TEXT("NULL")
    );

    return Super::RebuildWidget();
}

void UContractoryTerminalWidget::NativeConstruct()
{
    UE_LOG(LogTemp, Warning, TEXT("[Contractory Terminal UI] NativeConstruct ENTER"));

    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("[Contractory Terminal UI] Super::NativeConstruct OK"));

    SetTerminalAnimationOpen(true);

    if (!WidgetTree)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] WidgetTree=%s Root=%s Offers=%d Active=%d"),
        WidgetTree ? TEXT("OK") : TEXT("NULL"),
        (WidgetTree && WidgetTree->RootWidget) ? TEXT("OK") : TEXT("NULL"),
        OfferTexts.Num(),
        ActiveMissionTexts.Num()
    );

    if (!WidgetTree->RootWidget ||
        OfferTexts.Num() != 5 ||
        ActiveMissionTexts.Num() != 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Contractory Terminal UI] BuildTerminalInterface CALL"));
        BuildTerminalInterface();
        UE_LOG(LogTemp, Warning, TEXT("[Contractory Terminal UI] BuildTerminalInterface RETURN"));
    }

    if (!IsTerminalPowered())
    {
        if (CreditsText)
        {
            CreditsText->SetText(
                FText::FromString(
                    TEXT("TERMINAL HORS TENSION")
                )
            );
        }

        for (UButton* Button : OfferButtons)
        {
            if (Button)
            {
                Button->SetIsEnabled(false);
            }
        }

        for (UButton* Button : ActiveClaimButtons)
        {
            if (Button)
            {
                Button->SetIsEnabled(false);
            }
        }

        return;
    }

    BindMissionRemoteCallObject();

    // Mise a jour automatique pendant que le Terminal reste ouvert.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MissionStatePollTimer,
            this,
            &UContractoryTerminalWidget::PollMissionState,
            0.5f,
            true,
            0.5f
        );
    }
}


void UContractoryTerminalWidget::Init_Implementation()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] Init_Implementation ENTER")
    );

    Super::Init_Implementation();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] Init_Implementation SUPER OK")
    );
}


void UContractoryTerminalWidget::SetInputMode_Implementation()
{
    Super::SetInputMode_Implementation();

    APlayerController* PlayerController = GetOwningPlayer();

    if (!PlayerController)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory Terminal UI] SetInputMode : PlayerController NULL")
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
        TEXT("[Contractory Terminal UI] SOURIS ACTIVE / CAMERA BLOQUEE")
    );
}

void UContractoryTerminalWidget::OnPushedToGameUI_Implementation()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] OnPushedToGameUI ENTER")
    );

    Super::OnPushedToGameUI_Implementation();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] OnPushedToGameUI SUPER OK")
    );
}
bool UContractoryTerminalWidget::NativeCanCallInit()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] NativeCanCallInit -> TRUE")
    );

    return true;
}
void UContractoryTerminalWidget::NativeDestruct()
{
    if (MissionRCO)
    {
        MissionRCO->OnMissionStateUpdated.RemoveAll(this);
    }

    MissionRCO = nullptr;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            MissionStatePollTimer
        );
    }

    SetTerminalAnimationOpen(false);

    Super::NativeDestruct();
}

bool UContractoryTerminalWidget::IsTerminalPowered() const
{
    const AContractoryPoweredBuildable* Terminal =
        Cast<AContractoryPoweredBuildable>(
            mInteractObject
        );

    return Terminal &&
        Terminal->HasContractoryPower();
}
void UContractoryTerminalWidget::BindMissionRemoteCallObject()
{
    AFGPlayerController* PlayerController =
        Cast<AFGPlayerController>(GetOwningPlayer());

    if (!PlayerController)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory Terminal] Owning AFGPlayerController introuvable.")
        );

        return;
    }

    MissionRCO =
        Cast<UContractoryMissionRemoteCallObject>(
            PlayerController->GetRemoteCallObjectOfClass(
                UContractoryMissionRemoteCallObject::StaticClass()
            )
        );

    if (!MissionRCO)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory Terminal] Mission RCO introuvable.")
        );

        return;
    }

    MissionRCO->OnMissionStateUpdated.RemoveAll(this);

    MissionRCO->OnMissionStateUpdated.AddUObject(
        this,
        &UContractoryTerminalWidget::RefreshMissionInterface
    );

    // Affiche immediatement le cache existant.
    RefreshMissionInterface();

    // Puis demande au serveur l'etat reel le plus recent.
    MissionRCO->Server_RequestMissionState();
}

void UContractoryTerminalWidget::PollMissionState()
{
    if (
        MissionRCO &&
        IsTerminalPowered()
    )
    {
        MissionRCO->Server_RequestMissionState();
    }
}

void UContractoryTerminalWidget::RefreshMissionInterface()
{
    if (!MissionRCO)
    {
        return;
    }

    const int32 Credits =
        MissionRCO->GetCachedCredits();

    const TArray<FContractoryMissionDefinition>& Offers =
        MissionRCO->GetCachedOffers();

    const TArray<FContractoryActiveMission>& ActiveMissions =
        MissionRCO->GetCachedActiveMissions();

    if (CreditsText)
    {
        CreditsText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("CREDITS CONTRACTORY : %d"),
                    Credits
                )
            )
        );
    }

    if (ActiveTitleText)
    {
        ActiveTitleText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("MISSIONS ACTIVES %d / 3"),
                    ActiveMissions.Num()
                )
            )
        );
    }

    auto BuildRequirementList =
        [](
            const FContractoryMissionDefinition& Definition,
            const FContractoryActiveMission* Active
        ) -> FString
    {
        FString Text;

        for (
            int32 i = 0;
            i < Definition.Requirements.Num();
            ++i
        )
        {
            const FContractoryMissionRequirement& R =
                Definition.Requirements[i];

            if (!Text.IsEmpty())
            {
                Text += TEXT(" | ");
            }

            if (Active)
            {
                const int32 Delivered =
                    UContractoryMissionManager::
                        GetDeliveredForRequirement(
                            *Active,
                            i
                        );

                Text += FString::Printf(
                    TEXT("%d/%d"),
                    Delivered,
                    R.TargetAmount
                );
            }
            else
            {
                Text += FString::Printf(
                    TEXT("x%d"),
                    R.TargetAmount
                );
            }
        }

        return Text;
    };

    const bool bThreeSlotsUsed =
        ActiveMissions.Num() >= 3;

    for (int32 Index = 0; Index < 5; ++Index)
    {
        UTextBlock* MissionText =
            OfferTexts.IsValidIndex(Index)
                ? OfferTexts[Index]
                : nullptr;

        UButton* AcceptButton =
            OfferButtons.IsValidIndex(Index)
                ? OfferButtons[Index]
                : nullptr;

        UImage* MissionIcon =
            OfferIcons.IsValidIndex(Index)
                ? OfferIcons[Index]
                : nullptr;

        if (!Offers.IsValidIndex(Index))
        {
            if (MissionText)
            {
                MissionText->SetText(
                    FText::FromString(
                        TEXT("Aucune mission disponible")
                    )
                );
            }

            if (AcceptButton)
            {
                AcceptButton->SetIsEnabled(false);
            }

            if (MissionIcon)
            {
                MissionIcon->SetVisibility(
                    ESlateVisibility::Collapsed
                );
            }

            continue;
        }

        const FContractoryMissionDefinition& Mission =
            Offers[Index];

        if (MissionIcon)
        {
            const FString FirstItem =
                !Mission.Requirements.IsEmpty()
                    ? Mission.Requirements[0].ItemKey
                    : Mission.TargetItemKey;

            if (
                UTexture2D* IconTexture =
                    GetContractoryRecycleIcon(FirstItem)
            )
            {
                MissionIcon->SetBrushFromTexture(
                    IconTexture,
                    true
                );

                MissionIcon->SetVisibility(
                    ESlateVisibility::Visible
                );
            }
            else
            {
                MissionIcon->SetVisibility(
                    ESlateVisibility::Collapsed
                );
            }
        }

        if (MissionText)
        {
            MissionText->SetText(
                FText::FromString(
                    FString::Printf(
                        TEXT("%s | %s | +%d credits"),
                        *Mission.Title.ToString(),
                        *BuildRequirementList(
                            Mission,
                            nullptr
                        ),
                        Mission.RewardCredits
                    )
                )
            );
        }

        const bool bAlreadyActive =
            ActiveMissions.ContainsByPredicate(
                [&Mission](
                    const FContractoryActiveMission& ActiveMission
                )
                {
                    return ActiveMission.MissionId ==
                        Mission.MissionId;
                }
            );

        if (AcceptButton)
        {
            AcceptButton->SetIsEnabled(
                !bThreeSlotsUsed &&
                !bAlreadyActive
            );
        }
    }

    const TArray<FContractoryMissionDefinition> Catalogue =
        UContractorySecondaryMissions::GetMissionCatalogue();

    for (int32 Index = 0; Index < 3; ++Index)
    {
        UTextBlock* ActiveText =
            ActiveMissionTexts.IsValidIndex(Index)
                ? ActiveMissionTexts[Index]
                : nullptr;

        UButton* ClaimButton =
            ActiveClaimButtons.IsValidIndex(Index)
                ? ActiveClaimButtons[Index]
                : nullptr;

        UImage* ActiveIcon =
            ActiveMissionIcons.IsValidIndex(Index)
                ? ActiveMissionIcons[Index]
                : nullptr;

        if (!ActiveText)
        {
            continue;
        }

        if (!ActiveMissions.IsValidIndex(Index))
        {
            if (ActiveIcon)
            {
                ActiveIcon->SetVisibility(
                    ESlateVisibility::Collapsed
                );
            }

            ActiveText->SetText(
                FText::FromString(
                    FString::Printf(
                        TEXT("EMPLACEMENT %d : LIBRE"),
                        Index + 1
                    )
                )
            );

            if (ClaimButton)
            {
                ClaimButton->SetIsEnabled(false);
            }

            continue;
        }

        const FContractoryActiveMission& ActiveMission =
            ActiveMissions[Index];

        const FContractoryMissionDefinition* Definition =
            Catalogue.FindByPredicate(
                [&ActiveMission](
                    const FContractoryMissionDefinition& Mission
                )
                {
                    return Mission.MissionId ==
                        ActiveMission.MissionId;
                }
            );

        if (!Definition)
        {
            if (ActiveIcon)
            {
                ActiveIcon->SetVisibility(
                    ESlateVisibility::Collapsed
                );
            }

            ActiveText->SetText(
                FText::FromString(
                    FString::Printf(
                        TEXT("Mission %d"),
                        ActiveMission.MissionId
                    )
                )
            );

            if (ClaimButton)
            {
                ClaimButton->SetIsEnabled(false);
            }

            continue;
        }

        if (ActiveIcon)
        {
            const FString FirstItem =
                !Definition->Requirements.IsEmpty()
                    ? Definition->Requirements[0].ItemKey
                    : Definition->TargetItemKey;

            if (
                UTexture2D* IconTexture =
                    GetContractoryRecycleIcon(FirstItem)
            )
            {
                ActiveIcon->SetBrushFromTexture(
                    IconTexture,
                    true
                );

                ActiveIcon->SetVisibility(
                    ESlateVisibility::Visible
                );
            }
            else
            {
                ActiveIcon->SetVisibility(
                    ESlateVisibility::Collapsed
                );
            }
        }

        ActiveText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%s | %s | recompense %d credits"),
                    *Definition->Title.ToString(),
                    *BuildRequirementList(
                        *Definition,
                        &ActiveMission
                    ),
                    Definition->RewardCredits
                )
            )
        );

        if (ClaimButton)
        {
            const bool bReadyToClaim =
                UContractoryMissionManager::
                    IsMissionComplete(
                        ActiveMission,
                        *Definition
                    );

            ClaimButton->SetIsEnabled(
                bReadyToClaim &&
                IsTerminalPowered()
            );
        }
    }
}
void UContractoryTerminalWidget::AcceptOfferByIndex(
    int32 OfferIndex
)
{
    if (!MissionRCO || !IsTerminalPowered())
    {
        return;
    }

    const TArray<FContractoryMissionDefinition>& Offers =
        MissionRCO->GetCachedOffers();

    if (!Offers.IsValidIndex(OfferIndex))
    {
        return;
    }

    MissionRCO->Server_AcceptMission(
        Offers[OfferIndex].MissionId
    );
}

void UContractoryTerminalWidget::AcceptOffer0()
{
    AcceptOfferByIndex(0);
}

void UContractoryTerminalWidget::AcceptOffer1()
{
    AcceptOfferByIndex(1);
}

void UContractoryTerminalWidget::AcceptOffer2()
{
    AcceptOfferByIndex(2);
}

void UContractoryTerminalWidget::AcceptOffer3()
{
    AcceptOfferByIndex(3);
}

void UContractoryTerminalWidget::AcceptOffer4()
{
    AcceptOfferByIndex(4);
}

void UContractoryTerminalWidget::ClaimActiveByIndex(
    int32 ActiveIndex
)
{
    if (!MissionRCO || !IsTerminalPowered())
    {
        return;
    }

    const TArray<FContractoryActiveMission>& ActiveMissions =
        MissionRCO->GetCachedActiveMissions();

    if (!ActiveMissions.IsValidIndex(ActiveIndex))
    {
        return;
    }

    MissionRCO->Server_ClaimMission(
        ActiveMissions[ActiveIndex].MissionId
    );
}

void UContractoryTerminalWidget::ClaimActive0()
{
    ClaimActiveByIndex(0);
}

void UContractoryTerminalWidget::ClaimActive1()
{
    ClaimActiveByIndex(1);
}

void UContractoryTerminalWidget::ClaimActive2()
{
    ClaimActiveByIndex(2);
}

void UContractoryTerminalWidget::RefreshOffersClicked()
{
    if (MissionRCO && IsTerminalPowered())
    {
        MissionRCO->Server_RefreshOffers();
    }
}

void UContractoryTerminalWidget::SetTerminalAnimationOpen(
    bool bOpen
)
{
    AContractoryPoweredBuildable* Terminal =
        Cast<AContractoryPoweredBuildable>(
            mInteractObject
        );

    if (!Terminal)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[Contractory Terminal] mInteractObject n'est pas un ContractoryPoweredBuildable"
            )
        );

        return;
    }

    TArray<UChildActorComponent*> ChildActorComponents;

    Terminal->GetComponents<UChildActorComponent>(
        ChildActorComponents
    );

    for (
        UChildActorComponent* ChildActorComponent :
        ChildActorComponents
    )
    {
        if (!ChildActorComponent)
        {
            continue;
        }

        AActor* ChildActor =
            ChildActorComponent->GetChildActor();

        if (!ChildActor)
        {
            continue;
        }

        UContractoryTerminalAnimatorComponent* Animator =
            ChildActor->FindComponentByClass<
                UContractoryTerminalAnimatorComponent
            >();

        if (Animator)
        {
            Animator->SetTerminalOpen(bOpen);

            UE_LOG(
                LogTemp,
                Log,
                TEXT(
                    "[Contractory Terminal] Animation widget = %s"
                ),
                bOpen
                    ? TEXT("OUVERTE")
                    : TEXT("FERMEE")
            );

            return;
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[Contractory Terminal] Animateur introuvable sur le ChildActor"
        )
    );
}

void UContractoryTerminalWidget::BuildTerminalInterface()
{
    UE_LOG(LogTemp, Warning, TEXT("[Contractory Terminal UI] BuildTerminalInterface ENTER"));
    // CONTRACTORY_UI_080_PHASE1 - Habillage uniquement, logique conservee.
    const FString TextureBase = TEXT("/SatisfactoryReverse/UI/Textures/Contractory/");
    auto FindUITexture = [&](const TCHAR* Name) -> UTexture2D*
    {
        const FString AssetName(Name);
        const FString Path = TextureBase + AssetName + TEXT(".") + AssetName;
        return LoadObject<UTexture2D>(nullptr, *Path);
    };
    UTexture2D* PanelTexture = FindUITexture(TEXT("Panel_Main"));
    UTexture2D* CardTexture = FindUITexture(TEXT("Mission_Card"));
    UTexture2D* NormalTexture = FindUITexture(TEXT("Button_Normal"));
    UTexture2D* HoverTexture = FindUITexture(TEXT("Button_Hovered"));
    UTexture2D* PressedTexture = FindUITexture(TEXT("Button_Pressed"));

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory Terminal UI] Textures Panel=%s Card=%s Normal=%s Hover=%s Pressed=%s"),
        PanelTexture ? TEXT("OK") : TEXT("NULL"),
        CardTexture ? TEXT("OK") : TEXT("NULL"),
        NormalTexture ? TEXT("OK") : TEXT("NULL"),
        HoverTexture ? TEXT("OK") : TEXT("NULL"),
        PressedTexture ? TEXT("OK") : TEXT("NULL")
    );
    auto MakeBrush = [](UTexture2D* Texture) -> FSlateBrush
    {
        FSlateBrush Brush;
        if (Texture)
        {
            Brush.SetResourceObject(Texture);
            Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
            Brush.DrawAs = ESlateBrushDrawType::Box;
            Brush.Margin = FMargin(0.12f);
        }
        return Brush;
    };
    auto StyleButton = [&](UButton* Button)
    {
        if (!Button) return;
        FButtonStyle Style = Button->GetStyle();
        if (NormalTexture) Style.SetNormal(MakeBrush(NormalTexture));
        if (HoverTexture) Style.SetHovered(MakeBrush(HoverTexture));
        if (PressedTexture) Style.SetPressed(MakeBrush(PressedTexture));
        Button->SetStyle(Style);
    };
    auto WrapCard = [&](UWidget* Child, const FName& Name) -> UBorder*
    {
        UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
        Border->SetPadding(FMargin(12.0f, 7.0f));
        if (CardTexture) Border->SetBrush(MakeBrush(CardTexture));
        Border->SetContent(Child);
        return Border;
    };
    auto SetTextSize = [](UTextBlock* Text, int32 Size)
    {
        if (!Text) return;

        FSlateFontInfo Font = Text->GetFont();
        Font.Size = Size;
        Text->SetFont(Font);
    };

    OfferTexts.Reset();
    OfferIcons.Reset();
    OfferButtons.Reset();
    ActiveMissionTexts.Reset();
    ActiveMissionIcons.Reset();
    ActiveClaimButtons.Reset();

    MainBox =
        WidgetTree->ConstructWidget<UVerticalBox>(
            UVerticalBox::StaticClass(),
            TEXT("ContractoryMainBox")
        );

    // TEST ROOT 0.8.0 : conservation de la racine historique du Terminal.
    // Le fond Panel_Main sera remis autrement apres validation de l'ouverture.
    WidgetTree->RootWidget = MainBox;

    UTextBlock* Title =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("ContractoryTitle")
        );

    Title->SetText(
        FText::FromString(
            TEXT("CONTRACTORY")
        )
    );

    SetTextSize(Title, 24);
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.48f, 0.07f, 1.0f)));
    MainBox->AddChildToVerticalBox(Title);

    CreditsText =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("ContractoryCredits")
        );

    CreditsText->SetText(
        FText::FromString(
            TEXT("CREDITS CONTRACTORY : chargement...")
        )
    );

    SetTextSize(CreditsText, 18);

    MainBox->AddChildToVerticalBox(
        CreditsText
    );

    UTextBlock* OffersTitle =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("OffersTitle")
        );

    OffersTitle->SetText(
        FText::FromString(
            TEXT("MISSIONS DISPONIBLES")
        )
    );

    SetTextSize(OffersTitle, 18);
    OffersTitle->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.48f, 0.07f, 1.0f)));
    MainBox->AddChildToVerticalBox(
        OffersTitle
    );

    for (int32 Index = 0; Index < 5; ++Index)
    {
        UHorizontalBox* MissionRow =
            WidgetTree->ConstructWidget<UHorizontalBox>(
                UHorizontalBox::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("MissionRow_%d"),
                        Index
                    )
                )
            );

        UImage* MissionIcon =
            WidgetTree->ConstructWidget<UImage>(
                UImage::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("MissionIcon_%d"),
                        Index
                    )
                )
            );

        USizeBox* MissionIconBox =
            WidgetTree->ConstructWidget<USizeBox>(
                USizeBox::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("MissionIconBox_%d"),
                        Index
                    )
                )
            );

        MissionIconBox->SetWidthOverride(40.0f);
        MissionIconBox->SetHeightOverride(40.0f);
        MissionIconBox->AddChild(MissionIcon);

        UTextBlock* MissionText =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("MissionText_%d"),
                        Index
                    )
                )
            );

        MissionText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT(
                        "Mission %d - chargement..."
                    ),
                    Index + 1
                )
            )
        );

        UButton* AcceptButton =
            WidgetTree->ConstructWidget<UButton>(
                UButton::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("AcceptButton_%d"),
                        Index
                    )
                )
            );

        UTextBlock* ButtonText =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass()
            );

        ButtonText->SetText(
            FText::FromString(
                TEXT("ACCEPTER")
            )
        );

        AcceptButton->AddChild(ButtonText);
        StyleButton(AcceptButton);
        SetTextSize(ButtonText, 16);
        SetTextSize(MissionText, 16);

        ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        MissionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

        switch (Index)
        {
            case 0:
                AcceptButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::AcceptOffer0
                );
                break;

            case 1:
                AcceptButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::AcceptOffer1
                );
                break;

            case 2:
                AcceptButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::AcceptOffer2
                );
                break;

            case 3:
                AcceptButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::AcceptOffer3
                );
                break;

            case 4:
                AcceptButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::AcceptOffer4
                );
                break;

            default:
                break;
        }

        OfferTexts.Add(MissionText);
        OfferIcons.Add(MissionIcon);
        OfferButtons.Add(AcceptButton);

        MissionRow->AddChildToHorizontalBox(
            MissionIconBox
        );

        MissionRow->AddChildToHorizontalBox(
            MissionText
        );

        MissionRow->AddChildToHorizontalBox(
            AcceptButton
        );

        MainBox->AddChildToVerticalBox(
            WrapCard(MissionRow, FName(*FString::Printf(TEXT("MissionCard_%d"), Index)))
        );
    }

    UButton* RefreshOffersButton =
        WidgetTree->ConstructWidget<UButton>(
            UButton::StaticClass(),
            TEXT("RefreshOffersButton")
        );

    UTextBlock* RefreshOffersText =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass()
        );

    RefreshOffersText->SetText(
        FText::FromString(
            TEXT("RENOUVELER LES OFFRES")
        )
    );

    SetTextSize(RefreshOffersText, 16);
    StyleButton(RefreshOffersButton);
    RefreshOffersButton->AddChild(
        RefreshOffersText
    );

    RefreshOffersButton->OnClicked.AddDynamic(
        this,
        &UContractoryTerminalWidget::RefreshOffersClicked
    );

    MainBox->AddChildToVerticalBox(
        RefreshOffersButton
    );

    ActiveTitleText =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("ActiveTitle")
        );

    ActiveTitleText->SetText(
        FText::FromString(
            TEXT("MISSIONS ACTIVES 0 / 3")
        )
    );

    SetTextSize(ActiveTitleText, 18);
    ActiveTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.48f, 0.07f, 1.0f)));
    MainBox->AddChildToVerticalBox(
        ActiveTitleText
    );

    for (int32 Index = 0; Index < 3; ++Index)
    {
        UHorizontalBox* ActiveRow =
            WidgetTree->ConstructWidget<UHorizontalBox>(
                UHorizontalBox::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("ActiveRow_%d"),
                        Index
                    )
                )
            );

        UImage* ActiveMissionIcon =
            WidgetTree->ConstructWidget<UImage>(
                UImage::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("ActiveMissionIcon_%d"),
                        Index
                    )
                )
            );

        USizeBox* ActiveMissionIconBox =
            WidgetTree->ConstructWidget<USizeBox>(
                USizeBox::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("ActiveMissionIconBox_%d"),
                        Index
                    )
                )
            );

        ActiveMissionIconBox->SetWidthOverride(40.0f);
        ActiveMissionIconBox->SetHeightOverride(40.0f);
        ActiveMissionIconBox->AddChild(
            ActiveMissionIcon
        );

        UTextBlock* ActiveMission =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("ActiveMission_%d"),
                        Index
                    )
                )
            );

        ActiveMission->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("EMPLACEMENT %d : LIBRE"),
                    Index + 1
                )
            )
        );

        UButton* ClaimButton =
            WidgetTree->ConstructWidget<UButton>(
                UButton::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT("ClaimButton_%d"),
                        Index
                    )
                )
            );

        UTextBlock* ClaimButtonText =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass()
            );

        ClaimButtonText->SetText(
            FText::FromString(
                TEXT("RECLAMER")
            )
        );

        SetTextSize(ActiveMission, 16);
        SetTextSize(ClaimButtonText, 16);

        StyleButton(ClaimButton);
        ClaimButton->AddChild(
            ClaimButtonText
        );

        ClaimButton->SetIsEnabled(false);

        switch (Index)
        {
            case 0:
                ClaimButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::ClaimActive0
                );
                break;

            case 1:
                ClaimButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::ClaimActive1
                );
                break;

            case 2:
                ClaimButton->OnClicked.AddDynamic(
                    this,
                    &UContractoryTerminalWidget::ClaimActive2
                );
                break;

            default:
                break;
        }

        ActiveMissionTexts.Add(
            ActiveMission
        );

        ActiveMissionIcons.Add(
            ActiveMissionIcon
        );

        ActiveClaimButtons.Add(
            ClaimButton
        );

        ActiveRow->AddChildToHorizontalBox(
            ActiveMissionIconBox
        );

        ActiveRow->AddChildToHorizontalBox(
            ActiveMission
        );

        ActiveRow->AddChildToHorizontalBox(
            ClaimButton
        );

        ActiveMission->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        MainBox->AddChildToVerticalBox(
            WrapCard(ActiveRow, FName(*FString::Printf(TEXT("ActiveCard_%d"), Index)))
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("[Contractory Terminal UI] BuildTerminalInterface END"));
}
