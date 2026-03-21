// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MPPassiveAbilityBase.generated.h"

/**
 * 专为肉鸽游戏设计的被动/自动触发技能基类
 * 特性：在技能被赋予给玩家时，自动在服务器端激活，无需外部事件触发。
 */
UCLASS()
class MPROGUELIKE_API UMPPassiveAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMPPassiveAbilityBase();
	
	// 控制技能是否在被赋予时自动激活
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Activation")
	bool bActivateAbilityOnGranted;

	// 核心生命周期重写：当技能的 Avatar（宿主）被设置时调用
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
