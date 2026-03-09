// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeManagerComponent.h"
#include "Math/UnrealMathUtility.h"  // 包含数学函数库，例如 FMath::RandRange

// Sets default values for this component's properties
UUpgradeManagerComponent::UUpgradeManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	// 允许这个组件在网络间发送 RPC
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UUpgradeManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// =======================================================
// 核心：动态权重防重复抽卡算法
// =======================================================
TArray<FCardInfo> UUpgradeManagerComponent::DrawCards(int32 DrawCount)
{
	TArray<FCardInfo> ResultCards;
	
	// 没配数据表就直接返回空
	if (!CardPoolTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("抽卡失败：未配置 CardPoolTable!"));
		return ResultCards;
	}

	// 1. 把数据表里的所有行（卡牌）提取出来
	TArray<FCardInfo*> AllCards;
	CardPoolTable->GetAllRows<FCardInfo>(TEXT("DrawCardsContext"), AllCards);

	// 临时结构体：用于存储本轮有资格被抽取的卡片及其动态权重
	struct FCandidate
	{
		FCardInfo* Card;
		int32 CurrentWeight;
	};
	
	TArray<FCandidate> Candidates;
	int32 TotalWeight = 0; // 轮盘的总大小

	// 2. 动态权重计算与过滤 (O(N) 遍历)
	for (FCardInfo* Card : AllCards)
	{
		int32 CurrentLevel = 0;
		if (OwnedCards.Contains(Card->CardID))
		{
			CurrentLevel = OwnedCards[Card->CardID];
		}

		// 【机制A：满级剔除】如果已经满级，直接不加入候补池
		if (CurrentLevel >= Card->MaxLevel)
		{
			continue; 
		}
		
		// 【机制：专属隔离】如果不是通用卡，且玩家还没拥有它，直接不给抽！
		if (!Card->bIsGenericPool && CurrentLevel == 0)
		{
			continue;
		}

		// 【机制B：动态加权】计算这张卡当前的实际权重
		int32 FinalWeight = Card->BaseWeight;
		if (CurrentLevel > 0)
		{
			// 如果玩家已经拥有这张卡，权重乘以3！鼓励玩家升满级！
			FinalWeight *= 3; 
		}

		// 放入候补池，并累加总权重
		Candidates.Add({Card, FinalWeight});
		TotalWeight += FinalWeight;
	}

	// 3. 轮盘赌抽取（防重复）
	for (int32 i = 0; i < DrawCount; ++i)
	{
		// 如果卡池干了，或者权重异常，提前结束
		if (Candidates.Num() == 0 || TotalWeight <= 0) break;

		// 掷骰子：在 1 到 总权重 之间生成一个随机数
		int32 RandomValue = FMath::RandRange(1, TotalWeight);
		int32 CurrentSum = 0;

		// 拨动轮盘，看随机数落在哪张卡的区间里
		for (int32 j = 0; j < Candidates.Num(); ++j)
		{
			CurrentSum += Candidates[j].CurrentWeight;
			if (RandomValue <= CurrentSum)
			{
				// 抽中了！加入结果数组
				ResultCards.Add(*(Candidates[j].Card));

				// 【机制C：防重复抽卡】把它从候补池中拿走，同时扣除总权重
				TotalWeight -= Candidates[j].CurrentWeight;
				Candidates.RemoveAt(j);
				break; // 结束这次寻找，开始抽下一张
			}
		}
	}

	return ResultCards;
}

// =======================================================
// 玩家选卡后的结算
// =======================================================
void UUpgradeManagerComponent::SelectCard(FName SelectedCardID)
{
	// 如果拥有，等级+1；如果未拥有，添加进 Map 且等级设为 1
	int32& Level = OwnedCards.FindOrAdd(SelectedCardID);
	Level++;
	
	// 如果是服务器在执行这个函数，就打电话通知客户端同步数据！
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Client_SyncCard(SelectedCardID, Level);
	}
}

int32 UUpgradeManagerComponent::GetCardCurrentLevel(FName CardID) const
{
	if (const int32* FoundLevel = OwnedCards.Find(CardID))
	{
		return *FoundLevel;
	}
	return 0; // 如果没找到，说明玩家还没拿过这张卡，等级是 0
}

// 客户端接到电话后执行的具体动作
void UUpgradeManagerComponent::Client_SyncCard_Implementation(FName CardID, int32 NewLevel)
{
	// 客户端乖乖把服务器发来的最新等级，更新到自己的 Map 里
	OwnedCards.Add(CardID, NewLevel);
}