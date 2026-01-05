// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ViewResultActor.generated.h"

UCLASS()
class HOLDEM_API AViewResultActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AViewResultActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// 3D Widget Component                                                                                                                                                                                                        
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* WidgetComponent;

private:
	// GameState Delegate 구독                                                                                                                                                                                                    
	void TrySubscribeToGameState();
	
	// ShowDown 결과 수신                                                                                                                                                                                                         
	UFUNCTION()
	void OnShowdownResultReceived(FString WinnerNames, EHandRank HighestRank);
	
	// 로컬 플레이어 시점에 맞춰 회전                                                                                                                                                                                             
	void AdjustRotationForLocalPlayer();
	
	// Widget에 데이터 전달                                                                                                                                                                                                       
	void UpdateWidget(const FString& WinnerNames, EHandRank HighestRank);
	
	// 플래그                                                                                                                                                                                                                     
	bool bHasSubscribedDelegate = false;
	bool bHasAdjustedRotation = false;

};
