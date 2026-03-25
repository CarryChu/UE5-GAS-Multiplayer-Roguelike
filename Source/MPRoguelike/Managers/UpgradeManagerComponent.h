// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "UpgradeManagerComponent.generated.h"

// 定义一个本地多播委托，带两个参数（技能ID和最新等级）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillUpgradedSignature, FName, SkillID, int32, NewLevel);

// 1. 定义卡牌的类型
UENUM(BlueprintType)
enum class ECardType : uint8
{
	Weapon		UMETA(DisplayName = "武器/主动技能"),
	Passive		UMETA(DisplayName = "被动增益"),
	Consumable	UMETA(DisplayName = "消耗品(如回血/给金币)")
};

// 2. 单级卡牌的详细数据（每一级赋予什么属性）
USTRUCT(BlueprintType)
struct FCardLevelInfo
{
	GENERATED_BODY()
	
public:
	// 这一级的 UI 描述（给玩家看的）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Level")
	FText LevelDescription;
	
	// 质变时的专属名字（可选，如果不填就用基础名字）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Level|Evolution")
	FText OverrideName;

	// 质变时的专属图标（可选，如果不填就用基础图标）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Level|Evolution")
	UTexture2D* OverrideIcon = nullptr;
	
	// 这一级对应的 Gameplay Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Level")
	TSubclassOf<UGameplayEffect> LevelEffect;
};

// 3. 一整张卡牌的基础配置表（策划填表用的结构）
USTRUCT(BlueprintType)
struct FCardInfo : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 卡牌唯一ID (例如 "Skill_Fireball")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
	FName CardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
	FText CardName;
	
	// 卡牌的基础机制介绍
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info", meta = (MultiLine = "true"))
	FText CardDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
	ECardType CardType= ECardType::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
	UTexture2D* CardIcon = nullptr;
	
	// 是否属于通用卡池？
	// 如果为 false，则只有玩家已经拥有该卡时，才能在升级中抽到它（完美实现职业专属专属隔离）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Algorithm")
	bool bIsGenericPool = true; // 默认打勾

	// 基础抽取权重（数值越大，越容易被抽中）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Algorithm")
	int32 BaseWeight = 100;

	// 这张卡的等级上限（无尽狂潮里通常是 5 级满级）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Algorithm")
	int32 MaxLevel = 5;

	// 每一级的具体数据 (索引0代表Lv1，索引1代表Lv2...)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Levels")
	TArray<FCardLevelInfo> LevelInfos;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MPROGUELIKE_API UUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUpgradeManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// ===================================================
	// 抽卡核心系统变量
	// ===================================================

	// 策划配置好的卡池数据表 (要在蓝图里赋值！)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade System")
	UDataTable* CardPoolTable;

	// 记录玩家当前拥有的卡牌及其等级 (Key: CardID, Value: CurrentLevel)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Upgrade System")
	TMap<FName, int32> OwnedCards;

	// ===================================================
	// 抽卡核心系统函数
	// ===================================================

	// 核心抽卡函数：返回指定数量（默认3张）的不重复卡牌
	UFUNCTION(BlueprintCallable, Category = "Upgrade System")
	TArray<FCardInfo> DrawCards(int32 DrawCount = 3);

	// 玩家选择了一张卡后调用：提升等级
	UFUNCTION(BlueprintCallable, Category = "Upgrade System")
	void SelectCard(FName SelectedCardID);
	
	// 服务器专门打给客户端的电话，让客户端自己更新 Map
	UFUNCTION(Client, Reliable)
	void Client_SyncCard(FName CardID, int32 NewLevel);
	
	// 获取玩家拥有的某张卡的当前等级 (没拥有就返回 0)
	UFUNCTION(BlueprintPure, Category = "Upgrade System")
	int32 GetCardCurrentLevel(FName CardID) const;
	
	// 这是一个纯本地的广播频道，蓝图可以监听它！
	UPROPERTY(BlueprintAssignable, Category = "Upgrades")
	FOnSkillUpgradedSignature OnSkillUpgraded;
};
