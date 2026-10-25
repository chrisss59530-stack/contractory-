#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "ContractoryMissionHUDWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UProgressBar;
class UContractoryMissionRemoteCallObject;

UCLASS()
class SATISFACTORYREVERSE_API UContractoryMissionHUDWidget
    : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void BuildHUD();
    void BindMissionRCO();
    void PollMissionState();
    void RefreshHUD();

    UPROPERTY(Transient)
    TObjectPtr<UBorder> MainPanel;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UBorder>> MissionRows;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UImage>> MissionIcons;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTextBlock>> MissionTitles;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTextBlock>> MissionDetails;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UProgressBar>> MissionProgressBars;

    UPROPERTY(Transient)
    TObjectPtr<UContractoryMissionRemoteCallObject> MissionRCO;

    FTimerHandle MissionPollTimer;
};
