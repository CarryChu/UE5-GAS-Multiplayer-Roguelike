// Fill out your copyright notice in the Description page of Project Settings.

#include "ExpOrbManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AExpOrbManager::AExpOrbManager()
{
	PrimaryActorTick.bCanEverTick = true;  
    
	// ★ 必须加上这两行！保证管理器在网络中存在
	bReplicates = true;
	bAlwaysRelevant = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AExpOrbManager::BeginPlay()
{
	Super::BeginPlay();
}

void AExpOrbManager::AddExpOrb(FVector SpawnLocation, float ExpAmount)
{
	if (HasAuthority())
	{
		Multicast_AddExpOrb(SpawnLocation, ExpAmount);
	}
}

void AExpOrbManager::Multicast_AddExpOrb_Implementation(FVector SpawnLocation, float ExpAmount)
{

	SpawnLocation.Z -= 40.0f; 

	// 只保留最核心的数据层操作
	OrbList.Add(FExpOrbData(SpawnLocation, ExpAmount));
}

void AExpOrbManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Players);
    if (Players.Num() == 0) return;

    for (int32 i = OrbList.Num() - 1; i >= 0; i--)
    {
       FExpOrbData& Orb = OrbList[i];
       bool bIsPickedUp = false;

       // 如果检测到玩家则锁定目标，开始追踪（进入 Tracking 态）
       // 如果已经锁定目标了，就继续追踪直到被拾取
       if (Orb.TargetPlayer.IsValid())
       {
          AActor* Player = Orb.TargetPlayer.Get();
          FVector PlayerLoc = Player->GetActorLocation();
          float DistSquared = FVector::DistSquared(Orb.Location, PlayerLoc);

          // 判定是否达到拾取半径
          if (DistSquared <= (PickupRadius * PickupRadius))
          {
             bIsPickedUp = true;
             if (HasAuthority())
             {
                // 触发加经验逻辑
                // TODO: GameState AddSharedExp
             }
          }
          else
          {
             // 尚未拾取，计算插值与方向，向玩家飞行
             FVector Direction = (PlayerLoc - Orb.Location).GetSafeNormal();
             Orb.Location += Direction * MagnetSpeed * DeltaTime;
             
          }
       }
       else
       {
          // 如果处于游离状态（Idle 态），寻找最近的玩家
          for (AActor* Player : Players)
          {
             FVector PlayerLoc = Player->GetActorLocation();
             // 忽略高度差，纯测算平面距离
             float DistSquared = FVector::DistSquared(FVector(Orb.Location.X, Orb.Location.Y, 0), FVector(PlayerLoc.X, PlayerLoc.Y, 0));

             // 判定是否进入磁吸感应半径
             if (DistSquared <= (MagnetRadius * MagnetRadius))
             {
                Orb.TargetPlayer = Player; // ★ 核心：认主！状态切换至 Tracking
                break; 
             }
          }
       }

       // 如果已经被拾取，执行回收机制
       if (bIsPickedUp)
       {
          // （注：如果是独立Actor，这里通常会调用 OrbActor->Destroy()）
          OrbList.RemoveAtSwap(i); 
       }
    }
}