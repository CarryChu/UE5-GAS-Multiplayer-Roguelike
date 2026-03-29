#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExpOrbManager.generated.h"

/**
 * @struct FExpOrbData
 * @brief 经验球数据结构
 * @details 定义经验球的属性，包括位置、经验值和目标玩家信息
 * @author [Game Team]
 * @version 1.0
 * @date 2026
 */
USTRUCT(BlueprintType)
struct FExpOrbData
{
	GENERATED_BODY()

	/** @brief 经验球在世界中的位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location;

	/** @brief 经验球包含的经验值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ExpAmount;

	/** @brief 指向目标玩家的弱指针，防止循环引用 */
	TWeakObjectPtr<AActor> TargetPlayer;

	/** @brief 默认构造函数 */
	FExpOrbData() : Location(FVector::ZeroVector), ExpAmount(0.0f) {}

	/** @brief 参数化构造函数 */
	FExpOrbData(FVector Loc, float Exp) : Location(Loc), ExpAmount(Exp) {}
};

/**
 * @class AExpOrbManager
 * @brief 经验球管理器
 * @details 
 * 负责管理游戏中的经验球系统：
 * - 创建和生成经验球
 * - 处理经验球的磁吸机制（Magnet）
 * - 检测玩家拾取
 * - 在多人游戏中同步经验球数据
 * 
 * @note 该类在服务器上拥有权限，通过Replication将数据同步到客户端
 * @see FExpOrbData
 * @author [Game Team]
 * @version 1.0
 */
UCLASS()
class MPROGUELIKE_API AExpOrbManager : public AActor
{
	GENERATED_BODY()
	
public:	
	/** @brief 构造函数，初始化Actor组件 */
	AExpOrbManager();

protected:
	/** @brief 游戏开始时调用，初始化Manager */
	virtual void BeginPlay() override;

public:	
	/** @brief 每帧调用，更新经验球位置和检测拾取 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief 添加经验球到管理器
	 * @param SpawnLocation 生成位置
	 * @param ExpAmount 经验值数量
	 * @note 只能在服务器上调用，会自动同步到所有客户端
	 * @see Multicast_AddExpOrb
	 */
	UFUNCTION(BlueprintCallable, Category = "ExpOrb System")
	void AddExpOrb(FVector SpawnLocation, float ExpAmount);

	/**
	 * @brief 多播RPC：在所有客户端和服务器上生成经验球
	 * @param SpawnLocation 生成位置
	 * @param ExpAmount 经验值数量
	 * @note 该函数由AddExpOrb调用，确保数据同步
	 * @details NetMulticast确保在服务器和所有连接的客户端上执行
	 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddExpOrb(FVector SpawnLocation, float ExpAmount);

	/**
	 * @brief 当玩家拾取经验球时触发的事件（蓝图实现）
	 * @param ExpAmount 拾取的经验值
	 * @param Picker 拾取玩家的引用
	 * @note 由C++代码调用，具体逻辑由蓝图实现
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ExpOrb System")
	void OnOrbPickedUp(float ExpAmount, AActor* Picker);

	/**
	 * @brief 拾取半径
	 * @details 当经验球进入该半径范围内时，玩家可以直接拾取
	 * @note 单位为虚幻引擎单位（1 UU ≈ 1cm）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpOrb Settings")
	float PickupRadius = 80.0f;

	/**
	 * @brief 磁吸半径
	 * @details 当经验球进入该半径范围内时，会自动飞向玩家
	 * @note 单位为虚幻引擎单位，应大于PickupRadius
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpOrb Settings")
	float MagnetRadius = 300.0f;

	/**
	 * @brief 磁吸速度
	 * @details 经验球被吸向玩家时的移动速度
	 * @note 单位为虚幻引擎单位/秒
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpOrb Settings")
	float MagnetSpeed = 1000.0f;
	
	/**
	 * @brief 经验球列表
	 * @details 存储所有活跃的经验球数据
	 * @note 该属性被复制到所有客户端，确保多人游戏中的同步
	 * @see FExpOrbData
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ExpOrb System")
	TArray<FExpOrbData> OrbList;
	
};