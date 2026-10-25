#include "ContractoryDeliveryConveyorStub.h"

#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableConveyorBase.h"

AContractoryDeliveryConveyorStub::
AContractoryDeliveryConveyorStub()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void AContractoryDeliveryConveyorStub::
BeginPlay()
{
    Super::BeginPlay();

    SyncSpeedFromConnectedConveyor();
}

void AContractoryDeliveryConveyorStub::
Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    SyncSpeedFromConnectedConveyor();
}

void AContractoryDeliveryConveyorStub::
SyncSpeedFromConnectedConveyor()
{
    UFGFactoryConnectionComponent* Input =
        GetConnection0();

    if (!Input)
    {
        return;
    }

    UFGFactoryConnectionComponent* Other =
        Input->GetConnection();

    if (!Other)
    {
        return;
    }

    AFGBuildableConveyorBase* ConnectedBelt =
        Cast<AFGBuildableConveyorBase>(
            Other->GetOwner()
        );

    if (!ConnectedBelt)
    {
        return;
    }

    const float ExternalSpeed =
        ConnectedBelt->GetSpeed();

    if (ExternalSpeed <= 0.0f)
    {
        return;
    }

    if (FMath::IsNearlyEqual(
            LastDetectedExternalSpeed,
            ExternalSpeed,
            KINDA_SMALL_NUMBER
        ))
    {
        return;
    }

    // mSpeed appartient a CETTE instance Contractory.
    // Aucune modification du convoyeur joueur / vanilla.
    mSpeed = ExternalSpeed;

    LastDetectedExternalSpeed =
        ExternalSpeed;

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[Contractory Delivery Stub] "
            "Vitesse synchronisee : %.2f"
        ),
        mSpeed
    );
}
