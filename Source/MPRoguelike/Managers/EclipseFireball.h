#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "EclipseFireball.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

UCLASS()
class MPROGUELIKE_API AEclipseFireball : public AActor
{
	GENERATED_BODY()
    
public:    
	AEclipseFireball();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// ---------------------------------------------------------
	// 核心组件配置
	// ---------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* NiagaraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComp;

	// ---------------------------------------------------------
	// 对象池核心接口 (9个完整参数全部保留！)
	// ---------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void WakeUp(const FTransform& SpawnTransform, float LifeSpan, float Speed, float InCurveSpeed, float InSize, bool bInStopAtEnd, TSubclassOf<class UGameplayEffect> InHitEffectClass, FGameplayEffectSpecHandle InDamageSpec, AActor* InInstigator);

	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void Deactivate();
    
	// ================== 急刹车滞留指令 ==================
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void StopAndLinger();

	// 【极其关键：解决 6 级卡顿的终极密码！全部改为 Unreliable！】
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StopAndLinger();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_WakeUpVisuals(const FTransform& FlatTransform, float Speed, float InCurveSpeed);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Deactivate();

	// ---------------------------------------------------------
	// 战斗与碰撞逻辑
	// ---------------------------------------------------------
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	FGameplayEffectSpecHandle DamageSpecHandle;
	
	UPROPERTY()
	AActor* SkillInstigator;
    
	UPROPERTY()
	TSubclassOf<UGameplayEffect> AdditionalHitEffectClass;

	FTimerHandle DeactivateTimerHandle;
    
	float CurrentCurveSpeed = 0.0f;

	bool bStopAtEnd = false;
};