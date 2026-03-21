// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/Characters/CharacterBase.h"

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
	// 如果被改变的属性是 "移动速度" 
	if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		// 拿到拥有这个属性的角色
		if (ACharacter* AvatarCharacter = Cast<ACharacter>(GetOwningActor()))
		{
			// 把 GAS 里的数字，强行赋值给角色的移动组件！
			AvatarCharacter->GetCharacterMovement()->MaxWalkSpeed = GetMovementSpeed();
		}
	}
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 严谨起见，把血量锁死在 0 和 MaxHealth 之间
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		// 一旦血量归零！
		if (GetHealth() <= 0.0f)
		{
			// 获取到这具身体
			if (ACharacterBase* AvatarCharacter = Cast<ACharacterBase>(GetOwningActor()))
			{
				// 只有服务器法官才有资格宣判死亡！
				if (AvatarCharacter->HasAuthority() && !AvatarCharacter->GetIsDowned())
				{
					// 获取倒地标签
					FGameplayTag DownedTag = FGameplayTag::RequestGameplayTag(FName("State.Downed"));
    
					// 1. 先在本地强行加上游离标签
					AvatarCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(DownedTag);

					// 2. 然后手动把这个标签的数量设为 1，并强制同步给所有客户端
					AvatarCharacter->GetAbilitySystemComponent()->SetLooseGameplayTagCount(DownedTag, 1);
					
					// 呼叫蓝图里的死亡事件
					AvatarCharacter->OnServerHealthZero();
				}
			}
		}
	}
}
