#include "ContractoryDeliveryBuildable.h"

#include "Components/SceneComponent.h"
#include "Components/ChildActorComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

#include "FGInventoryComponent.h"
#include "FGFactoryConnectionComponent.h"
#include "FGCharacterPlayer.h"
#include "Resources/FGItemDescriptor.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "ContractoryMissionSaveManager.h"
#include "ContractoryMissionManager.h"
#include "ContractorySecondaryMissions.h"

namespace
{
    bool FindContractoryMissionDefinition(
        int32 MissionId,
        FContractoryMissionDefinition& OutDefinition
    )
    {
        for (
            const FContractoryMissionDefinition& Definition :
            UContractorySecondaryMissions::GetMissionCatalogue()
        )
        {
            if (Definition.MissionId == MissionId)
            {
                OutDefinition = Definition;
                return true;
            }
        }

        return false;
    }

    int32 GetRemainingDeliveryNeed(
        const FContractoryPlayerMissionState* State,
        const FString& ItemKey
    )
    {
        if (!State || ItemKey.IsEmpty())
        {
            return 0;
        }

        int32 TotalNeeded = 0;

        for (const FContractoryActiveMission& Active :
             State->Session.ActiveMissions)
        {
            FContractoryMissionDefinition Definition;

            if (!FindContractoryMissionDefinition(
                    Active.MissionId, Definition))
            {
                continue;
            }

            for (int32 i = 0;
                 i < Definition.Requirements.Num(); ++i)
            {
                const FContractoryMissionRequirement& Requirement =
                    Definition.Requirements[i];

                if (Requirement.ItemKey != ItemKey)
                {
                    continue;
                }

                const int32 Delivered =
                    UContractoryMissionManager::
                        GetDeliveredForRequirement(Active, i);

                TotalNeeded += FMath::Max(
                    0,
                    Requirement.TargetAmount - Delivered
                );
            }
        }

        return TotalNeeded;
    }
    FString GetContractoryItemKey(UClass* ItemClass)
    {
        if (!ItemClass)
        {
            return FString();
        }

        FString Result = ItemClass->GetName();

        Result.RemoveFromEnd(
            TEXT("_C")
        );

        return Result;
    }
}

AContractoryDeliveryBuildable::
AContractoryDeliveryBuildable()
{
    // Position de la prise electrique sur le flanc droit
    PowerAnchor->SetRelativeLocation(
        FVector(143.0f, 14.0f, 88.0f)
    );

    PowerAnchor->SetRelativeRotation(
        FRotator(0.0f, 90.0f, 0.0f)
    );

    // ============================================================
    // INVENTAIRE DE LIVRAISON
    // ============================================================

    DeliveryInventory =
        CreateDefaultSubobject<UFGInventoryComponent>(
            TEXT("ContractoryDeliveryInventory")
        );

    // ============================================================
    // ENTREE CONVOYEUR
    // ============================================================

    DeliveryInputAnchor =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("ContractoryDeliveryInputAnchor")
        );

    DeliveryInputAnchor->SetupAttachment(
        RootComponent
    );

    DeliveryInputAnchor->SetRelativeLocation(
        FVector(0.0f, 170.0f, 71.3f)
    );

    DeliveryInputAnchor->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

    DeliveryInputConnection =
        CreateDefaultSubobject<UFGFactoryConnectionComponent>(
            TEXT("ContractoryDeliveryInput")
        );

    DeliveryInputConnection->SetupAttachment(
        DeliveryInputAnchor
    );

    DeliveryInputConnection->SetMobility(
        EComponentMobility::Movable
    );

    DeliveryInputConnection->SetDirection(
            EFactoryConnectionDirection::FCD_INPUT
        );
        DeliveryInputConnection->SetConnectorClearance(0.0f);
        DeliveryInputConnection->SetForwardPeekAndGrabToBuildable(false);

    DeliveryInputConnection->SetInventory(
        DeliveryInventory
    );

    DeliveryInputConnection->SetInventoryAccessIndex(
        0
    );
}

bool AContractoryDeliveryBuildable::IsUseable_Implementation() const
{
    // E = depot manuel.
    // Aucun menu Contractory n'est ouvert.
    return true;
}

void AContractoryDeliveryBuildable::OnUse_Implementation(
    AFGCharacterPlayer* byCharacter,
    const FUseState& state
)
{
    if (
        !byCharacter ||
        !HasAuthority() ||
        !HasContractoryPower() ||
        !DeliveryInventory
    )
    {
        return;
    }

    UFGInventoryComponent* PlayerInventory =
        byCharacter->GetInventory();

    if (!PlayerInventory)
    {
        return;
    }

    int32 TotalTransferred = 0;

    const int32 InventorySize =
        PlayerInventory->GetSizeLinear();

    for (
        int32 Index = 0;
        Index < InventorySize;
        ++Index
    )
    {
        FInventoryStack Stack;

        if (
            !PlayerInventory->GetStackFromIndex(
                Index,
                Stack
            )
        )
        {
            continue;
        }

        if (
            Stack.NumItems <= 0 ||
            !Stack.Item.GetItemClass()
        )
        {
            continue;
        }

        UClass* ItemClass =
            Stack.Item.GetItemClass().Get();

        // Seules les ressources actuellement demandees
        // par les missions peuvent etre deposees.
        if (
            !IsDeliveryItemAllowed(
                ItemClass,
                0
            )
        )
        {
            continue;
        }

        AContractoryMissionSaveManager* Manager =
            ResolveMissionSaveManager();

        if (!Manager)
        {
            continue;
        }

        const FContractoryPlayerMissionState* State =
            Manager->FindPlayer(TEXT("CONTRACTORY_SHARED"));

        const FString ItemKey =
            GetContractoryItemKey(ItemClass);

        const int32 Needed =
            GetRemainingDeliveryNeed(State, ItemKey);

        if (Needed <= 0)
        {
            continue;
        }

        FInventoryStack LimitedStack = Stack;
        LimitedStack.NumItems =
            FMath::Min(Stack.NumItems, Needed);

        const int32 Added =
            DeliveryInventory->AddStackToIndex(
                0,
                LimitedStack,
                true,
                PlayerInventory
            );

        if (Added <= 0)
        {
            continue;
        }

        PlayerInventory->RemoveFromIndex(
            Index,
            Added
        );

        TotalTransferred += Added;

        // Consomme immediatement les ressources acceptees
        // afin de liberer le slot pour le type suivant.
        ProcessDeliveryInventory();
    }

    if (TotalTransferred > 0)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[Contractory Livraison] "
                "Depot manuel : %d ressource(s)"
            ),
            TotalTransferred
        );

        ForceNetUpdate();
    }
}

void AContractoryDeliveryBuildable::BeginPlay()
{
    Super::BeginPlay();



    // ============================================================
    // INVENTAIRE : UN SLOT
    // ============================================================

    if (DeliveryInventory)
    {
        if (
            HasAuthority() &&
            DeliveryInventory->GetSizeLinear() != 1
        )
        {
            DeliveryInventory->Resize(1);
        }

        DeliveryInventory->AddArbitrarySlotSize(
            0,
            100000
        );

        // Filtre dynamique :
        // seules les ressources des missions actives sont autorisees.
        DeliveryInventory->mItemFilter.Unbind();

        DeliveryInventory->mItemFilter.BindUObject(
            this,
            &AContractoryDeliveryBuildable::
                IsDeliveryItemAllowed
        );
    }

    // ============================================================
    // CONNEXION CONVOYEUR
    // ============================================================

    if (
        DeliveryInputConnection &&
        DeliveryInventory
    )
    {
        DeliveryInputConnection->SetDirection(
            EFactoryConnectionDirection::FCD_INPUT
        );
        DeliveryInputConnection->SetConnectorClearance(0.0f);
        DeliveryInputConnection->SetForwardPeekAndGrabToBuildable(false);

        DeliveryInputConnection->SetInventory(
            DeliveryInventory
        );

        DeliveryInputConnection->SetInventoryAccessIndex(
            0
        );
    }


    ResolveMissionSaveManager();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[Contractory] Livraison ready : "
            "Inventory=%s Connection=%s"
            " Registered=%s Owner=%s Direction=%d"
            " Connected=%s InventoryIndex=%d"
            " WorldLocation=%s WorldRotation=%s"
        ),
        DeliveryInventory
            ? *DeliveryInventory->GetName()
            : TEXT("NONE"),
        DeliveryInputConnection
            ? *DeliveryInputConnection->GetName()
            : TEXT("NONE"),
        DeliveryInputConnection && DeliveryInputConnection->IsRegistered()
            ? TEXT("YES") : TEXT("NO"),
        DeliveryInputConnection && DeliveryInputConnection->GetOuterBuildable()
            ? *DeliveryInputConnection->GetOuterBuildable()->GetName()
            : TEXT("NONE"),
        DeliveryInputConnection
            ? static_cast<int32>(DeliveryInputConnection->GetDirection()) : -1,
        DeliveryInputConnection && DeliveryInputConnection->IsConnected()
            ? TEXT("YES") : TEXT("NO"),
        DeliveryInputConnection
            ? DeliveryInputConnection->GetInventoryAccessIndex() : -1,
        DeliveryInputConnection
            ? *DeliveryInputConnection->GetComponentLocation().ToString()
            : TEXT("NONE"),
        DeliveryInputConnection
            ? *DeliveryInputConnection->GetComponentRotation().ToString()
            : TEXT("NONE")
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Contractory BELT] FactoryConnections=%d InputRegistered=%s InputConnected=%s"),
        static_cast<int32>(GetNumFactoryConnections()),
        DeliveryInputConnection && DeliveryInputConnection->IsRegistered()
            ? TEXT("YES") : TEXT("NO"),
        DeliveryInputConnection && DeliveryInputConnection->IsConnected()
            ? TEXT("YES") : TEXT("NO")
    );
}

void AContractoryDeliveryBuildable::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority())
    {
        return;
    }

    static float DiagnosticTimer = 0.0f;
    DiagnosticTimer += DeltaSeconds;

    if (DiagnosticTimer >= 5.0f)
    {
        DiagnosticTimer = 0.0f;

        FInventoryStack DiagnosticStack;
        const bool bHasStack =
            DeliveryInventory &&
            DeliveryInventory->GetStackFromIndex(0, DiagnosticStack);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Contractory DIAG] Power=%s Belt=%s InventoryItems=%d Manager=%s"),
            HasContractoryPower() ? TEXT("YES") : TEXT("NO"),
            DeliveryInputConnection && DeliveryInputConnection->IsConnected()
                ? TEXT("CONNECTED") : TEXT("DISCONNECTED"),
            bHasStack ? DiagnosticStack.NumItems : 0,
            ResolveMissionSaveManager() ? TEXT("YES") : TEXT("NO")
        );
    }

    if (!HasContractoryPower())
    {
        return;
    }

    ProcessDeliveryInventory();
}



AContractoryMissionSaveManager*
AContractoryDeliveryBuildable::
ResolveMissionSaveManager()
{
    if (IsValid(MissionSaveManager))
    {
        return MissionSaveManager;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    for (
        TActorIterator<AContractoryMissionSaveManager> It(World);
        It;
        ++It
    )
    {
        MissionSaveManager = *It;
        break;
    }

    return MissionSaveManager;
}

bool AContractoryDeliveryBuildable::
IsDeliveryItemAllowed(
    TSubclassOf<UObject> ItemClass,
    int32 Index
)
{
    if (Index != INDEX_NONE && Index != 0)
    {
        return false;
    }

    if (!HasContractoryPower())
    {
        return false;
    }

    UClass* RawClass = ItemClass.Get();

    if (
        !RawClass ||
        !RawClass->IsChildOf(UFGItemDescriptor::StaticClass())
    )
    {
        return false;
    }

    AContractoryMissionSaveManager* Manager =
        ResolveMissionSaveManager();

    if (!Manager)
    {
        return false;
    }

    const FContractoryPlayerMissionState* State =
        Manager->FindPlayer(TEXT("CONTRACTORY_SHARED"));

    if (!State)
    {
        return false;
    }

    const FString ItemKey =
        GetContractoryItemKey(RawClass);

    if (ItemKey.IsEmpty())
    {
        return false;
    }

    for (
        const FContractoryActiveMission& Active :
        State->Session.ActiveMissions
    )
    {
        FContractoryMissionDefinition Definition;

        if (
            !FindContractoryMissionDefinition(
                Active.MissionId,
                Definition
            )
        )
        {
            continue;
        }

        for (
            int32 RequirementIndex = 0;
            RequirementIndex < Definition.Requirements.Num();
            ++RequirementIndex
        )
        {
            const FContractoryMissionRequirement& Requirement =
                Definition.Requirements[RequirementIndex];

            if (
                Requirement.ItemKey == ItemKey &&
                UContractoryMissionManager::GetDeliveredForRequirement(
                    Active,
                    RequirementIndex
                ) < Requirement.TargetAmount
            )
            {
                return true;
            }
        }
    }

    return false;
}
void AContractoryDeliveryBuildable::
ProcessDeliveryInventory()
{
    if (
        !DeliveryInventory ||
        !HasContractoryPower()
    )
    {
        return;
    }

    AContractoryMissionSaveManager* Manager =
        ResolveMissionSaveManager();

    if (!Manager)
    {
        return;
    }

    FContractoryPlayerMissionState* State =
        Manager->FindOrAddServerPlayer(
            TEXT("CONTRACTORY_SHARED")
        );

    if (!State)
    {
        return;
    }

    FInventoryStack Stack;

    if (
        !DeliveryInventory->GetStackFromIndex(
            0,
            Stack
        )
    )
    {
        return;
    }

    if (
        Stack.NumItems <= 0 ||
        !Stack.Item.GetItemClass()
    )
    {
        return;
    }

    UClass* ItemClass =
        Stack.Item.GetItemClass().Get();

    const FString ItemKey =
        GetContractoryItemKey(ItemClass);

    if (ItemKey.IsEmpty())
    {
        return;
    }

    int32 RemainingItems = Stack.NumItems;
    int32 AcceptedItems = 0;

    // Une ressource peut alimenter plusieurs missions actives.
    for (
        FContractoryActiveMission& Active :
        State->Session.ActiveMissions
    )
    {
        if (RemainingItems <= 0)
        {
            break;
        }

        const int32 Accepted =
            UContractoryMissionManager::
                RecordVerifiedDeliveryForItem(
                    State->Session,
                    Active.MissionId,
                    ItemKey,
                    RemainingItems
                );

        if (Accepted <= 0)
        {
            continue;
        }

        AcceptedItems += Accepted;
        RemainingItems -= Accepted;

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[Contractory] Livraison : "
                "Mission=%d Item=%s +%d"
            ),
            Active.MissionId,
            *ItemKey,
            Accepted
        );
    }

    if (AcceptedItems <= 0)
    {
        return;
    }

    DeliveryInventory->RemoveFromIndex(
        0,
        AcceptedItems
    );

    Manager->ForceNetUpdate();
}
