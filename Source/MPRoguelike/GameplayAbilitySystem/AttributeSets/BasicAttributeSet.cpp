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
       if (ACharacter* AvatarCharacter = Cast<ACharacter>(GetOwningActor()))
       {
          AvatarCharacter->GetCharacterMovement()->MaxWalkSpeed = GetMovementSpeed();
       }
    }

    // ====================================================================
    // 🌟 生命值变化处理核心区
    // ====================================================================
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
       // -----------------------------------------------------------------
       // 【绝对防御阀门】：游戏结束/团灭时的无敌拦截！
       // -----------------------------------------------------------------
       FGameplayTag GameOverTag = FGameplayTag::RequestGameplayTag(FName("State.GameOver"));
       
       // 如果玩家身上有这个标签，且这是一次“扣血”行为 (Magnitude < 0)
       if (Data.Target.HasMatchingGameplayTag(GameOverTag) && Data.EvaluatedData.Magnitude < 0.0f)
       {
           // 极其神绝的操作：因为此时血已经被扣了，我们减去一个负数(负负得正)，把血量加回来！
           float RestoredHealth = GetHealth() - Data.EvaluatedData.Magnitude;
           SetHealth(FMath::Clamp(RestoredHealth, 0.0f, GetMaxHealth()));
           
           // 极其无情地拦截：直接踢出函数！后面的受击动画、震屏统统取消！
           return; 
       }
       // -----------------------------------------------------------------

       // 把血量锁死在 0 和 MaxHealth 之间
       SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

       // -----------------------------------------------------
       // 1. 判断受击者是不是【小怪 (EnemyBase)】
       // -----------------------------------------------------
       if (AEnemyBase* EnemyAvatar = Cast<AEnemyBase>(GetOwningActor()))
       {
          if (GetHealth() > 0.0f && Data.EvaluatedData.Magnitude < 0.0f)
          {
             FGameplayTagContainer EffectTags;
             Data.EffectSpec.GetAllAssetTags(EffectTags);
             FGameplayTag DoTTag = FGameplayTag::RequestGameplayTag(FName("Damage.DoT"));

             if (!EffectTags.HasTag(DoTTag))
             {
                AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
                EnemyAvatar->Multicast_PlayHitReact(Instigator);
             }
          }
       }
       
       // -----------------------------------------------------
       // 2. 判断受击者是不是【玩家 (CharacterBase)】
       // -----------------------------------------------------
       else if (ACharacterBase* AvatarCharacter = Cast<ACharacterBase>(GetOwningActor()))
       {
          if (Data.EvaluatedData.Magnitude < 0.0f)
          {
             AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
             AvatarCharacter->Client_PlayHitCameraShake();
             AvatarCharacter->Multicast_PlayHitFeedback(Instigator);
          }
          
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
