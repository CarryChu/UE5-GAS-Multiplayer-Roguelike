// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPassiveAbilityBase.h"
#include "AbilitySystemComponent.h"

UMPPassiveAbilityBase::UMPPassiveAbilityBase()
{
	// 被动/自动攻击技能默认配置：每个 Actor 实例化，且仅在服务器端执行
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	// ★ 核心设置：当技能被赋予时，自动激活自己！这个标志会让 GAS 在赋予技能后自动调用 OnAvatarSet 来尝试激活技能
	bActivateAbilityOnGranted = true;
}

void UMPPassiveAbilityBase::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// 第一重锁：判断开关是否开启
	if (bActivateAbilityOnGranted)
	{
		// 第二重锁：确保 ASC 有效且只在服务器端运行
		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() && ActorInfo->IsNetAuthority())
		{
			// 激活技能！

			if (bool bActivated = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false))
			{
				UE_LOG(LogTemp, Log, TEXT("被动技能被赋予，已自动激活: %s"), *GetName());
			}
		}
	}
}