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
	
public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
