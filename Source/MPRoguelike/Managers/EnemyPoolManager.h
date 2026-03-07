// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MPRoguelike/GameplayAbilitySystem/Characters/EnemyBase.h"
#include "EnemyPoolManager.generated.h"

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
	
	// 在蓝图里指定的怪物类型
	UPROPERTY(EditAnywhere, Category = "Pool Settings")
	TSubclassOf<AEnemyBase> EnemyClassToSpawn;

	// 池子的容量（一次性生成多少只）
	UPROPERTY(EditAnywhere, Category = "Pool Settings")
	int32 PoolSize = 100;

	// 蓝图调用：从池子里获取一只可用的怪物
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AEnemyBase* GetEnemyFromPool(FVector SpawnLocation);
	
private:
	// 这就是我们用来装怪物的“柜子”
	UPROPERTY()
	TArray<AEnemyBase*> EnemyPool;
};
