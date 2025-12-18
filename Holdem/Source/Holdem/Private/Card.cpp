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

	 if (bIsFlipping)
      {
          switch (FlipPhase)
          {
          case EFlipAnimationPhase::Lifting:
              {
                  FlipProgress += DeltaTime / LiftDuration;

                  if (FlipProgress >= 1.0f)
                  {
                      // Lifting 완료 → Flipping으로 전환
                      FlipPhase = EFlipAnimationPhase::Flipping;
                      FlipProgress = 0.0f;
                      SetActorLocation(FlipTargetLocation);
                  }
                  else                                                                                                                                                  
                  {
                      // 위로 올라가기
                      float Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, FlipProgress, 2.0f);
                      FVector NewLocation = FMath::Lerp(FlipInitialLocation, FlipTargetLocation, Alpha);
                      SetActorLocation(NewLocation);
                  }
              }
              break;

          case EFlipAnimationPhase::Flipping:
              {
                  FlipProgress += DeltaTime / FlipDuration;

                  if (FlipProgress >= 1.0f)
                  {
                      // Flipping 완료 → Dropping으로 전환
                      FlipPhase = EFlipAnimationPhase::Dropping;
                      FlipProgress = 0.0f;
                      SetActorRotation(FlipTargetRotation);
                  }
                  else                                                                                                                                                  
                  {
                      // 제자리에서 뒤집기 (위치는 그대로)
                      float Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, FlipProgress, 2.0f);
                      FRotator NewRotation = FMath::Lerp(FlipInitialRotation, FlipTargetRotation, Alpha);
                      SetActorRotation(NewRotation);
                  }
              }
              break;

          case EFlipAnimationPhase::Dropping:
              {
                  FlipProgress += DeltaTime / DropDuration;

                  if (FlipProgress >= 1.0f)
                  {
                      // 모든 애니메이션 완료
                      bIsFlipping = false;
                      SetActorLocation(FlipInitialLocation);

                      // 물리 재활성화
                      Mesh->SetSimulatePhysics(true);
                      Mesh->SetEnableGravity(true);
                  }
                  else                                                                                                                                                  
                  {
                      // 아래로 내려오기
                      float Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, FlipProgress, 2.0f);
                      FVector NewLocation = FMath::Lerp(FlipTargetLocation, FlipInitialLocation, Alpha);
                      SetActorLocation(NewLocation);
                  }
              }
              break;
          }
      }
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

void ACard::PlayFlipAnimation()
{
	if (!HasAuthority()) return;

	Multicast_PlayFlipAnimation();
}

void ACard::Multicast_PlayFlipAnimation_Implementation()
{
	// 물리 비활성화
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	
	// 현재 위치/회전 저장
	FlipInitialLocation = GetActorLocation();
	FlipInitialRotation = GetActorRotation();

	// 들어올린 위치 계산
	FlipTargetLocation = FlipInitialLocation + FVector(0, 0, FlipLiftHeight);

	// 목표 회전: 180도
	FlipTargetRotation = FlipInitialRotation;
	FlipTargetRotation.Roll += 180.f;

	// 첫 단계: Lifting
	FlipPhase = EFlipAnimationPhase::Lifting;
	FlipProgress = 0.0f;
	bIsFlipping = true;
}

