// 在项目设置的描述页面中填写您的版权声明。

#include "CharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"

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
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置旋转速率（Yaw为500度/秒）
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

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
