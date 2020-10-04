// Andrew Esberto 2020 drwmakesgames@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DHealthComponent.generated.h"

// OnHealthChanged Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UDHealthComponent*, HealthComp, float, CurrentHealth, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPRINGGAMEJAM_API UDHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadonly, Category = "HealthComponent")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float MaxHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(Replicated)
	bool bIsInvulnerable;

	UPROPERTY(BlueprintReadOnly)
	bool bAtMaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bIsDead;

public:	
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	bool IsDead() const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(const float HealAmount);

	void SetInvulnerability(bool bInvulnerable);
};
