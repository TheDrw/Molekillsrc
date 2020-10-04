// Andrew Esberto 2020 drwmakesgames@gmail.com



#include "DMolecule.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

ADMolecule::ADMolecule()
{
	PrimaryActorTick.bCanEverTick = true;
	Variance = 1.f;
}

void ADMolecule::BeginPlay()
{
	Super::BeginPlay();

	Variance = UKismetMathLibrary::RandomFloatInRange(-1.f, 1.f);
	StartHeight = GetActorLocation().Z;
}

void ADMolecule::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Period = UGameplayStatics::GetTimeSeconds(GetWorld());
	float NewAddedHeight = sin(Period * 5.f + Variance) * 10.f; //(DeltaTime + sin(Period + (Variance + 5.f))) * 1.05f;
	SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, NewAddedHeight + StartHeight));
}
