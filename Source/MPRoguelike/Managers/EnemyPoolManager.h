// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MPRoguelike/GameplayAbilitySystem/Characters/EnemyBase.h"
#include "EnemyPoolManager.generated.h"

USTRUCT()
struct FEnemyArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AEnemyBase*> Enemies;
};

UCLASS()
class MPROGUELIKE_API AEnemyPoolManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyPoolManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

public:
	
	// 【核心升级1】字典配置表：让你能在蓝图面板里自由添加：[近战怪:100只], [远程怪:50只]...
	UPROPERTY(EditAnywhere, Category = "Pool Settings")
	TMap<TSubclassOf<AEnemyBase>, int32> PoolConfig;
	
	// 同屏单兵种的最大硬上限（防内存爆炸机制）
	UPROPERTY(EditAnywhere, Category = "Pool Settings")
	int32 MaxPoolSizePerClass = 300;

	// 【核心升级2】取怪时，必须告诉对象池：你需要哪种怪！
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AEnemyBase* GetEnemyFromPool(TSubclassOf<AEnemyBase> EnemyClass, FVector SpawnLocation);
	
private:
	// 核心数据结构：每种怪对应一个敌人数组
	UPROPERTY()
	TMap<TSubclassOf<AEnemyBase>, FEnemyArray> EnemyPoolMap;
};
