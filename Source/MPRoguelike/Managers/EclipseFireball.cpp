#include "EclipseFireball.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AEclipseFireball::AEclipseFireball()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AEclipseFireball::OnOverlapBegin);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(RootComponent);
	NiagaraComp->SetAutoActivate(false);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->SetAutoActivate(false);
	ProjectileMovementComp->ProjectileGravityScale = 0.f; 
	
	ProjectileMovementComp->bRotationFollowsVelocity = true;
}

void AEclipseFireball::BeginPlay()
{
	Super::BeginPlay();
	Deactivate(); 
}

// =========================================================
// 螺旋制导核心引擎！
// =========================================================
void AEclipseFireball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 只要有角速度，并且物理组件在运行，就围绕Z轴旋转它的速度向量！
	if (CurrentCurveSpeed != 0.0f && ProjectileMovementComp && ProjectileMovementComp->IsActive())
	{
		FVector CurrentVelocity = ProjectileMovementComp->Velocity;
		FVector NewVelocity = CurrentVelocity.RotateAngleAxis(CurrentCurveSpeed * DeltaTime, FVector::UpVector);
		ProjectileMovementComp->Velocity = NewVelocity;
	}
}

// =========================================================
// 主控端唤醒
// =========================================================
void AEclipseFireball::WakeUp(const FTransform& SpawnTransform, float LifeSpan, float Speed, float InCurveSpeed, float InSize, bool bInStopAtEnd, TSubclassOf<class UGameplayEffect> InHitEffectClass, FGameplayEffectSpecHandle InDamageSpec, AActor* InInstigator)
{
	if (!HasAuthority()) return;

	DamageSpecHandle = InDamageSpec;
	SkillInstigator = InInstigator;
	bStopAtEnd = bInStopAtEnd;
	AdditionalHitEffectClass = InHitEffectClass;
	CurrentCurveSpeed = InCurveSpeed;

	FRotator FlatRotation = SpawnTransform.Rotator();
	FlatRotation.Pitch = 0.0f;
	FlatRotation.Roll = 0.0f;
	FTransform FlatTransform = SpawnTransform;
	FlatTransform.SetRotation(FlatRotation.Quaternion());
	
	// 设置体积缩放 (Lv.2 机制)
	FlatTransform.SetScale3D(FVector(InSize));

	// 生命分流 (Lv.6 机制)
	if (bStopAtEnd)
	{
		// 飞到极限距离时，执行悬停
		GetWorldTimerManager().SetTimer(DeactivateTimerHandle, this, &AEclipseFireball::StopAndLinger, LifeSpan, false);
		
	}
	else
	{
		// 普通飞行，时间到了休眠
		GetWorldTimerManager().SetTimer(DeactivateTimerHandle, this, &AEclipseFireball::Deactivate, LifeSpan, false);
	}

	Multicast_WakeUpVisuals(FlatTransform, Speed, InCurveSpeed);
}

// 极其关键：因为改为 Unreliable，所以 12 发齐射绝对不再卡顿！
void AEclipseFireball::Multicast_WakeUpVisuals_Implementation(const FTransform& FlatTransform, float Speed, float InCurveSpeed)
{
	SetActorTransform(FlatTransform, false, nullptr, ETeleportType::TeleportPhysics);
	CurrentCurveSpeed = InCurveSpeed; 

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->bInitialVelocityInLocalSpace = false; 
		ProjectileMovementComp->MaxSpeed = Speed;
		ProjectileMovementComp->InitialSpeed = Speed;
		ProjectileMovementComp->Velocity = GetActorForwardVector() * Speed; 
		ProjectileMovementComp->Activate(true); 
		
	}

	if (NiagaraComp) NiagaraComp->Activate(true); 
}

// =========================================================
// 终点悬停与回收
// =========================================================
void AEclipseFireball::StopAndLinger()
{
	if (HasAuthority())
	{
		Multicast_StopAndLinger();
		
		// 悬停 1 秒后，真正消失！
		GetWorldTimerManager().SetTimer(DeactivateTimerHandle, this, &AEclipseFireball::Deactivate, 1.0f, false);
	}
}

void AEclipseFireball::Multicast_StopAndLinger_Implementation()
{
	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->Velocity = FVector::ZeroVector; 
		ProjectileMovementComp->Deactivate(); 
	}
	// 碰撞体和特效绝不关闭，让它在原地继续绞杀敌人！
}

void AEclipseFireball::Deactivate()
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(DeactivateTimerHandle);
		Multicast_Deactivate();
	}
}

void AEclipseFireball::Multicast_Deactivate_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (ProjectileMovementComp) ProjectileMovementComp->Deactivate();
	if (NiagaraComp) NiagaraComp->Deactivate();
}

// =========================================================
// 碰撞与附伤
// =========================================================
void AEclipseFireball::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || OtherActor == this || OtherActor->IsA(AEclipseFireball::StaticClass())) return;

	if (OtherActor && OtherActor != SkillInstigator && OtherActor->ActorHasTag(TEXT("Enemy")))
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC && DamageSpecHandle.IsValid())
		{
			// 造成基础伤害
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			
			if (AdditionalHitEffectClass)
			{
				UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SkillInstigator);
				if (InstigatorASC)
				{
					FGameplayEffectContextHandle ContextHandle = InstigatorASC->MakeEffectContext();
					ContextHandle.AddInstigator(SkillInstigator, this);
					FGameplayEffectSpecHandle HitSpecHandle = InstigatorASC->MakeOutgoingSpec(AdditionalHitEffectClass, 1.0f, ContextHandle);
					
					TargetASC->ApplyGameplayEffectSpecToSelf(*HitSpecHandle.Data.Get());
				}
			}
		}
	}
}