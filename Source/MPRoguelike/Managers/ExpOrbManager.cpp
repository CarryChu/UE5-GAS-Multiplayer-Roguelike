/**
 * @file ExpOrbManager.cpp
 * @brief 经验球管理器实现
 * @details 实现经验球的生成、磁吸、拾取和多人同步逻辑
 * @author [Game Team]
 * @version 1.0
 * @date 2026
 */

#include "ExpOrbManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

/**
 * @brief AExpOrbManager构造函数
 * @details 
 * 初始化Manager的复制属性和Tick设置
 * - 启用Tick更新
 * - 设置为可复制（Replicated）
 * - 设置为始终相关（AlwaysRelevant），确保所有客户端都能接收数据
 * - 创建并设置根组件
 */
AExpOrbManager::AExpOrbManager()
{
 	PrimaryActorTick.bCanEverTick = true;  // 启用Tick回调
	bReplicates = true;                     // 启用网络复制
	bAlwaysRelevant = true;                 // 始终相关，所有客户端都能收到更新

	// 创建Scene组件作为根组件
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

/**
 * @brief BeginPlay事件回调
 * @details 游戏开始时调用，用于初始化Manager
 * @note 当前实现为空，可用于未来的初始化需求
 */
void AExpOrbManager::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * @brief 添加经验球到管理系统
 * @param SpawnLocation 球体生成的世界位置
 * @param ExpAmount 球体包含的经验值
 * @details 
 * 此函数只能在服务器上有效执行（HasAuthority检查）
 * 调用多播RPC确保所有客户端都能看到生成的经验球
 * @note 服务器权限检查确保游戏逻辑的一致性和安全性
 * @see Multicast_AddExpOrb
 */
void AExpOrbManager::AddExpOrb(FVector SpawnLocation, float ExpAmount)
{
	if (HasAuthority())  // 仅服务器执行
	{
		Multicast_AddExpOrb(SpawnLocation, ExpAmount);  // 多播给所有客户端
	}
}

/**
 * @brief 多播RPC实现：在所有机器上添加经验球
 * @param SpawnLocation 球体生成位置
 * @param ExpAmount 球体经验值
 * @details 
 * 将经验球数据添加到OrbList数组中
 * Z轴位置向下调整40单位，防止球体卡入地面
 * 该函数通过NetMulticast自动在服务器和所有连接的客户端上执行
 * @note 球体初始没有目标玩家，通过Tick逻辑中的距离检测来获取目标
 */
void AExpOrbManager::Multicast_AddExpOrb_Implementation(FVector SpawnLocation, float ExpAmount)
{
	SpawnLocation.Z -= 40.0f;  // 调整Z轴防止穿地
	OrbList.Add(FExpOrbData(SpawnLocation, ExpAmount));  // 将新经验球添加到列表
}

/**
 * @brief 每帧更新逻辑
 * @param DeltaTime 上一帧的时间增量（单位：秒）
 * @details 
 * 核心逻辑流程：
 * 1. 权限检查：仅在服务器（或单人模式）执行业务逻辑
 * 2. 获取所有玩家对象
 * 3. 遍历经验球列表，更新每个球的状态：
 *    - 如果已有目标玩家：检查拾取范围，若不在磁吸范围则移动到目标
 *    - 如果无目标玩家：扫描所有玩家，找到第一个进入磁吸范围的玩家
 * 4. 移除已拾取的经验球
 * 
 * @note 客户端的球体显示通过复制的OrbList自动同步
 * @warning 核心修复：敌我识别逻辑，跳过非玩家控制的角色（如敌人AI）
 */
void AExpOrbManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ========================================
	// 权限检查：仅在服务器或单人游戏中执行
	// ========================================
	if (!HasAuthority() && GetNetMode() != NM_Standalone) return;

	// 获取所有玩家角色
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Players);
	if (Players.Num() == 0) return;

	// ========================================
	// 遍历经验球列表，更新每个经验球的状态
	// ========================================
	for (int32 i = OrbList.Num() - 1; i >= 0; i--)
	{
		FExpOrbData& Orb = OrbList[i];
		bool bIsPickedUp = false;

		// 如果球已经有目标玩家
		if (Orb.TargetPlayer.IsValid())
		{
			AActor* Player = Orb.TargetPlayer.Get();
			FVector PlayerLoc = Player->GetActorLocation();
			
			// 计算球与玩家之间的距离平方（避免开方操作以提高性能）
			float DistSquared = FVector::DistSquared(Orb.Location, PlayerLoc);

			// 检查是否在拾取范围内
			if (DistSquared <= (PickupRadius * PickupRadius))
			{
				bIsPickedUp = true;  // 标记为已拾取
				if (HasAuthority())
				{
					OnOrbPickedUp(Orb.ExpAmount, Player);  // 触发拾取事件
				}
			}
			else
			{
				// 球体还未到达玩家，继续磁吸移动
				FVector Direction = (PlayerLoc - Orb.Location).GetSafeNormal();
				Orb.Location += Direction * MagnetSpeed * DeltaTime;
			}
		}
		else
		{
			// 球没有目标玩家，扫描所有玩家找到第一个在磁吸范围内的
			for (AActor* Player : Players)
			{
				// ==========================================
				// ★ 核心修复：敌我识别！
				// ==========================================
				// 检查是否是真正的玩家控制的角色
				APawn* Pawn = Cast<APawn>(Player);
				
				// 如果转换成功，并且它不是由真实玩家控制的（比如是小怪 AI），就直接跳过！
				if (Pawn && !Pawn->IsPlayerControlled())
				{
					continue;  // 跳过非玩家控制的对象（如敌人）
				}
				
				FVector PlayerLoc = Player->GetActorLocation();
				
				// 仅在X-Y平面上计算距离（忽略Z轴高度差）
				float DistSquared = FVector::DistSquared(
					FVector(Orb.Location.X, Orb.Location.Y, 0), 
					FVector(PlayerLoc.X, PlayerLoc.Y, 0)
				);

				// 检查是否在磁吸范围内
				if (DistSquared <= (MagnetRadius * MagnetRadius))
				{
					Orb.TargetPlayer = Player;  // 设置目标玩家
					break;  // 找到目标后立即退出循环
				}
			}
		}

		// 如果球已被拾取，从列表中移除（使用RemoveAtSwap以提高性能）
		if (bIsPickedUp)
		{
			OrbList.RemoveAtSwap(i);
		}
	}
}

/**
 * @brief 获取需要复制的属性列表
 * @param OutLifetimeProps 输出参数，包含需要复制的属性信息
 * @details 
 * 告诉Unreal引擎哪些属性需要在网络中复制
 * OrbList包含所有经验球的位置和经验值信息，需要同步到所有客户端
 * 以便客户端能够显示和更新经验球的位置
 * @note 这是Unreal网络复制系统的必要实现
 * @see DOREPLIFETIME
 */
void AExpOrbManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExpOrbManager, OrbList);  // 将OrbList添加到复制列表
}