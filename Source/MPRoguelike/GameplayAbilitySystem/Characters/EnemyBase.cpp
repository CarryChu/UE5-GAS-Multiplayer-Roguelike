// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"


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
}

UAbilitySystemComponent* AEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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
	}
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// 怪物休眠（假死）
void AEnemyBase::GoToSleep()
{
	bIsSleeping = true;
	
	// 告诉所有客户端：这只怪死透了，赶紧把它藏起来！
	if (HasAuthority())
	{
		Multicast_GoToSleep();
	}
}

void AEnemyBase::Multicast_GoToSleep_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	GetCharacterMovement()->StopMovementImmediately();
}

// 怪物苏醒（重生）
void AEnemyBase::WakeUp_Implementation(FVector Location)
{
	bIsSleeping = false;

	// 告诉所有客户端：把它摆到正确位置，并解除隐身！
	if (HasAuthority())
	{
		Multicast_WakeUp(Location);
	}
	
}

void AEnemyBase::Multicast_WakeUp_Implementation(FVector Location)
{
	// ★ 极其关键的参数：TeleportPhysics，它会告诉客户端“这是瞬移，别给我搞平滑移动的动画过渡！”
	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
}