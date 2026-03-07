// 在项目设置的描述页面中填写您的版权声明。

#include "CharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

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