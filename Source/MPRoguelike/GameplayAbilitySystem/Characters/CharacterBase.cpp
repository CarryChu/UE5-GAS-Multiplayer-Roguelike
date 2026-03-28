// 在项目设置的描述页面中填写您的版权声明。

#include "CharacterBase.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/GameplayStatics.h"


// 设置默认值
ACharacterBase::ACharacterBase()
{
	// 设置此角色每帧调用Tick()。如果不需要，可以关闭以提高性能。
	PrimaryActorTick.bCanEverTick = true;

	// 添加能力系统组件
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// 设置能力系统组件为可复制
	AbilitySystemComponent->SetIsReplicated(true);
	// 设置复制模式
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	// 初始化胶囊体大小（半径35.0，高度90.0）
	GetCapsuleComponent()->InitCapsuleSize(35.0F, 90.0f);

	// 禁用控制器旋转（Pitch、Yaw、Roll），角色将根据移动方向旋转
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 设置角色移动组件根据移动方向旋转
	// GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置旋转速率（Yaw为500度/秒）
	// GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// 设置跳跃Z轴速度
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	// 设置空中控制系数
	GetCharacterMovement()->AirControl = .35f;
	// 设置最大步行速度
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	// 设置最小模拟步行速度
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	// 设置步行制动减速度
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	// 设置坠落制动减速度
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


	// 添加基本属性集组件
	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));
}

// 当游戏开始或生成时调用
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

// 当被控制器拥有时调用
void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 如果能力系统组件存在，初始化能力Actor信息
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// 当PlayerState复制时调用
void ACharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 如果能力系统组件存在，初始化能力Actor信息
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// 每帧调用
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 调用以绑定功能到输入
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 获取能力系统组件
UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ACharacterBase::GetIsDowned() const
{
	if (AbilitySystemComponent)
	{
		// 核心：不再读布尔值，而是向 GAS 询问“有没有 State.Downed 这个标签”
		FGameplayTag DownedTag = FGameplayTag::RequestGameplayTag(FName("State.Downed"));
		return AbilitySystemComponent->HasMatchingGameplayTag(DownedTag);
	}
	return false;
}

void ACharacterBase::ServerRevivePlayer()
{
	if (AbilitySystemComponent)
	{
		FGameplayTag DownedTag = FGameplayTag::RequestGameplayTag(FName("State.Downed"));

		// 1. 在服务器本地强行摘除游离标签
		AbilitySystemComponent->RemoveLooseGameplayTag(DownedTag);

		// 2. 强制把这个标签的网络同步数量设为 0！这会瞬间通知所有客机
		AbilitySystemComponent->SetLooseGameplayTagCount(DownedTag, 0);
	}
}

// 开始自动攻击循环
void ACharacterBase::StartAutoAttack()
{
	// 联机安全锁：确保只有服务器（Authority）才能开启攻击定时器，防止客户端作弊多开定时器
	if (HasAuthority())
	{
		// 获取当前攻速（假设AttackSpeed为1.0，则每秒攻击1次；若为2.0，则每0.5秒攻击1次）
		float CurrentAttackInterval = 1.0f;
		if (BasicAttributeSet && BasicAttributeSet->GetAttackSpeed() > 0.0f)
		{
			CurrentAttackInterval = 1.0f / BasicAttributeSet->GetAttackSpeed();
		}

		// 开启循环定时器
		GetWorld()->GetTimerManager().SetTimer(
			AutoAttackTimerHandle, 
			this, 
			&ACharacterBase::TriggerAutoAttack, 
			CurrentAttackInterval, 
			true // true代表循环执行
		);
	}
}

// 定时器触发逻辑
void ACharacterBase::TriggerAutoAttack()
{
	// 每次定时器时间到了，就调用这个函数去执行具体攻击
	PerformAttack();
}

// 子类攻击行为的默认实现（如果子类没重写，就执行这里）
void ACharacterBase::PerformAttack_Implementation()
{
	// 基类不知道自己是法师还是战士，所以这里留空即可，或者打印一句话方便调试
	UE_LOG(LogTemp, Warning, TEXT("Base Character is attacking!"));
}

void ACharacterBase::GiveDefaultAbilities()
{
	// 只有服务器有资格赋予技能！
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (TSubclassOf<UGameplayAbility>& StartupAbility : DefaultAbilities)
		{
			// 将技能装载进角色的 GAS 系统中
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility, 1, INDEX_NONE, this));
		}
	}
}


void ACharacterBase::SwitchSpectateTarget(int32 Direction)
{
	// 1. 安全校验：确保当前世界存在
	if (!GetWorld()) return;

	// 2. 获取全场所有的玩家角色
	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterBase::StaticClass(), AllCharacters);

	// 3. 筛选出“活着”且“不是自己”的队友
	TArray<ACharacterBase*> AliveTeammates;
	for (AActor* Actor : AllCharacters)
	{
		// 安全的向下转型
		ACharacterBase* Character = Cast<ACharacterBase>(Actor);
		
		// 核心判断：指针有效 + 不是自己 + 没有倒地
		if (Character && Character != this && !Character->GetIsDowned())
		{
			AliveTeammates.Add(Character);
		}
	}

	// 4. 如果场上没有活着的队友了，直接返回，不切换
	if (AliveTeammates.Num() == 0) return;

	// 5. 算法核心：计算下一个观战目标的索引 (带防越界处理)
	SpectateIndex += Direction;
	
	// 处理往回切 (PgUp) 时的负数越界
	if (SpectateIndex < 0)
	{
		SpectateIndex = AliveTeammates.Num() - 1;
	}
	// 处理往前切 (PgDn) 时的正数越界 (使用取模运算符)
	else
	{
		SpectateIndex = SpectateIndex % AliveTeammates.Num();
	}

	// 6. 夺取控制权并切换摄像机
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 0.2f 代表切换镜头时的平滑过渡时间
		PC->SetViewTargetWithBlend(AliveTeammates[SpectateIndex], 0.2f);
	}
}

// 只有客户端本地会执行这个函数！
void ACharacterBase::Client_PlayHitCameraShake_Implementation()
{
	// 1. 确保策划配置了震屏类
	if (HitCameraShakeClass)
	{
		// 2. 拿到当前角色的玩家控制器
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// 3. 终极安全锁：确保只震动“本地玩家”的屏幕，绝不乱震！
			if (PC->IsLocalController())
			{
				PC->ClientStartCameraShake(HitCameraShakeClass);
			}
		}
	}
}

void ACharacterBase::Multicast_PlayHitFeedback_Implementation(AActor* DamageInstigator)
{
	BP_OnHitFeedback(DamageInstigator);
}

void ACharacterBase::GetCooldownByTag(FGameplayTag InCooldownTag, float& TimeRemaining, float& TotalDuration) const
{
	// 默认保底值，确保没 CD 时输出 0
	TimeRemaining = 0.f;
	TotalDuration = 0.f;

	// 获取能力系统组件 (ASC)
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    
	if (ASC && InCooldownTag.IsValid())
	{
		// 1. 极其正统的操作：构建一个 GameplayEffect 查询器 (Query)
		FGameplayTagContainer TagContainer(InCooldownTag);
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer);
       
		// 2. 调用只接受 1 个参数 (Query) 的原生 API，获取结果数组
		TArray<float> TimeRemains = ASC->GetActiveEffectsTimeRemaining(Query);
		TArray<float> Durations = ASC->GetActiveEffectsDuration(Query);

		// 3. 极其严谨的遍历：拿到时间最长的那个 CD (防止同时中多个减速/冷却)
		if (TimeRemains.Num() > 0 && Durations.Num() > 0)
		{
			float MaxTime = 0.f;
			int32 MaxIndex = 0;
			for (int32 i = 0; i < TimeRemains.Num(); ++i)
			{
				if (TimeRemains[i] > MaxTime)
				{
					MaxTime = TimeRemains[i];
					MaxIndex = i;
				}
			}
          
			// 4. 安全注入输出值，传回给蓝图
			if (Durations.IsValidIndex(MaxIndex))
			{
				TimeRemaining = TimeRemains[MaxIndex];
				TotalDuration = Durations[MaxIndex];
			}
		}
	}
}
