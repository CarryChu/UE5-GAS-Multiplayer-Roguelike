// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyPoolManager.h"

AEnemyPoolManager::AEnemyPoolManager()
{
	PrimaryActorTick.bCanEverTick = false; 
}

void AEnemyPoolManager::BeginPlay()
{
	Super::BeginPlay();

	// 只有服务器才有资格创建怪物池！
	if (HasAuthority())
	{
		// 遍历蓝图里配置的所有怪物类型和数量
		for (const auto& Pair : PoolConfig)
		{
			TSubclassOf<AEnemyBase> EnemyClass = Pair.Key;
			int32 Amount = Pair.Value;

			// 防呆设计：如果没有选类型，就跳过
			if (!EnemyClass) continue;

			// 为这个兵种新建一个专属的数组，并把它放进字典里
			FEnemyArray& NewPoolArray = EnemyPoolMap.Add(EnemyClass);

			// 根据配置的数量，生成对应数量的怪物，并放进这个数组里
			for (int32 i = 0; i < Amount; i++)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// 生成Enemy
				if (AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
				{
					NewEnemy->GoToSleep(); 
					NewPoolArray.Enemies.Add(NewEnemy); // 装进对应的数组里
				}
			}
		}
	}
}

AEnemyBase* AEnemyPoolManager::GetEnemyFromPool(TSubclassOf<AEnemyBase> EnemyClass, FVector SpawnLocation)
{
	// 只有服务器能分配怪物，而且必须传入有效的Enemy类型
	if (!HasAuthority() || !EnemyClass) return nullptr;

	// 1. 查字典，找到这个怪对应的数组
	if (FEnemyArray* FoundPool = EnemyPoolMap.Find(EnemyClass))
	{
		// 2. 在专属数组里，找一个正在休眠的怪
		for (AEnemyBase* Enemy : FoundPool->Enemies)
		{
			if (Enemy && Enemy->bIsSleeping)
			{
				Enemy->WakeUp(SpawnLocation);
				return Enemy;
			}
		}
		
		// 如果循环结束还没 return，说明没有怪了！
		// 在强行生成新怪扩充池子之前，先检查一下池子是不是已经满了？
		if (FoundPool->Enemies.Num() >= MaxPoolSizePerClass)
		{
			// 触发保护机制：场上这种怪已经达到 300 只了，全都在活跃，拒绝生成新怪！
			// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("警告：触发同屏数量上限，停止刷怪！"));
			return nullptr; 
		}
		
		// 3. 危险保底：这个兵种的池子空了！（要么是配置的数量太少，要么是玩家杀得太快了！）则生成一个新怪，加入池子，并返回它
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		if (AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
		{
			FoundPool->Enemies.Add(NewEnemy); // 加入对应的数组
			NewEnemy->WakeUp(SpawnLocation); 
			return NewEnemy;
		}
	}

	// 如果连数组都没建（你没在 Config 里配这个怪），直接返回空
	return nullptr; 
}