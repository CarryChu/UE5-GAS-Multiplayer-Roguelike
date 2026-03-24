// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "CharacterBase.generated.h"

UCLASS()
class MPROGUELIKE_API ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();
	
	// Ability System Component
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AbilitySystem")
	class UBasicAttributeSet* BasicAttributeSet;
	
	// 在蓝图里配置角色天生自带的技能
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	// 蓝图可调用的：赋予技能函数
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void GiveDefaultAbilities();

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;
	
	// 用于控制自动攻击的定时器句柄
	FTimerHandle AutoAttackTimerHandle;
	
	// 启动自动攻击循环（由服务器端调用）
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAutoAttack();
	
	// 定时器每次触发时执行的函数
	UFUNCTION()
	void TriggerAutoAttack();
	
	
	// C++只负责按时喊口号，具体的“发射火球”还是“挥剑”，在法师/战士蓝图里连节点！
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void PerformAttack();
	virtual void PerformAttack_Implementation();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// 供 C++ 和蓝图调用的接口：通过 GAS 标签获取是否处于 Downed 状态
	UFUNCTION(BlueprintPure, Category = "State")
	bool GetIsDowned() const;
	
	// 供服务端调用的接口：复活玩家（移除倒地标签）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "State")
	void ServerRevivePlayer();
	
	// 当服务器端的 GAS 系统确认玩家血量归零时，触发这个事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnServerHealthZero();
	
	// 记录当前正在观战的队友索引
	UPROPERTY(BlueprintReadWrite, Category = "Spectate")
	int32 SpectateIndex = 0;

	// 核心观战切换函数 (暴露给蓝图的 PgUp / PgDn 调用)
	UFUNCTION(BlueprintCallable, Category = "Spectate")
	void SwitchSpectateTarget(int32 Direction);
	
};
