// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "None"),
	Cocktails UMETA(DisplayName = "Cocktails"),
	Whiskey UMETA(DisplayName = "Whiskey"),
	Cigar UMETA(DisplayName = "Cigar")
};

USTRUCT(BlueprintType)
struct FItemMeshData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* ItemMesh;
};

UCLASS()
class HOLDEM_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UInteractableComponent* InteractableComp;

public:
	// Data
	UPROPERTY(ReplicatedUsing = OnRep_ItemData, BlueprintReadOnly)
	EItemType ItemType;

	UFUNCTION()
	void OnRep_ItemData();
	
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	class UDataTable* ItemMeshTable;

	UFUNCTION(BlueprintCallable)
	void SetItemMesh();
};
