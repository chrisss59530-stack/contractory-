#include "ContractoryMissionHUDWidget.h"

#include "ContractoryMissionRemoteCallObject.h"
#include "ContractorySecondaryMissions.h"

#include "FGPlayerController.h"
#include "Resources/FGItemDescriptor.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"

#include "Engine/Texture2D.h"

static void ContractorySetFontSize(
    UTextBlock* Text,
    int32 Size
)
{
    if (!Text)
    {
        return;
    }

    FSlateFontInfo Font = Text->GetFont();
    Font.Size = Size;
    Text->SetFont(Font);
}

static UTexture2D* ContractoryGetMissionIcon(
    const FString& ItemKey
)
{
    if (ItemKey.IsEmpty())
    {
        return nullptr;
    }

    const FString Path =
        FString::Printf(
            TEXT(
                "/SatisfactoryReverse/Recyclage/%s.%s_C"
            ),
            *ItemKey,
            *ItemKey
        );

    UClass* DescriptorClass =
        LoadClass<UFGItemDescriptor>(
            nullptr,
            *Path
        );

    if (!DescriptorClass)
    {
        return nullptr;
    }

    return UFGItemDescriptor::GetSmallIcon(
        DescriptorClass
    );
}

TSharedRef<SWidget>
UContractoryMissionHUDWidget::RebuildWidget()
{
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        BuildHUD();
    }

    return Super::RebuildWidget();
}

void UContractoryMissionHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!WidgetTree->RootWidget)
    {
        BuildHUD();
    }

    BindMissionRCO();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MissionPollTimer,
            this,
            &UContractoryMissionHUDWidget::PollMissionState,
            1.0f,
            true,
            0.5f
        );
    }
}

void UContractoryMissionHUDWidget::NativeDestruct()
{
    if (MissionRCO)
    {
        MissionRCO->OnMissionStateUpdated.RemoveAll(
            this
        );
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            MissionPollTimer
        );
    }

    MissionRCO = nullptr;

    Super::NativeDestruct();
}

void UContractoryMissionHUDWidget::BindMissionRCO()
{
    if (MissionRCO)
    {
        return;
    }

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
            PlayerController
                ->GetRemoteCallObjectOfClass(
                    UContractoryMissionRemoteCallObject
                        ::StaticClass()
                )
        );

    if (!MissionRCO)
    {
        MissionRCO =
            Cast<UContractoryMissionRemoteCallObject>(
                PlayerController
                    ->RegisterRemoteCallObjectClass(
                        UContractoryMissionRemoteCallObject
                            ::StaticClass()
                    )
            );
    }

    if (!MissionRCO)
    {
        return;
    }

    MissionRCO->OnMissionStateUpdated.RemoveAll(
        this
    );

    MissionRCO->OnMissionStateUpdated.AddUObject(
        this,
        &UContractoryMissionHUDWidget::RefreshHUD
    );

    RefreshHUD();
    MissionRCO->Server_RequestMissionState();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Contractory HUD] Mission RCO connecte")
    );
}

void UContractoryMissionHUDWidget::PollMissionState()
{
    if (!MissionRCO)
    {
        BindMissionRCO();
        return;
    }

    MissionRCO->Server_RequestMissionState();
}

void UContractoryMissionHUDWidget::RefreshHUD()
{
    if (!MissionRCO || !MainPanel)
    {
        return;
    }

    const TArray<FContractoryActiveMission>& ActiveMissions =
        MissionRCO->GetCachedActiveMissions();

    MainPanel->SetVisibility(
        ActiveMissions.Num() > 0
            ? ESlateVisibility::HitTestInvisible
            : ESlateVisibility::Collapsed
    );

    const TArray<FContractoryMissionDefinition> Catalogue =
        UContractorySecondaryMissions
            ::GetMissionCatalogue();

    for (int32 Index = 0; Index < 3; ++Index)
    {
        UBorder* Row =
            MissionRows.IsValidIndex(Index)
                ? MissionRows[Index]
                : nullptr;

        UImage* Icon =
            MissionIcons.IsValidIndex(Index)
                ? MissionIcons[Index]
                : nullptr;

        UTextBlock* Title =
            MissionTitles.IsValidIndex(Index)
                ? MissionTitles[Index]
                : nullptr;

        UTextBlock* Details =
            MissionDetails.IsValidIndex(Index)
                ? MissionDetails[Index]
                : nullptr;

        UProgressBar* Progress =
            MissionProgressBars.IsValidIndex(Index)
                ? MissionProgressBars[Index]
                : nullptr;

        if (!Row)
        {
            continue;
        }

        if (!ActiveMissions.IsValidIndex(Index))
        {
            Row->SetVisibility(
                ESlateVisibility::Collapsed
            );
            continue;
        }

        Row->SetVisibility(
            ESlateVisibility::HitTestInvisible
        );

        const FContractoryActiveMission& Active =
            ActiveMissions[Index];

        const FContractoryMissionDefinition* Definition =
            Catalogue.FindByPredicate(
                [&Active](
                    const FContractoryMissionDefinition& Mission
                )
                {
                    return Mission.MissionId ==
                        Active.MissionId;
                }
            );

        if (!Definition)
        {
            if (Title)
            {
                Title->SetText(
                    FText::FromString(
                        FString::Printf(
                            TEXT("Mission %d"),
                            Active.MissionId
                        )
                    )
                );
            }

            if (Details)
            {
                Details->SetText(
                    FText::FromString(
                        TEXT("Synchronisation...")
                    )
                );
            }

            if (Progress)
            {
                Progress->SetPercent(0.0f);
            }

            continue;
        }

        const int32 Delivered =
            FMath::Max(
                Active.DeliveredAmount,
                0
            );

        const int32 Target =
            FMath::Max(
                Definition->TargetAmount,
                1
            );

        const int32 Remaining =
            FMath::Max(
                Target - Delivered,
                0
            );

        const bool bReady =
            Delivered >= Target;

        const float Percent =
            FMath::Clamp(
                static_cast<float>(Delivered) /
                static_cast<float>(Target),
                0.0f,
                1.0f
            );

        if (Icon)
        {
            if (UTexture2D* Texture =
                ContractoryGetMissionIcon(
                    Definition->TargetItemKey
                ))
            {
                Icon->SetBrushFromTexture(
                    Texture,
                    true
                );

                Icon->SetVisibility(
                    ESlateVisibility::HitTestInvisible
                );
            }
            else
            {
                Icon->SetVisibility(
                    ESlateVisibility::Collapsed
                );
            }
        }

        if (Title)
        {
            Title->SetText(
                Definition->Title
            );

            Title->SetColorAndOpacity(
                FSlateColor(
                    bReady
                        ? FLinearColor(
                            0.20f,
                            1.00f,
                            0.30f,
                            1.00f
                        )
                        : FLinearColor::White
                )
            );
        }

        if (Details)
        {
            FString Text;

            if (bReady)
            {
                Text = FString::Printf(
                    TEXT(
                        "%d / %d  |  PRETE A RECLAMER  |  +%d credits"
                    ),
                    Delivered,
                    Target,
                    Definition->RewardCredits
                );
            }
            else
            {
                Text = FString::Printf(
                    TEXT(
                        "%d / %d  |  Reste : %d  |  +%d credits"
                    ),
                    Delivered,
                    Target,
                    Remaining,
                    Definition->RewardCredits
                );
            }

            Details->SetText(
                FText::FromString(Text)
            );

            Details->SetColorAndOpacity(
                FSlateColor(
                    bReady
                        ? FLinearColor(
                            0.20f,
                            1.00f,
                            0.30f,
                            1.00f
                        )
                        : FLinearColor(
                            0.85f,
                            0.85f,
                            0.85f,
                            1.00f
                        )
                )
            );
        }

        if (Progress)
        {
            Progress->SetPercent(
                Percent
            );

            Progress->SetFillColorAndOpacity(
                bReady
                    ? FLinearColor(
                        0.20f,
                        1.00f,
                        0.30f,
                        1.00f
                    )
                    : FLinearColor(
                        1.00f,
                        0.48f,
                        0.07f,
                        1.00f
                    )
            );
        }
    }
}

void UContractoryMissionHUDWidget::BuildHUD()
{
    if (!WidgetTree)
    {
        return;
    }

    MissionRows.Reset();
    MissionIcons.Reset();
    MissionTitles.Reset();
    MissionDetails.Reset();
    MissionProgressBars.Reset();

    UCanvasPanel* Root =
        WidgetTree->ConstructWidget<UCanvasPanel>(
            UCanvasPanel::StaticClass(),
            TEXT("ContractoryHUDRoot")
        );

    WidgetTree->RootWidget = Root;

    USizeBox* PanelSize =
        WidgetTree->ConstructWidget<USizeBox>(
            USizeBox::StaticClass(),
            TEXT("ContractoryHUDPanelSize")
        );

    PanelSize->SetWidthOverride(380.0f);

    MainPanel =
        WidgetTree->ConstructWidget<UBorder>(
            UBorder::StaticClass(),
            TEXT("ContractoryHUDPanel")
        );

    MainPanel->SetPadding(FMargin(9.0f, 7.0f, 9.0f, 8.0f));

    MainPanel->SetBrushColor(
        FLinearColor(
            0.025f,
            0.030f,
            0.035f,
            0.88f
        )
    );

    MainPanel->SetVisibility(
        ESlateVisibility::Collapsed
    );

    PanelSize->AddChild(
        MainPanel
    );

    UCanvasPanelSlot* PanelSlot =
        Root->AddChildToCanvas(
            PanelSize
        );

    PanelSlot->SetAnchors(
        FAnchors(
            0.0f, 0.0f, 0.0f, 0.0f
        )
    );

    PanelSlot->SetAlignment(
        FVector2D(
            0.0f, 0.0f
        )
    );

    PanelSlot->SetPosition(FVector2D(30.0f, 145.0f));

    PanelSlot->SetAutoSize(true);

    UVerticalBox* MainBox =
        WidgetTree->ConstructWidget<UVerticalBox>(
            UVerticalBox::StaticClass(),
            TEXT("ContractoryHUDMainBox")
        );

    MainPanel->SetContent(
        MainBox
    );

    UTextBlock* Header =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("ContractoryHUDHeader")
        );

    Header->SetText(
        FText::FromString(
            TEXT("CONTRACTORY  |  MISSIONS EN COURS")
        )
    );

    ContractorySetFontSize(Header, 15);

    Header->SetColorAndOpacity(
        FSlateColor(
            FLinearColor(
                1.0f,
                0.48f,
                0.07f,
                1.0f
            )
        )
    );

    MainBox->AddChildToVerticalBox(
        Header
    );

    UBorder* HeaderLine =
        WidgetTree->ConstructWidget<UBorder>(
            UBorder::StaticClass(),
            TEXT("ContractoryHUDHeaderLine")
        );

    HeaderLine->SetPadding(
        FMargin(
            0.0f,
            1.5f
        )
    );

    HeaderLine->SetBrushColor(
        FLinearColor(
            1.0f,
            0.48f,
            0.07f,
            1.0f
        )
    );

    MainBox->AddChildToVerticalBox(
        HeaderLine
    );

    for (int32 Index = 0; Index < 3; ++Index)
    {
        UBorder* Row =
            WidgetTree->ConstructWidget<UBorder>(
                UBorder::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT(
                            "ContractoryHUDRow_%d"
                        ),
                        Index
                    )
                )
            );

        Row->SetPadding(FMargin(5.0f, 5.0f));

        Row->SetBrushColor(
            FLinearColor(
                0.055f,
                0.065f,
                0.075f,
                0.93f
            )
        );

        Row->SetVisibility(
            ESlateVisibility::Collapsed
        );

        MainBox->AddChildToVerticalBox(
            Row
        );

        UHorizontalBox* RowBox =
            WidgetTree->ConstructWidget<UHorizontalBox>(
                UHorizontalBox::StaticClass(),
                FName(
                    *FString::Printf(
                        TEXT(
                            "ContractoryHUDRowBox_%d"
                        ),
                        Index
                    )
                )
            );

        Row->SetContent(
            RowBox
        );

        USizeBox* IconBox =
            WidgetTree->ConstructWidget<USizeBox>(
                USizeBox::StaticClass()
            );

        IconBox->SetWidthOverride(32.0f);

        IconBox->SetHeightOverride(32.0f);

        UImage* Icon =
            WidgetTree->ConstructWidget<UImage>(
                UImage::StaticClass()
            );

        IconBox->AddChild(
            Icon
        );

        RowBox->AddChildToHorizontalBox(
            IconBox
        );

        USizeBox* InfoSize =
            WidgetTree->ConstructWidget<USizeBox>(
                USizeBox::StaticClass()
            );

        InfoSize->SetWidthOverride(320.0f);

        UVerticalBox* InfoBox =
            WidgetTree->ConstructWidget<UVerticalBox>(
                UVerticalBox::StaticClass()
            );

        InfoSize->AddChild(
            InfoBox
        );

        RowBox->AddChildToHorizontalBox(
            InfoSize
        );

        UTextBlock* Title =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass()
            );

        ContractorySetFontSize(Title, 13);

        Title->SetText(
            FText::FromString(
                TEXT("Mission Contractory")
            )
        );

        InfoBox->AddChildToVerticalBox(
            Title
        );

        UTextBlock* Details =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass()
            );

        ContractorySetFontSize(Details, 11);

        Details->SetText(
            FText::FromString(
                TEXT("Synchronisation...")
            )
        );

        InfoBox->AddChildToVerticalBox(
            Details
        );

        UProgressBar* Progress =
            WidgetTree->ConstructWidget<UProgressBar>(
                UProgressBar::StaticClass()
            );

        Progress->SetPercent(
            0.0f
        );

        Progress->SetFillColorAndOpacity(
            FLinearColor(
                1.0f,
                0.48f,
                0.07f,
                1.0f
            )
        );

        InfoBox->AddChildToVerticalBox(
            Progress
        );

        MissionRows.Add(Row);
        MissionIcons.Add(Icon);
        MissionTitles.Add(Title);
        MissionDetails.Add(Details);
        MissionProgressBars.Add(Progress);
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[Contractory HUD] Interface construite")
    );
}
