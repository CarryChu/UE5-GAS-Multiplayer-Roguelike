// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPoolManager.h"


AEnemyPoolManager::AEnemyPoolManager()
{
	// 管理器本身不需要每帧运行
	PrimaryActorTick.bCanEverTick = false; 
}

void AEnemyPoolManager::BeginPlay()
{
	Super::BeginPlay();

	// 铁律：只有服务器才有资格创建怪物池！
	if (HasAuthority() && EnemyClassToSpawn)
	{
		for (int32 i = 0; i < PoolSize; i++)
		{
			// 设置生成参数：即使重叠也强制生成
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// 生成怪物（统一放在坐标 0,0,0 的地底或者虚空）

			if (AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
			{
				NewEnemy->GoToSleep(); // 生成后立刻让它进入休眠状态
				EnemyPool.Add(NewEnemy); // 塞进数组保管
			}
		}
	}
}

AEnemyBase* AEnemyPoolManager::GetEnemyFromPool(FVector SpawnLocation)
{
	// 只有服务器能分配怪物
	if (!HasAuthority()) return nullptr;

	// 遍历柜子，找一个正在休眠的盘子
	for (AEnemyBase* Enemy : EnemyPool)
	{
		if (Enemy && Enemy->bIsSleeping)
		{
			// 找到了！把它唤醒并传送到指定位置
			Enemy->WakeUp(SpawnLocation);
			return Enemy;
		}
	}
	
	// 2. 危险情况保底：如果 100 只怪全都在场上活着（或者被错误地 Destroy 掉了）
	// 系统为了不让游戏崩溃，只能临时生成新的怪来扩充池子
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	if (AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		EnemyPool.Add(NewEnemy);
		NewEnemy->WakeUp(SpawnLocation); // 生成后直接唤醒投入战斗
		return NewEnemy;
	}

	// 如果100只怪全都在场上（池子空了），返回空指针，这秒钟就不刷怪了
	return nullptr; 
}
