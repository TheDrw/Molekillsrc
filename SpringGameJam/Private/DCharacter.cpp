// Andrew Esberto 2020 drwmakesgames@gmail.com


#include "DCharacter.h"
#include "DHealthComponent.h"
#include "DMolecule.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADCharacter::ADCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input

	AzimuthGimbalSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("AzimuthGimbalSceneComp"));
	AzimuthGimbalSceneComp->SetupAttachment(RootComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;  // rotate camera based on viewer vision
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritYaw = true;
	SpringArmComp->bInheritRoll = true;
	SpringArmComp->SetupAttachment(AzimuthGimbalSceneComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	DashHitSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("DashHitSphereComp"));
	DashHitSphereComp->SetRelativeLocation(FVector::ForwardVector * 44.f);
	DashHitSphereComp->SetSphereRadius(72.f);
	DashHitSphereComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	DashHitSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DashHitSphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DashHitSphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	DashHitSphereComp->SetGenerateOverlapEvents(true);
	DashHitSphereComp->SetIsReplicated(true);
	DashHitSphereComp->bMultiBodyOverlap = true;
	DashHitSphereComp->SetupAttachment(RootComponent);
	
	HealthComp = CreateDefaultSubobject<UDHealthComponent>(TEXT("HealthComp"));

	DashDamage = 9001.f; // IT IS OVER NINE...all right, all right, i know you've heard it a thousand times... OVER 9000 TIMES!!!
	DashForce = 10000.f;
	RageDashForce = 15000.f;
	DashTimeLength = 0.1f;
	RageDashTimeLength = 0.2f;
	RateOfDashes = 1.f;
	RateOfRageDashes = 0.5f;
	DashActiveHitColliderTimeLength = 0.1f;
	RageDashActiveHitColliderTimeLength = 0.2f;
	RunSpeed = 800.f;

	bIsInRageMode = false;

	bIsDead = false;
	bIsRunning = false;
	bIsDashing = false;
	bIsFalling = false;

	RageModeDuration = 10.f;
	NumberOfComboForRageMode = 5;

	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	AudioComponent = nullptr;
	DashState = EDashState::Ready;
	bUsingInputForward = false;
	bUsingInputRight = false;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	HealthComp->OnHealthChanged.AddDynamic(this, &ADCharacter::OnHealthChanged);
	DashHitSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ADCharacter::DashHitSphereBeginOverlap);
}

// Called when the game starts or when spawned
void ADCharacter::BeginPlay()
{
	
	CurrentDashTime = RateOfDashes;
	
	Super::BeginPlay();
	
	CurrentDashRate = RateOfDashes;
	CurrentRageDashRate = RateOfRageDashes;

	CurrentComboForRageMode = 0;

	DashHitSphereComp->SetGenerateOverlapEvents(false);
}

// Called every frame
void ADCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorldTimerManager().IsTimerActive(TimerHandle_DashCooldownTimer))
	{
		float DashRateDuration = CurrentDashRate;
		if (bIsInRageMode)
		{
			DashRateDuration = CurrentRageDashRate;
		}

		CurrentDashTime += DeltaTime;
		OnDashActive.Broadcast(CurrentDashTime, DashRateDuration);
	}

	if (bIsInRageMode)
	{
		CurrentRageModeTime -= DeltaTime;
		CurrentDashTime = CurrentRageModeTime < 0.f ? 0.f : CurrentRageModeTime;
		OnRageModeActive.Broadcast(CurrentRageModeTime, RageModeDuration);
	}

	const float PADDING = 10.f;
	const float CURRENT_SPEED = GetCharacterMovement()->Velocity.Size();
	if (CURRENT_SPEED <= GetCharacterMovement()->GetMaxSpeed() + PADDING
		&& DashState == EDashState::Active)
	{
		SetDashState(EDashState::NotReady);
	}

	// if jus landed this frame
	if (bIsFalling && !GetCharacterMovement()->IsFalling())
	{
		if (DashState == EDashState::NotReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("LANDED"));
			SetDashState(EDashState::Ready);
		}
	}

	bIsFalling = GetCharacterMovement()->IsFalling() ? true : false;

	if (CURRENT_SPEED > 0.f && !bIsFalling)
	{
		bIsRunning = true;
	}
	else if(!bIsFalling)
	{
		bIsRunning = false;
	}

	if (HasAuthority())
	{
		ReplicatedRotation = GetActorRotation();
	}
}

// Called to bind functionality to input
void ADCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADCharacter::MoveRight);
	PlayerInputComponent->BindAxis("RotateCamera", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ADCharacter::DoJump);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ADCharacter::StartDash);
}

void ADCharacter::MoveForward(float Value)
{
	if (Controller != nullptr)
	{
		if (Value != 0.f)
		{
			bUsingInputForward = true;
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			AddMovementInput(Direction, Value);
		}
		else
		{
			bUsingInputForward = false;
		}
	}
}

void ADCharacter::MoveRight(float Value)
{
	if (Controller != nullptr)
	{
		if (Value != 0.f)
		{
			bUsingInputRight = true;
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(Direction, Value);
		}
		else
		{
			bUsingInputRight = false;
		}
	}
}

void ADCharacter::StartDash()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASJKD"));
		ServerStartDash();
		return;
	}

	if (DashState != EDashState::Ready) return;

	if (GetWorldTimerManager().IsTimerActive(TimerHandle_DashCooldownTimer))
	{
		UE_LOG(LogTemp, Warning, TEXT("DASH ON COOLDOWN ATIVE"));
		return;
	}


	// only change rotation to camera fwd when not inputing any directional input
	bool bIdle = !(bUsingInputForward || bUsingInputRight);
	// dash forward if idle
	if (bIdle)
	{
		FRotator CameraForwardRotation = CameraComp->GetForwardVector().Rotation();
		CameraForwardRotation.Roll = 0.f;
		CameraForwardRotation.Pitch = 0.f;

		GetCapsuleComponent()->SetRelativeRotation(CameraForwardRotation);
	}

	DashHitSphereComp->SetGenerateOverlapEvents(true);
	GetWorldTimerManager().SetTimer(TimerHandle_DashLaunchDelay, this, &ADCharacter::DashLaunch, 0.025f, false);
}

void ADCharacter::ServerStartDash_Implementation()
{
	StartDash();
}

bool ADCharacter::ServerStartDash_Validate()
{
	return true;
}

void ADCharacter::DashLaunch()
{
	SetDashState(EDashState::Active);
	CurrentDashTime = 0.f;
	CurrentDashRate = RateOfDashes;
	CurrentRageDashRate = RateOfRageDashes;

	CurrentGroundFriction = GetCharacterMovement()->GroundFriction;
	// make it so dashing on ground is like dashing in air. resets friction when dashing is end
	GetCharacterMovement()->GroundFriction = 0.f; 

	if (bIsInRageMode)
	{
		UGameplayStatics::PlaySound2D(this, RageDashStartupSoundEffect);
		UGameplayStatics::PlaySound2D(this, RageDashWhooshSoundEffect);
		LaunchCharacter(GetActorForwardVector() * RageDashForce, true, true);

		GetWorldTimerManager().SetTimer(TimerHandle_DashCooldownTimer, this, &ADCharacter::EndDashCooldownTimer, CurrentRageDashRate, false);
		GetWorldTimerManager().SetTimer(TimerHandle_DashLengthTimer, this, &ADCharacter::EndDash, RageDashTimeLength, false);
		GetWorldTimerManager().SetTimer(TimerHandle_DashActiveTime, this, &ADCharacter::DisableDashHitOverlap, RageDashActiveHitColliderTimeLength, false);
	}
	else
	{
		UGameplayStatics::PlaySound2D(this, DashStartupSoundEffect);
		UGameplayStatics::PlaySound2D(this, DashWhooshSoundEffect);
		LaunchCharacter(GetActorForwardVector() * DashForce, true, true);

		GetWorldTimerManager().SetTimer(TimerHandle_DashCooldownTimer, this, &ADCharacter::EndDashCooldownTimer, CurrentDashRate, false);
		GetWorldTimerManager().SetTimer(TimerHandle_DashLengthTimer, this, &ADCharacter::EndDash, DashTimeLength, false);
		GetWorldTimerManager().SetTimer(TimerHandle_DashActiveTime, this, &ADCharacter::DisableDashHitOverlap, DashActiveHitColliderTimeLength, false);
	}
}

void ADCharacter::EndDash()
{
	if (DashState != EDashState::Active) return;

	GetCharacterMovement()->GroundFriction = CurrentGroundFriction;
	GetCharacterMovement()->StopMovementImmediately();
	SetDashState(EDashState::NotReady);
	

	if (!GetCharacterMovement()->IsFalling()) return;
	
	GetWorldTimerManager().ClearTimer(TimerHandle_DashLengthTimer);
}

void ADCharacter::EndDashCooldownTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("COOLDOWN FIN"));
	CurrentDashTime = RateOfDashes;
	GetWorldTimerManager().ClearTimer(TimerHandle_DashCooldownTimer);
}

void ADCharacter::SetDashState(EDashState NewState)
{
	if (DashState == NewState) return;

	auto PreviousDashState = DashState;
	DashState = NewState;
	if (DashState == EDashState::Active)
	{
		bIsDashing = true;
		HealthComp->SetInvulnerability(true);
		GetCharacterMovement()->GroundFriction = 4.f;
		GetCharacterMovement()->AirControl = 0.5f;
		OnDashAttackStart(bIsInRageMode);
	}
	else
	{
		bIsDashing = false;
		HealthComp->SetInvulnerability(false);
		GetCharacterMovement()->GroundFriction = 8.f;
		GetCharacterMovement()->AirControl = 0.05f;
	}

	if (DashState == EDashState::NotReady)
	{
		DisableDashHitOverlap();

		if (!bIsFalling)
		{
			DashState = EDashState::Ready;
		}
	}

	if (PreviousDashState == EDashState::Active && NewState == EDashState::NotReady)
	{
		OnDashAttackEnd();
	}
}

void ADCharacter::DisableDashHitOverlap()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_DashActiveTime))
	{
		UE_LOG(LogTemp, Warning, TEXT("DISABLE ACTIVE OVERLAP"));
		DashHitSphereComp->SetGenerateOverlapEvents(false);
		GetWorldTimerManager().ClearTimer(TimerHandle_DashActiveTime);
	}
}

void ADCharacter::StartRageMode()
{
	UE_LOG(LogTemp, Warning, TEXT("RAGE MODE ON"));
	CurrentRageModeTime = RageModeDuration;
	bIsInRageMode = true;
	OnRageModeStart.Broadcast();
	UGameplayStatics::PlaySound2D(this, RageDashModeActivatedSoundEffect);
	AudioComponent = UGameplayStatics::SpawnSound2D(this, RageDashModeAmbienceSoundEffect);

	EndDashCooldownTimer(); // you can get a dash reset when entering rage mode
	GetWorldTimerManager().SetTimer(TimerHandle_RageModeTimer, this, &ADCharacter::EndRageMode, RageModeDuration, false);
}

void ADCharacter::EndRageMode()
{
	UE_LOG(LogTemp, Warning, TEXT("RAGE MODE OFF"));

	bIsInRageMode = false;
	UGameplayStatics::PlaySound2D(this, RageDashModeDeactivatedSoundEffect);
	CurrentComboForRageMode = 0;

	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
	
	CurrentRageModeTime = RageModeDuration;
	OnRageModeEnd.Broadcast();
}

void ADCharacter::DoJump()
{
	if (DashState != EDashState::Active)
	{
		ACharacter::Jump();
	}
}

void ADCharacter::OnHealthChanged(UDHealthComponent* OwningHealthComp, float CurrentHealth, float HealthDelta, 
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (HealthComp->IsDead())
	{
		bIsDead = HealthComp->IsDead();
		UGameplayStatics::PlaySound2D(this, DiedSoundEffect);
		GetMovementComponent()->StopMovementImmediately();
		DisableInput(GetWorld()->GetFirstPlayerController());
		OnDied.Broadcast();
	}
	else
	{
		OnHitSelf.Broadcast();
		UGameplayStatics::PlaySound2D(this, GotHitSoundEffect);
	}
}

void ADCharacter::DashHitSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	
	UE_LOG(LogTemp, Warning, TEXT("HIT SPHER"));

	auto MyOwner = GetOwner();
	auto TargetHealthComp = Cast<UDHealthComponent>(OtherActor->GetComponentByClass(UDHealthComponent::StaticClass()));
	if (TargetHealthComp && MyOwner)
	{
		if (bIsInRageMode)
		{
			UGameplayStatics::PlaySound2D(this, RageDashHitSoundEffect);
		}
		else
		{
			UGameplayStatics::PlaySound2D(this, DashHitSoundEffect);
		}

		UGameplayStatics::ApplyDamage(OtherActor, DashDamage, MyOwner->GetInstigatorController(), MyOwner, DashDamageType);

		auto Molecule = Cast<ADMolecule>(OtherActor);
		if (Molecule)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *OtherActor->GetFName().ToString());
			CurrentComboForRageMode++;
			if (CurrentComboForRageMode == NumberOfComboForRageMode)
			{
				StartRageMode();
			}
		}
	}
}

void ADCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADCharacter, ReplicatedRotation);

}

void ADCharacter::OnRep_ReplicatedRotation()
{
	SetActorRotation(ReplicatedRotation);
}