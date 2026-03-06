// 在项目设置的描述页面中填写您的版权声明。

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 游戏玩法能力系统的基本属性集。
 * 此类定义了核心属性，如健康和耐力，具有复制和通知支持。
 */
UCLASS()
class MPROGUELIKE_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	
	UBasicAttributeSet();
	
	// 角色的当前健康值。复制并在变化时通知。
	UPROPERTY(BlueprintReadOnly, Category="Attributes", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health)
	
	// 角色的最大健康值。复制并在变化时通知。
	UPROPERTY(BlueprintReadOnly, Category="Attributes", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth)
	
	
	// 角色的当前耐力值。复制并在变化时通知。
	UPROPERTY(BlueprintReadOnly, Category="Attributes", ReplicatedUsing=OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Stamina)
	
	// 角色的最大耐力值。复制并在变化时通知。
	UPROPERTY(BlueprintReadOnly, Category="Attributes", ReplicatedUsing=OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxStamina)
	
public:
	// 健康属性变化的复制通知。
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Health, OldValue);
	}
	
	// 最大健康属性变化的复制通知。
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxHealth, OldValue);
	}
	
	// 耐力属性变化的复制通知。
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Stamina, OldValue);
	}
	
	// 最大耐力属性变化的复制通知。
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxStamina, OldValue);
	}
	
	// 重写以指定哪些属性应通过网络复制。
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
