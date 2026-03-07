// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ExpOrbManager.generated.h"

// 定义一个结构体，用来在内存里记录每个球的数据
USTRUCT()
struct FExpOrbData
{
	GENERATED_BODY()

	FVector Location;      // 当前位置
	float ExpValue;        // 经验值大小
	
	// ★ 核心修改：记录这颗球被哪个玩家吸走了（认主锁定！）
	TWeakObjectPtr<AActor> TargetPlayer; 

	FExpOrbData() : Location(FVector::ZeroVector), ExpValue(0.f), TargetPlayer(nullptr) {}
	FExpOrbData(FVector InLoc, float InVal) : Location(InLoc), ExpValue(InVal), TargetPlayer(nullptr) {}
};

UCLASS()
class MPROGUELIKE_API AExpOrbManager : public AActor
{
	GENERATED_BODY()

public:
	AExpOrbManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHierarchicalInstancedStaticMeshComponent* HISMComponent;

	UFUNCTION(BlueprintCallable, Category = "ExpSystem")
	void AddExpOrb(FVector SpawnLocation, float ExpAmount);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddExpOrb(FVector SpawnLocation, float ExpAmount);

	UPROPERTY(EditAnywhere, Category = "ExpSystem")
	float MagnetRadius = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "ExpSystem")
	float PickupRadius = 50.f;

	UPROPERTY(EditAnywhere, Category = "ExpSystem")
	float MagnetSpeed = 1500.f;
	
private:
	TArray<FExpOrbData> OrbList;
};