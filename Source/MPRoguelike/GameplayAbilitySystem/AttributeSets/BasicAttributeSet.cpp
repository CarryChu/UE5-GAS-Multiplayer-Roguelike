// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	// 赋予所有属性的初始默认值（使用宏生成的 Init 函数）
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitArmor(0.f);
	InitAttackRange(1500.f);
	InitAttackPower(10.f);
	InitAttackSpeed(1.0f);
	InitMovementSpeed(500.f);
}

// 注册所有属性的网络同步（四人联机核心部分）
void UBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// COND_None表示同步给所有人，REPNOTIFY_Always表示即使值没有改变，只要服务器发送了更新，客户端就会触发OnRep函数
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void UBasicAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 如果被改变的属性是 "移动速度" (注意：这里替换成你实际定义的移动速度变量名)
	if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		// 拿到拥有这个属性的角色
		if (ACharacter* AvatarCharacter = Cast<ACharacter>(GetOwningActor()))
		{
			// 把 GAS 里的数字，强行赋值给角色的双腿（移动组件）！
			AvatarCharacter->GetCharacterMovement()->MaxWalkSpeed = GetMovementSpeed();
		}
	}
}
