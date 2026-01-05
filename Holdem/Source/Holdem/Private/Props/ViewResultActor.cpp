// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/ViewResultActor.h"

#include "Components/WidgetComponent.h"
#include "Game/HoldemGameState.h"
#include "Game/HoldemPlayerState.h"
#include "UI/ResultWidget.h"

// Sets default values
AViewResultActor::AViewResultActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root Component (더미)
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
  
	// WidgetComponent 생성                                                                                                                                                                                                       
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(Root);
	
	// Replication 설정 (Level 배치이므로 기본값 사용)                                                                                                                                                                            
	bReplicates = false; // Level에 배치된 Actor는 Replicate 불필요
}

// Called when the game starts or when spawned
void AViewResultActor::BeginPlay()
{
	Super::BeginPlay();

	WidgetComponent->SetVisibility(false);

	// 로컬 플레이어 시점에 맞춰 회전
	AdjustRotationForLocalPlayer();
	
	// GameState Delegate 구독 시도
	TrySubscribeToGameState();
}

// Called every frame
void AViewResultActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 회전 조정 재시도                                                                                                                                                                                                           
	if (!bHasAdjustedRotation)
	{
	    AdjustRotationForLocalPlayer();
	}
	
	// Delegate 구독 재시도                                                                                                                                                                                                       
	if (!bHasSubscribedDelegate)
	{
	    TrySubscribeToGameState();
	}
	
	// 둘 다 완료되면 Tick 비활성화 (최적화)                                                                                                                                                                                      
	if (bHasAdjustedRotation && bHasSubscribedDelegate)
	{
	    SetActorTickEnabled(false);
	}
}

void AViewResultActor::AdjustRotationForLocalPlayer()                                                                                                                                                                           
{
	if (!GetWorld()) return;
	
	// 로컬 PlayerController 가져오기                                                                                                                                                                                             
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || !LocalPC->IsLocalPlayerController())
	  return;

	AHoldemPlayerState* PS = LocalPC->GetPlayerState<AHoldemPlayerState>();
	if (!PS)
	  return; // PlayerState 아직 없음, 다음 Tick에 재시도                                                                                                                                                                      

	// SeatIndex에 따라 회전 (0=0도, 1=90도, 2=180도, 3=270도)                                                                                                                                                                    
	float TargetYaw = PS->SeatIndex * 90.0f;

	FRotator NewRotation = WidgetComponent->GetRelativeRotation();
	NewRotation.Yaw = TargetYaw;
	WidgetComponent->SetRelativeRotation(NewRotation);

	bHasAdjustedRotation = true;

	UE_LOG(LogTemp, Log, TEXT("[ResultWidget] Adjusted rotation for SeatIndex %d: Yaw=%.1f"),
	  PS->SeatIndex, TargetYaw);
}

void AViewResultActor::TrySubscribeToGameState()                                                                                                                                                                                
{
  AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
  if (!GS)
      return; // GameState 아직 없음, 다음 Tick에 재시도                                                                                                                                                                        

  // Delegate 구독                                                                                                                                                                                                              
  GS->OnShowdownResultChanged.AddDynamic(this, &AViewResultActor::OnShowdownResultReceived);

  bHasSubscribedDelegate = true;

  UE_LOG(LogTemp, Log, TEXT("[ResultWidget] Subscribed to GameState delegate"));
}

void AViewResultActor::OnShowdownResultReceived(FString WinnerNames, EHandRank HighestRank)                                                                                                                                     
{
  UE_LOG(LogTemp, Warning, TEXT("[ResultWidget] Received ShowDown result: %s - %s"),
      *WinnerNames, *UEnum::GetValueAsString(HighestRank));

  // Widget 표시                                                                                                                                                                                                                
  WidgetComponent->SetVisibility(true);

  // Widget 업데이트
  UpdateWidget(WinnerNames, HighestRank);

  // 5초 후 숨기기 (타이머)
  FTimerHandle HideTimer;
  GetWorldTimerManager().SetTimer(HideTimer, [this]()
  {
      WidgetComponent->SetVisibility(false);
  }, 5.0f, false);
}

void AViewResultActor::UpdateWidget(const FString& WinnerNames, EHandRank HighestRank)                                                                                                                                          
{
  // Widget 인스턴스 가져오기
  UResultWidget* ResultWidget = Cast<UResultWidget>(WidgetComponent->GetUserWidgetObject());
  if (!ResultWidget)
  {
      UE_LOG(LogTemp, Error, TEXT("[ResultWidget] Widget instance is null!"));
      return;
  }

  // ResultWidget의 업데이트 함수 호출 
  ResultWidget->UpdateResult(WinnerNames, HighestRank);
}