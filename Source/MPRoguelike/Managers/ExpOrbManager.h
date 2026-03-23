#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExpOrbManager.generated.h"

USTRUCT(BlueprintType)
struct FExpOrbData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ExpAmount;
	TWeakObjectPtr<AActor> TargetPlayer;

	FExpOrbData() : Location(FVector::ZeroVector), ExpAmount(0.0f) {}
	FExpOrbData(FVector Loc, float Exp) : Location(Loc), ExpAmount(Exp) {}
};

UCLASS()
class MPROGUELIKE_API AExpOrbManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AExpOrbManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "ExpOrb System")
	void AddExpOrb(FVector SpawnLocation, float ExpAmount);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddExpOrb(FVector SpawnLocation, float ExpAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "ExpOrb System")
	void OnOrbPickedUp(float ExpAmount, AActor* Picker);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpOrb Settings")
	float PickupRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpOrb Settings")
	float MagnetRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpOrb Settings")
	float MagnetSpeed = 1000.0f;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ExpOrb System")
	TArray<FExpOrbData> OrbList;
	
};