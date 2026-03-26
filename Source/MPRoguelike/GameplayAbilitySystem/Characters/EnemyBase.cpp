// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "CharacterBase.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"


class AAIController;
// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true; // 允许每帧运行，我们需要它来寻路

	bUseControllerRotationYaw = false; // 禁用控制器Yaw旋转
	GetCharacterMovement()->bOrientRotationToMovement = true; // 开启面向移动方向
	
	// 创建 GAS 组件
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// 怪物通常用 Minimal 复制模式以节省网络带宽
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal); 

	// 创建属性集
	AttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("AttributeSet"));
	
	bIsSleeping = true; // 默认就是睡着的
	
	// 在构造时就直接隐藏、关碰撞、关Tick！
	AActor::SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	AActor::SetActorTickEnabled(false);
}

UAbilitySystemComponent* AEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// 在组件完全初始化后调用，确保初始状态正确（包括复制）
void AEnemyBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// 敌人生成时默认是休眠的，所以初始隐藏
	// 这样做确保在网络复制之前就是正确的状态
	if (bIsSleeping)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		GetCharacterMovement()->StopMovementImmediately();
	}
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化怪物的 GAS 系统！
	if (AbilitySystemComponent)
	{
		// 对于普通怪物，Owner 和 Avatar 都是它自己 (this, this)
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// 给怪物自己装上窃听器！盯死 Debuff.Slow 这个标签！
		FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(TEXT("Debuff.Slow"));
		AbilitySystemComponent->RegisterGameplayTagEvent(SlowTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AEnemyBase::OnSlowTagChanged);
		
		// 监听血量变化
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBasicAttributeSet::GetHealthAttribute()).AddUObject(this, &AEnemyBase::HealthChangedCallback);
		
	}
	
	// 只有服务器需要找玩家并驱动AI
	if (HasAuthority())
	{
		// 每 0.5 秒执行一次 FindClosestPlayer 函数，且循环执行
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_FindPlayer, this, &AEnemyBase::FindClosestPlayer, 0.5f, true);
	}
}

void AEnemyBase::FindClosestPlayer()
{
	// 只有服务器才需要找目标
	if (!HasAuthority() || bIsSleeping) return;

	AActor* ClosestPlayer = nullptr;
	// 设置一个初始的最大距离（等同于你蓝图里的 9999999.0）
	float MinDistanceSq = MAX_FLT; 

	// 高性能写法：遍历当前世界上的所有玩家（最多就4个，极速！）
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn())
		{
			APawn* PlayerPawn = PC->GetPawn();

			// 【可选】：如果你有自己的玩家基类（比如 ACharacterBase），
			// 你可以在这里 Cast 一下，判断玩家是不是处于倒地/死亡状态 (GetIsDowned)
			 if (ACharacterBase* MyPlayer = Cast<ACharacterBase>(PlayerPawn))
			 {
			     if (MyPlayer->GetIsDowned()) continue; // 如果死了，就跳过不追他
			 }

			// 计算距离的平方（等同于蓝图的 Get Squared Distance To）
			float DistSq = FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation());
			
			// 如果距离比当前记录的最小距离还要小，就更新它
			if (DistSq < MinDistanceSq)
			{
				MinDistanceSq = DistSq;
				ClosestPlayer = PlayerPawn;
			}
		}
	}

	// 循环结束后，把找到的最近玩家存入大脑！
	TargetPlayer = ClosestPlayer;

	// 如果锁定了目标，且怪物没有在Sleep
	if (TargetPlayer && !bIsSleeping)
	{
		// 获取怪物头上的 AI 控制器
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			// 下达指令：自动计算避障路径并移动向玩家！
			// 50.0f 是接受半径，意思是离玩家 50 个单位时就停下，防止怪物贴脸穿模
			AIController->MoveToActor(TargetPlayer, 50.0f); 
		}
	}
}

// 执行每帧逻辑
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSleeping || !HasAuthority()) return;

	// 拦截逻辑：是否正在攻击？
	if (AbilitySystemComponent)
	{
		FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"));
		if (AbilitySystemComponent->HasMatchingGameplayTag(AttackTag))
		{
			// 【核心修改】：如果有攻击标签，立刻命令 AI 控制器刹车！
			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				AIController->StopMovement();
			}
			return; // 攻击时站定不动
		}
	}

}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
// 1. 注册要同步的变量
void AEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 告诉引擎同步这个变量
	DOREPLIFETIME(AEnemyBase, bIsSleeping); 
}

// 2. 核心隐身/显身逻辑（服务器和客户端都会跑这个）
void AEnemyBase::OnRep_IsSleeping()
{
	if (bIsSleeping)
	{
		// Sleep：隐藏、关碰撞、关Tick
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		// 彻底封杀移动组件，断绝一切网络平滑和本地预测！
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();          // 禁用移动
			MoveComp->SetComponentTickEnabled(false); // 停止组件更新
		}
	}
	else
	{
		// WakeUp：显示、开碰撞、开Tick
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);
		// 重启移动组件，让怪物能够正常寻路
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking); // 恢复为寻路/行走模式
			MoveComp->SetComponentTickEnabled(true);
		}
		OnClientWakeUpUI();
	}
}

// GoToSleep
void AEnemyBase::GoToSleep()
{
	if (HasAuthority())
	{
		bIsSleeping = true;
		// 怪物一旦死亡，立刻把它传送到地下 10000 米深处！
		// 这样即使客户端在进行网络平滑插值，也绝对不会在玩家的屏幕上留下任何残影！
		SetActorLocation(FVector(0.f, 0.f, -10000.f), false, nullptr, ETeleportType::TeleportPhysics);
		OnRep_IsSleeping(); // 服务器本地手动调用一次
	}
}

// 4. 修改 WakeUp 的 C++ 核心实现
void AEnemyBase::WakeUp_Implementation(FVector Location)
{
	if (HasAuthority()) // 确保只有服务器能改状态
	{
		bIsSleeping = false;
		// 瞬间传送
		SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
       
		if (AbilitySystemComponent)
		{
			// 👉 【绝杀修正】：打断所有正在释放的技能（这会自动清除 State.Attacking 这种施法标签）！
			// 绝对不要用 ClearAllAbilities()！
			AbilitySystemComponent->CancelAllAbilities();
          
			// 洗清所有残留的减速/持续伤害等负面状态
			FGameplayTagContainer TagsToRemove;
			TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Debuff")));
			AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
		}
		OnRep_IsSleeping(); // 服务器本地手动调用一次
	}
}

// 怪物被挂上标签后，根据标签改变移动速度
void AEnemyBase::OnSlowTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 确保移动组件和属性集都存在
	if (GetCharacterMovement() && AttributeSet)
	{
		float BaseSpeed = AttributeSet->GetMovementSpeed();

		if (NewCount > 0)
		{
			// 被打中了，减速到 70%！
			GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * 0.7f;

		}
		else
		{
			// 2秒到了，标签消失，恢复原速！
			GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		}
	}
}

// 实现回调逻辑
void AEnemyBase::HealthChangedCallback(const FOnAttributeChangeData& Data)
{
	// 当血量发生变化时，C++ 会立刻呼叫蓝图里的事件，并把新旧血量传过去！
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

// 当这只怪收到全服广播时，执行这个逻辑
void AEnemyBase::Multicast_PlayHitReact_Implementation(AActor* DamageCauser)
{
	// 呼叫蓝图，所有的客户端（包括服务器本身）都会运行这里的蓝图节点！
	OnTakeHitReact(DamageCauser);
}