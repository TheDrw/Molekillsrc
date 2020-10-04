// Andrew Esberto 2020 drwmakesgames@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBreakableObject.generated.h"

class UDHealthComponent;
class UStaticMeshComponent;
class USoundBase;
class UDestructibleComponent;
class USphereComponent;

UCLASS()
class SPRINGGAMEJAM_API ADBreakableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBreakableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHealthChanged(UDHealthComponent* OwningHealthComp, float CurrentHealth, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent)
	void OnObjectDied();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HurtSphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
    UStaticMeshComponent* MeshComp;

	//UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	//UStaticMeshComponent* MeshComp;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	//UParticleSystem* BreakParticleEffect;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	//USoundBase* BreakSoundEffect;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	//USoundBase* GotHitSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Destructible")
	UDestructibleComponent* DestructibleComp;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_Broken, VisibleAnywhere, BlueprintReadonly, Category = "Destructible")
	bool bIsObjectBroken;

	UFUNCTION()
	void OnRep_Broken();
};
