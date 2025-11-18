// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "Components/InteractableComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	InteractableComp = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComp"));

	// Initial settings
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// 무게 설정
	Mesh->SetMassOverrideInKg(NAME_None, 0.1f, true);

	// Replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Initial state
	ItemType = EItemType::None;
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItem, ItemType);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::OnRep_ItemData()
{
	SetItemMesh();
}

void AItem::SetItemMesh()
{
	if (ItemMeshTable)
	{
		FString RowName = FString::FromInt((int32)ItemType);
		FItemMeshData* MeshData =
			ItemMeshTable->FindRow<FItemMeshData>(FName(*RowName), TEXT("ItemMesh"));

		if (MeshData)
		{
			if (MeshData->ItemMesh)
			{
				Mesh->SetStaticMesh(MeshData->ItemMesh);
			}
		}
	}
}
