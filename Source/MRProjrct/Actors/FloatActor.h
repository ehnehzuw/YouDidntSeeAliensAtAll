// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "FloatActor.generated.h"

UCLASS()
class MRPROJRCT_API AFloatActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloatActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Float")
	void SetHeld(bool bInHeld);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Float")
	float TargetHeight = 0.f;

	UPROPERTY(EditAnywhere, Category = "Float")
	float FloatAmplitude = 50.f;

	UPROPERTY(EditAnywhere, Category = "Float")
	float FloatSpeed = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Float")
	bool bPauseFloatingWhileHeld = true;

	UPROPERTY(EditAnywhere, Category = "Float")
	bool bDisablePhysicsWhileHeld = true;

private:
	UFUNCTION()
	void OnRep_IsHeld();

	UFUNCTION(Server, Reliable)
	void ServerSetHeld(bool bInHeld);

	float HoverOriginZ = 0.f;
	float RunningTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsHeld)
	bool bIsHeld = false;

	bool bWasAttachedLastFrame = false;
	bool bRestorePhysicsAfterRelease = false;
	bool bRestoreGravityAfterRelease = false;

	void ApplyHeldState(bool bNewHeld, bool bForce = false);

};
