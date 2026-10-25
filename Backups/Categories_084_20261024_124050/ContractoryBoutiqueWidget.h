#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/FGInteractWidget.h"
#include "Components/Button.h"
#include "ContractoryBoutiqueWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UImage;
class UScrollBox;
class UEditableTextBox;
class UWidget;
class UContractoryMissionRemoteCallObject;
class UFGSchematic;

UCLASS()
class SATISFACTORYREVERSE_API UContractoryBoutiqueBuyButton
    : public UButton
{
    GENERATED_BODY()

public:
    void Configure(
        UContractoryMissionRemoteCallObject* InMissionRCO,
        const FString& InSchematicClassPath
    );

private:
    UFUNCTION()
    void HandleClicked();

    UPROPERTY(Transient)
    TObjectPtr<UContractoryMissionRemoteCallObject> MissionRCO;

    FString SchematicClassPath;
};

UCLASS()
class SATISFACTORYREVERSE_API UContractoryBoutiqueWidget
    : public UFGInteractWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SetInputMode_Implementation() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void BuildBoutiqueInterface();
    void BindMissionRemoteCallObject();
    void RefreshBoutiqueInterface();
    void PollBoutiqueState();

        // CONTRACTORY_BOUTIQUE_SEARCH_V2
    UFUNCTION()
    void HandleSearchChanged(const FText& SearchText);

    void HandleBoutiquePurchaseResult(
        bool bSuccess,
        int32 RemainingCredits,
        const FString& SchematicClassPath,
        const FString& FailureReason
    );

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> MainBox;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> CreditsText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ResultText;
    // CONTRACTORY_BOUTIQUE_SEARCH_V2
    UPROPERTY(Transient)
    TObjectPtr<UEditableTextBox> SearchBox;

    UPROPERTY(Transient)
    TObjectPtr<UScrollBox> CatalogueScroll;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UWidget>> EntryCards;

    TArray<FString> EntrySearchTexts;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTextBlock>> EntryStatusTexts;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTextBlock>> EntryButtonTexts;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UContractoryBoutiqueBuyButton>> BuyButtons;

    UPROPERTY(Transient)
    TArray<TSubclassOf<UFGSchematic>> SchematicClasses;

    TArray<FString> SchematicPaths;
    TArray<int32> EntryPrices;

    UPROPERTY(Transient)
    TObjectPtr<UContractoryMissionRemoteCallObject> MissionRCO;

    FTimerHandle BoutiquePollTimer;
};

