// Fill out your copyright notice in the Description page of Project Settings.


#include "Card.h"

#include "Components/InteractableComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACard::ACard()
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
	Mesh->SetMassOverrideInKg(NAME_None, 0.01f, true);

	// Replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Initial state
	// 처음에는 덱 안에
	CardState = ECardState::InDeck;
}

// Called when the game starts or when spawned
void ACard::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACard::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACard, CardData);
	DOREPLIFETIME(ACard, CardState);
}

// Called every frame
void ACard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACard::OnRep_CardData()
{
	UpdateCardVisual();
}

void ACard::SetCardData(const FCardData& InCardData)
{
	CardData = InCardData;

	// 서버에서만 직접 Visual 업데이트
	// 클라이언트는 OnRep_CardData에서 자동으로 업데이트됨
	if (HasAuthority())
	{
		UpdateCardVisual();
	}
}

void ACard::UpdateCardVisual()
{
	// 카드 정보에 맞는 머테리얼 적용
	if (CardVisualTable)
	{
		FString RowName = FString::FromInt(CardData.GetCardID());
		FCardVisualData* VisualData =
			CardVisualTable->FindRow<FCardVisualData>(FName(*RowName), TEXT("CardVisual"));

		if (VisualData)
		{
			if (VisualData->CardMesh)
			{
				Mesh->SetStaticMesh(VisualData->CardMesh);
			}
		}
	}
}

void ACard::ResetToOriginalPosition()
{
	SetActorLocation(OriginalPosition);
	SetActorRotation(OriginalRotation);

	CardState = ECardState::OnTable;
	
	UE_LOG(LogTemp, Log, TEXT("Card %d reset to original position"), CardData.GetCardID());
}

