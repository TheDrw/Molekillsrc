// Andrew Esberto 2020 drwmakesgames@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "DBreakableObject.h"
#include "DMolecule.generated.h"

/**
 * 
 */
UCLASS()
class SPRINGGAMEJAM_API ADMolecule : public ADBreakableObject
{
	GENERATED_BODY()
public:

	ADMolecule();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
protected:

	float Variance;
	float StartHeight;
};
