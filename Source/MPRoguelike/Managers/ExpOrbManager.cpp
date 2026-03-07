// Fill out your copyright notice in the Description page of Project Settings.

#include "ExpOrbManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AExpOrbManager::AExpOrbManager()
{
	PrimaryActorTick.bCanEverTick = true;  
	
	// ★ 必须加上这两行！
	bReplicates = true;
	bAlwaysRelevant = true;

	HISMComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISMComponent"));
	RootComponent = HISMComponent;
	HISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AExpOrbManager::BeginPlay()
{
	Super::BeginPlay();
}

void AExpOrbManager::AddExpOrb(FVector SpawnLocation, float ExpAmount)
{
	if (HasAuthority())
	{
		Multicast_AddExpOrb(SpawnLocation, ExpAmount);
	}
}

void AExpOrbManager::Multicast_AddExpOrb_Implementation(FVector SpawnLocation, float ExpAmount)
{
	// ★ 抬高 40 厘米，防止卡在地底看不见！
	SpawnLocation.Z += 40.0f; 

	OrbList.Add(FExpOrbData(SpawnLocation, ExpAmount));

	FVector OrbScale = FVector(0.5f, 0.5f, 0.5f); 
	FTransform InstanceTransform(FRotator::ZeroRotator, SpawnLocation, OrbScale);
	
	HISMComponent->AddInstance(InstanceTransform);
}

void AExpOrbManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Players);
	if (Players.Num() == 0) return;

	for (int32 i = OrbList.Num() - 1; i >= 0; i--)
	{
		FExpOrbData& Orb = OrbList[i];
		bool bIsPickedUp = false;

		// 1. 如果已经认主，死死追着那个玩家
		if (Orb.TargetPlayer.IsValid())
		{
			AActor* Player = Orb.TargetPlayer.Get();
			FVector PlayerLoc = Player->GetActorLocation();
			float DistSquared = FVector::DistSquared(Orb.Location, PlayerLoc);

			if (DistSquared <= (PickupRadius * PickupRadius))
			{
				bIsPickedUp = true;
				if (HasAuthority())
				{
					// TODO: GameState AddSharedExp
				}
			}
			else
			{
				FVector Direction = (PlayerLoc - Orb.Location).GetSafeNormal();
				Orb.Location += Direction * MagnetSpeed * DeltaTime;

				FVector OrbScale = FVector(0.5f, 0.5f, 0.5f); 
				FTransform NewTransform(FRotator::ZeroRotator, Orb.Location, OrbScale);
				HISMComponent->UpdateInstanceTransform(i, NewTransform, true, true, true);
			}
		}
		else
		{
			// 2. 如果没认主（静止在地上），看看谁靠近了
			for (AActor* Player : Players)
			{
				FVector PlayerLoc = Player->GetActorLocation();
				// 忽略高度差，纯测算平面距离
				float DistSquared = FVector::DistSquared(FVector(Orb.Location.X, Orb.Location.Y, 0), FVector(PlayerLoc.X, PlayerLoc.Y, 0));

				if (DistSquared <= (MagnetRadius * MagnetRadius))
				{
					Orb.TargetPlayer = Player; // ★ 核心：认主！
					break; 
				}
			}
		}

		if (bIsPickedUp)
		{
			HISMComponent->RemoveInstance(i);
			OrbList.RemoveAtSwap(i); 
		}
	}
}