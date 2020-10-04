// Andrew Esberto 2020 drwmakesgames@gmail.com


#include "DBreakableObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DHealthComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DestructibleComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADBreakableObject::ADBreakableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));

	HealthComp = CreateDefaultSubobject<UDHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ADBreakableObject::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	RootComponent = MeshComp;

	DestructibleComp = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DestructibleComp"));
	DestructibleComp->SetSimulatePhysics(false);
	DestructibleComp->SetGenerateOverlapEvents(false);
	DestructibleComp->SetCollisionObjectType(ECollisionChannel::ECC_Destructible);
	DestructibleComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	DestructibleComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	DestructibleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	DestructibleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	DestructibleComp->SetupAttachment(RootComponent);

	HurtSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("HurtSphereComp"));
	HurtSphereComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	HurtSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HurtSphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HurtSphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	HurtSphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	HurtSphereComp->SetSphereRadius(100.f);
	HurtSphereComp->SetupAttachment(RootComponent);
	
	bIsObjectBroken = false;
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ADBreakableObject::BeginPlay()
{
	Super::BeginPlay();
	DestructibleComp->Deactivate();
	DestructibleComp->SetVisibility(false);
}

void ADBreakableObject::OnHealthChanged(UDHealthComponent* OwningHealthComp, float CurrentHealth, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsObjectBroken) return;
	
	if (OwningHealthComp->IsDead())
	{
		/*FVector Dir = GetActorLocation() - DamageCauser->GetActorLocation();
		DestructibleComp->ApplyDamage(MAX_FLT, GetActorLocation(), Dir, 10000.f);*/
		bIsObjectBroken = true;
		OnRep_Broken();
		//OnObjectDied();
	}
}

void ADBreakableObject::OnRep_Broken()
{
	OnObjectDied();
}


void ADBreakableObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADBreakableObject, bIsObjectBroken);
}