// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Card.generated.h"

// 카드 무늬
UENUM(BlueprintType)
enum class ECardSuit : uint8
{
	Spades		UMETA(DisplayName = "Spades"),   // ♠
	Hearts		UMETA(DisplayName = "Hearts"),   // ♥
	Diamonds	UMETA(DisplayName = "Diamonds"), // ♦
	Clubs		UMETA(DisplayName = "Clubs")	 // ♣
};

// 카드 숫자
UENUM(BlueprintType)
enum class ECardRank : uint8
{
	Ace = 0		UMETA(DisplayName = "Ace"),
	Two = 1		UMETA(DisplayName = "2"),
	Three = 2	UMETA(DisplayName = "3"),
	Four = 3	UMETA(DisplayName = "4"),
	Five = 4	UMETA(DisplayName = "5"),
	Six = 5		UMETA(DisplayName = "6"),
	Seven = 6	UMETA(DisplayName = "7"),
	Eight = 7	UMETA(DisplayName = "8"),
	Nine = 8	UMETA(DisplayName = "9"),
	Ten = 9		UMETA(DisplayName = "10"),
	Jack = 10	UMETA(DisplayName = "Jack"),
	Queen = 11	UMETA(DisplayName = "Queen"),
	King = 12	UMETA(DisplayName = "King")
};

// 카드 상태
UENUM(BlueprintType)
enum class ECardState : uint8
{
	InDeck		UMETA(DisplayName = "In Deck"), // 덱에 있음 (스폰 안됨)
	OnTable		UMETA(DisplayName = "On Table"), // 공개 카드
	InHand		UMETA(DisplayName = "In Hand"), // 플레이어 소유 카드
	Displaced	UMETA(DisplayName = "Displaced"), // 원래 위치에서 벗어남 (임시)
	Folded		UMETA(DisplayName = "Folded") // 폴드됨
};

// 논리적인 카드 데이터
USTRUCT(BlueprintType)
struct FCardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECardSuit Suit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECardRank Rank;
	// 초기화
	FCardData() : Suit(ECardSuit::Spades), Rank(ECardRank::Ace) {}
	FCardData(ECardSuit InSuit, ECardRank InRank)
		: Suit(InSuit), Rank(InRank) {}

	// 카드 고유 ID
	FORCEINLINE int32 GetCardID() const
	{
		return (int32)Suit * 13 + (int32)Rank;
	}

	// 비교
	FORCEINLINE bool operator==(const FCardData& OtherCard) const
	{
		return Suit == OtherCard.Suit && Rank == OtherCard.Rank;
	}
};

// 카드 Material 데이터
USTRUCT(BlueprintType)
struct FCardVisualData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* CardMesh;
};

UCLASS()
class HOLDEM_API ACard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Card")
	class UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere)
	class UInteractableComponent* InteractableComp;

public:
	// Card Data
	UPROPERTY(ReplicatedUsing=OnRep_CardData, BlueprintReadOnly, Category = "Card")
	FCardData CardData;

	UFUNCTION()
	void OnRep_CardData();
	
	// Card Material
	UPROPERTY(EditDefaultsOnly, Category = "Card")
	class UDataTable* CardVisualTable;
	// 현재 카드 상태
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Card")
	ECardState CardState;
	// 스폰 되는 위치
	FVector OriginalPosition;
	// 스폰 됐을 때 rotation 값
	FRotator OriginalRotation;

public:
	// 카드 설정
	UFUNCTION(BlueprintCallable, Category = "Card")
	void SetCardData(const FCardData& InCardData);
	// 카드 머테리얼 설정
	void UpdateCardVisual();
	// 카드 원위치로
	UFUNCTION(BlueprintCallable, Category = "Card")
	void ResetToOriginalPosition();

};
