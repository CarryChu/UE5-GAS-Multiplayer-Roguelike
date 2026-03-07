// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
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
	// BlueprintNativeEvent 允许你在 C++ 写逻辑，同时在蓝图里也能作为红节点触发
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pool")
	void WakeUp(FVector Location);

	// C++ 层的实现必须加 _Implementation 后缀
	virtual void WakeUp_Implementation(FVector Location);
    
    // 死亡或回收时进入休眠
    UFUNCTION(BlueprintCallable, Category = "Pool")
    virtual void GoToSleep();
    
    // 标记当前是否处于休眠状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pool")
    bool bIsSleeping = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Transient)
	UBasicAttributeSet* AttributeSet;
	
	// 让所有客户端一起隐身怪物
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_GoToSleep();

	// 让所有客户端一起显示怪物并瞬间传送
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WakeUp(FVector Location);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
