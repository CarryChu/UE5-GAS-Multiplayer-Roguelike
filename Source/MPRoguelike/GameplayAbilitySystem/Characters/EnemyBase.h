// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "EnemyBase.generated.h"

class UAbilitySystemComponent;
class UBasicAttributeSet;

UCLASS()
class MPROGUELIKE_API AEnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

	// 必须实现的 GAS 接口
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// 从对象池中唤醒（重生）
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pool")
	void WakeUp(FVector Location);
	virtual void WakeUp_Implementation(FVector Location);
    
	// 死亡或回收时进入休眠
    UFUNCTION(BlueprintCallable, Category = "Pool")
    virtual void GoToSleep();

	// 在生成时调用，确保初始状态正确
	virtual void PostInitializeComponents() override;
    
	// 将状态变量改为 RepNotify
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsSleeping, Category = "Pool")
	bool bIsSleeping = true; // 注意这里初始值改成 true

	// 当 bIsSleeping 从服务器同步过来时，客户端会自动执行这个函数
	UFUNCTION()
	void OnRep_IsSleeping();

	// 声明网络复制函数（必须有）
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Transient)
	UBasicAttributeSet* AttributeSet;

	virtual void OnSlowTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	// 存储小怪当前锁定的目标玩家
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	AActor* TargetPlayer;

	// 搜寻玩家的定时器句柄
	FTimerHandle TimerHandle_FindPlayer;

	// 搜寻最近玩家的函数 (对应你蓝图里的 FindClosestPlayer)
	void FindClosestPlayer();
	
	// C++ 接收 GAS 属性变化的底层回调
	virtual void HealthChangedCallback(const struct FOnAttributeChangeData& Data);
	
public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// 这会在蓝图里自动生成一个叫 "Event On Health Changed" 的红头事件节点！
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|UI")
	void OnHealthChanged(float OldValue, float NewValue);
	
	// 专门让客户端在醒来时刷新 UI 的事件！
	UFUNCTION(BlueprintImplementableEvent, Category = "Pool|UI")
	void OnClientWakeUpUI();
	
	// 服务器专用的大喇叭：向所有客户端广播“这只怪挨打了”
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitReact(AActor* DamageCauser);
	
	// 供 C++ 呼叫蓝图的受击卡肉与击退事件，带上伤害来源（凶手）
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnTakeHitReact(AActor* DamageCauser);
};
