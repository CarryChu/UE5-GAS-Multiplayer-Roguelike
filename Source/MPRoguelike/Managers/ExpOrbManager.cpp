#include "ExpOrbManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AExpOrbManager::AExpOrbManager()
{
 	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
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
	SpawnLocation.Z -= 40.0f;
	OrbList.Add(FExpOrbData(SpawnLocation, ExpAmount));
}

void AExpOrbManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() && GetNetMode() != NM_Standalone) return;

	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Players);
	if (Players.Num() == 0) return;

	for (int32 i = OrbList.Num() - 1; i >= 0; i--)
	{
		FExpOrbData& Orb = OrbList[i];
		bool bIsPickedUp = false;

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
					OnOrbPickedUp(Orb.ExpAmount, Player);
				}
			}
			else
			{
				FVector Direction = (PlayerLoc - Orb.Location).GetSafeNormal();
				Orb.Location += Direction * MagnetSpeed * DeltaTime;
			}
		}
		else
		{
			for (AActor* Player : Players)
			{
				
				// ==========================================
				// ★ 核心修复：敌我识别！
				// ==========================================
				APawn* Pawn = Cast<APawn>(Player);
				// 如果转换成功，并且它不是由真实玩家控制的（比如是小怪 AI），就直接跳过！
				if (Pawn && !Pawn->IsPlayerControlled())
				{
					continue; 
				}
				
				FVector PlayerLoc = Player->GetActorLocation();
				float DistSquared = FVector::DistSquared(FVector(Orb.Location.X, Orb.Location.Y, 0), FVector(PlayerLoc.X, PlayerLoc.Y, 0));

				if (DistSquared <= (MagnetRadius * MagnetRadius))
				{
					Orb.TargetPlayer = Player;
					break;
				}
			}
		}

		if (bIsPickedUp)
		{
			OrbList.RemoveAtSwap(i);
		}
	}
}

void AExpOrbManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExpOrbManager, OrbList);
}