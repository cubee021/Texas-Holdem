// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractableComponent.h"

#include "MyPlayer.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	bIsPickedUp = false;
	bOriginalSimulatePhysics = false;
	OriginalCollisionType = ECollisionEnabled::NoCollision;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInteractableComponent::PickUp(USceneComponent* AttachTarget, FName AttachSocketName)
{
	if (bIsPickedUp) return;

	// Attach할 타겟 유효한지 확인
	if (!AttachTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractableComponent::PickUp - AttachTarget is null!"));
		return;
	}

	// Owner actor의 PrimitiveComponent 찾기
	UPrimitiveComponent* PrimComp = GetOwnerPrimitiveComponent();
	if (!PrimComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractableComponent::PickUp - No PrimitiveComponent found on owner!"));
		return;
	}

	// 원래 상태 저장
	// Drop 시 원래 상태로 되돌리기 위함
	bOriginalSimulatePhysics = PrimComp->IsSimulatingPhysics();
	OriginalCollisionType = PrimComp->GetCollisionEnabled();

	// pick up
	PrimComp->SetSimulatePhysics(false);

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->AttachToComponent(AttachTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}

	bIsPickedUp = true;
	UE_LOG(LogTemp, Log, TEXT("InteractableComponent::PickUp - %s picked up"), *GetOwner()->GetName());
}

void UInteractableComponent::Drop()
{
	if (!bIsPickedUp) return;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		// detach
		Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		UPrimitiveComponent* PrimComp = GetOwnerPrimitiveComponent();
		if (PrimComp)
		{
			PrimComp->SetSimulatePhysics(bOriginalSimulatePhysics);
			PrimComp->SetCollisionEnabled(OriginalCollisionType);
		}

		bIsPickedUp = false;

		UE_LOG(LogTemp, Log, TEXT("InteractableComponent::Drop - %s dropped"), *Owner->GetName());
	}
}

UPrimitiveComponent* UInteractableComponent::GetOwnerPrimitiveComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());

	// RootComponent가 PrimitiveComponent가 아니면, 자식 중에서 찾기
	if (!PrimComp)
	{
		TArray<UActorComponent*> Components;
		Owner->GetComponents(UPrimitiveComponent::StaticClass(), Components);

		if (Components.Num() > 0)
		{
			// 첫 번쩨 primitive component 반환
			PrimComp = Cast<UPrimitiveComponent>(Components[0]);
		}
	}

	return PrimComp;
}

