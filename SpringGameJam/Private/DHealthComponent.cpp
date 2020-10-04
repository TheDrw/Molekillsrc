// Andrew Esberto 2020 drwmakesgames@gmail.com


#include "DHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UDHealthComponent::UDHealthComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	MaxHealth = 100.f;
	bIsDead = false;
}

// Called when the game starts
void UDHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	bIsInvulnerable = false;

	// only hook if server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner && MyOwner->HasAuthority())
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UDHealthComponent::HandleTakeAnyDamage);
		}
	}
	CurrentHealth = MaxHealth;
	bAtMaxHealth = true;
}

void UDHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsInvulnerable)
	{
		UE_LOG(LogTemp, Warning, TEXT("INVULNERABLE!!!"));
		return;
	}

	if (Damage <= 0.f || !IsAlive())
	{
		return;
	}

	// if self
	if (GetOwner() != DamagedActor)
	{
		return;
	}

	

	bAtMaxHealth = false;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	bIsDead = CurrentHealth <= 0.f;
	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
}

float UDHealthComponent::GetHealth() const
{
	return CurrentHealth;
}

bool UDHealthComponent::IsAlive() const
{
	return CurrentHealth > 0.f;
}

bool UDHealthComponent::IsDead() const
{
	return bIsDead;
}

void UDHealthComponent::Heal(const float HealAmount)
{
	if (bIsDead) return;

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	bAtMaxHealth = CurrentHealth == MaxHealth;
	UE_LOG(LogTemp, Log, TEXT("Healthj changed : %s (+%s)"), *FString::SanitizeFloat(CurrentHealth), *FString::SanitizeFloat(HealAmount));
	OnHealthChanged.Broadcast(this, CurrentHealth, -HealAmount, nullptr, nullptr, nullptr);
}

void UDHealthComponent::SetInvulnerability(bool bInvulnerable)
{
	bIsInvulnerable = bInvulnerable;
}

void UDHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDHealthComponent, CurrentHealth);
	DOREPLIFETIME(UDHealthComponent, bIsInvulnerable);
	DOREPLIFETIME(UDHealthComponent, bIsDead);
}