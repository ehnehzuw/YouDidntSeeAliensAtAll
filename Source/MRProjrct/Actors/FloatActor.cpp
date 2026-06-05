// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFloatActor::AFloatActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 30.f;
	MinNetUpdateFrequency = 15.f;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetIsReplicated(true);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
}

// Called when the game starts or when spawned
void AFloatActor::BeginPlay()
{
	Super::BeginPlay();

	if (Mesh)
	{
		HoverOriginZ = Mesh->GetComponentLocation().Z;
		RunningTime = 0.f;
	}

	bWasAttachedLastFrame = (GetAttachParentActor() != nullptr);
	if (HasAuthority() && bWasAttachedLastFrame)
	{
		ApplyHeldState(true);
	}
}

// Called every frame
void AFloatActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Mesh)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	const bool bIsCurrentlyAttached = (GetAttachParentActor() != nullptr);
	if (bIsCurrentlyAttached != bWasAttachedLastFrame)
	{
		ApplyHeldState(bIsCurrentlyAttached);
		bWasAttachedLastFrame = bIsCurrentlyAttached;
	}

	if (bIsHeld && bPauseFloatingWhileHeld)
	{
		return;
	}

	RunningTime += DeltaTime;

	const float DesiredZ = HoverOriginZ + TargetHeight + FMath::Sin(RunningTime * FloatSpeed) * FloatAmplitude* 0.1f;

	FVector NewLocation = GetActorLocation();
	NewLocation.Z = DesiredZ;
	SetActorLocation(NewLocation);
}

void AFloatActor::SetHeld(bool bInHeld)
{
	if (!HasAuthority())
	{
		ServerSetHeld(bInHeld);
		return;
	}

	ApplyHeldState(bInHeld);
}

void AFloatActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFloatActor, bIsHeld);
}

void AFloatActor::OnRep_IsHeld()
{
	ApplyHeldState(bIsHeld, true);
}

void AFloatActor::ServerSetHeld_Implementation(bool bInHeld)
{
	ApplyHeldState(bInHeld);
}

void AFloatActor::ApplyHeldState(bool bNewHeld, bool bForce)
{
	const bool bPreviousHeld = bIsHeld;
	if (!bForce && bPreviousHeld == bNewHeld)
	{
		return;
	}

	bIsHeld = bNewHeld;

	if (!Mesh)
	{
		return;
	}

	if (bIsHeld)
	{
		if (bDisablePhysicsWhileHeld)
		{
			if (!bForce || !bRestorePhysicsAfterRelease)
			{
				bRestorePhysicsAfterRelease = Mesh->IsSimulatingPhysics();
				bRestoreGravityAfterRelease = Mesh->IsGravityEnabled();
			}

			Mesh->SetEnableGravity(false);
			Mesh->SetSimulatePhysics(false);
		}
	}
	else
	{
		if (bPreviousHeld || bForce)
		{
			HoverOriginZ = GetActorLocation().Z - TargetHeight;
			RunningTime = 0.f;
		}

		if (bDisablePhysicsWhileHeld && bRestorePhysicsAfterRelease)
		{
			Mesh->SetSimulatePhysics(true);
			Mesh->SetEnableGravity(bRestoreGravityAfterRelease);
		}

		bRestorePhysicsAfterRelease = false;
		bRestoreGravityAfterRelease = false;
	}

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

