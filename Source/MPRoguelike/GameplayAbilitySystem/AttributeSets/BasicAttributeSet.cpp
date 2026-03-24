// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MPRoguelike/GameplayAbilitySystem/Characters/CharacterBase.h"
#include "MPRoguelike/GameplayAbilitySystem/Characters/EnemyBase.h"

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

// 统一拦截并处理所有生命值变动
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
		// 把血量锁死在 0 和 MaxHealth 之间
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		// -----------------------------------------------------
		// 1. 判断受击者是不是【小怪 (EnemyBase)】
		// -----------------------------------------------------
		if (AEnemyBase* EnemyAvatar = Cast<AEnemyBase>(GetOwningActor()))
		{
			// 如果小怪还活着，处理受击顿帧与击退
			if (GetHealth() > 0.0f)
			{
				// 获取造成这次伤害的 GE 身上带的所有标签
				FGameplayTagContainer EffectTags;
				Data.EffectSpec.GetAllAssetTags(EffectTags);
				
				// 获取我们的 DoT 标签
				FGameplayTag DoTTag = FGameplayTag::RequestGameplayTag(FName("Damage.DoT"));

				// 核心过滤：如果这个伤害【不包含】 DoT 标签，才允许触发卡肉！
				if (!EffectTags.HasTag(DoTTag))
				{
					// 从 GAS 上下文中抓出“是谁造成了伤害” (Instigator)
					AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
					
					// 呼叫小怪受击事件，并把凶手传过去
					EnemyAvatar->Multicast_PlayHitReact(Instigator);
				}
			}
		}
		
		// -----------------------------------------------------
		// 2. 判断受击者是不是【玩家 (CharacterBase)】
		// -----------------------------------------------------
		else if (ACharacterBase* AvatarCharacter = Cast<ACharacterBase>(GetOwningActor()))
		{
			// 判断血量是否为零或以下 (玩家死亡逻辑保持不变)
			if (GetHealth() <= 0.0f)
			{
				if (AvatarCharacter->HasAuthority() && !AvatarCharacter->GetIsDowned())
				{
					FGameplayTag DownedTag = FGameplayTag::RequestGameplayTag(FName("State.Downed"));
					AvatarCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(DownedTag);
					AvatarCharacter->GetAbilitySystemComponent()->SetLooseGameplayTagCount(DownedTag, 1);
					AvatarCharacter->OnServerHealthZero();
				}
			}
		}
	}
}
