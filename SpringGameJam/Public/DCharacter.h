// Andrew Esberto 2020 drwmakesgames@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USceneComponent;
class UDHealthComponent;
class USphereComponent;
class UParticleSystem;
class USoundBase;
class UAudioComponent;

// OnDashStart Event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDashStart, float, DashTime, float, CurrentDashTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDashActive, float, CurrentDashTime, float, DashDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitEnemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitSelf);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRageModeStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRageModeActive, float, CurrentRageTime, float, RageDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRageModeEnd);

UENUM(BlueprintType)
enum class EDashState : uint8
{
	Ready, Active, NotReady
};

USTRUCT()
struct FDashAttack
{
	GENERATED_BODY()
public:

};

UCLASS()
class SPRINGGAMEJAM_API ADCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsRunning;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsTest;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsDashing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsFalling;

protected:
	void MoveForward(float Value);
	void MoveRight(float Value);

	void StartDash();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartDash();

	void EndDash();

	void EndDashCooldownTimer();

	void DashLaunch();

	void DisableDashHitOverlap();

	void StartRageMode();
	void EndRageMode();

	void SetDashState(EDashState NewState);

	void DoJump();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnDashAttackStart(bool bRageMode);

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnDashAttackEnd();

	UFUNCTION()
	void OnHealthChanged(UDHealthComponent* OwningHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void DashHitSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	USceneComponent* AzimuthGimbalSceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UDHealthComponent* HealthComp;

	/* Dash hit collider (aka hitsphere)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USphereComponent* DashHitSphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float DashDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float DashForce;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RageDashForce;

	/* The amount of tiem you will bev active in dashing */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float DashTimeLength;

	/* The amount of time you will be active in rage dashing  */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RageDashTimeLength;

	/* Dashes that can be performed per second */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RateOfDashes;

	/* Rage Dashes that can be performed per second */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RateOfRageDashes;

	/* The length of time of when the dash hit is active in seconds. Generally should be or close ot DashtimeLength but not over*/
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float DashActiveHitColliderTimeLength;

	/* The length of time of when the rage dash hit is active in seconds. Generally should be or close to RageDashTimeLength but not over */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RageDashActiveHitColliderTimeLength;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RunSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	bool bIsInRageMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TSubclassOf<UDamageType> DashDamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	int NumberOfComboForRageMode;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float RageModeDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* GotHitSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* DiedSoundEffect;

	///
	/// Default Dash Setup
	///

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* DashStartupSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* DashWhooshSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* DashHitSoundEffect;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* DashParticleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* DashHitParticleEffect;

	///
	/// Rage Dash settup
	//

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		USoundBase* RageDashModeActivatedSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		USoundBase* RageDashModeDeactivatedSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		USoundBase* RageDashModeAmbienceSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		USoundBase* RageDashStartupSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		USoundBase* RageDashWhooshSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		USoundBase* RageDashHitSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		UParticleSystem* RageDashTraileEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		UParticleSystem* RageDashParticleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		UParticleSystem* RageDashStartupParticleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		UParticleSystem* RageDashHitParticleEffect;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentDashTime;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDashStart OnDashStart;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDashActive OnDashActive;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDied OnHitEnemy;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDied OnHitSelf;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRageModeStart OnRageModeStart;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRageModeEnd OnRageModeEnd;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRageModeActive OnRageModeActive;

	

	//UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	//void OnDashBegin(float Duration);

	EDashState DashState;

	FTimerHandle TimerHandle_DashLengthTimer;

	FTimerHandle TimerHandle_DashCooldownTimer;

	FTimerHandle TimerHandle_DashActiveTime;

	FTimerHandle TimerHandle_RageModeTimer;

	FTimerHandle TimerHandle_DashLaunchDelay;

	UAudioComponent* AudioComponent;

	float CurrentRageModeTime;
	int CurrentComboForRageMode;

	float CurrentRageDashRate;
	float CurrentDashRate;

	float TimeBetweenDashes;
	float LastTimeDashed;

	bool bUsingInputForward, bUsingInputRight;
	//FRotator CurrentRotationRate;

	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedRotation)
	FRotator ReplicatedRotation;

	UFUNCTION()
	void OnRep_ReplicatedRotation();


	float CurrentGroundFriction;
};