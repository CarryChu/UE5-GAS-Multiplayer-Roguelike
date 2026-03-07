// 在项目设置的描述页面中填写您的版权声明。

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 游戏玩法能力系统的基本属性集。
 * 此类定义了核心属性，如生命值、攻击力等，具有网络复制(Replication)和通知支持。
 */
UCLASS()
class MPROGUELIKE_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	
	UBasicAttributeSet();
	
	// ==========================================
	// 生存属性 (Survival Attributes)
	// ==========================================

	// 角色的当前生命值。复制并在变化时通知。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Survival", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health)
	
	// 角色的最大生命值。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Survival", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth)

	// 角色的护甲/防御力。用于按比例或固定值减免受到的伤害。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Survival", ReplicatedUsing=OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Armor)

	// ==========================================
	// 战斗属性 (Combat Attributes)
	// ==========================================

	// 攻击范围。决定战士的近战判定半径，或法师火球的最大飞行距离。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_AttackRange)
	FGameplayAttributeData AttackRange;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, AttackRange)
	
	// 基础攻击力。决定了角色造成的初始伤害。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, AttackPower)

	// 攻击速度。默认1.0。例如设为2.0表示攻速翻倍（自动攻击的定时器间隔减半）。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, AttackSpeed)

	// ==========================================
	// 移动属性 (Mobility Attributes)
	// ==========================================

	// 移动速度。用于动态修改角色的最大行走速度。
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Mobility", ReplicatedUsing=OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MovementSpeed)

public:
	// ==========================================
	// 网络复制通知函数 (RepNotify Functions)
	// ==========================================

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Health, OldValue);
	}
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxHealth, OldValue);
	}

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Armor, OldValue);
	}

	UFUNCTION()
	void OnRep_AttackRange(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, AttackRange, OldValue);
	}
	
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, AttackPower, OldValue);
	}

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, AttackSpeed, OldValue);
	}

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MovementSpeed, OldValue);
	}
};