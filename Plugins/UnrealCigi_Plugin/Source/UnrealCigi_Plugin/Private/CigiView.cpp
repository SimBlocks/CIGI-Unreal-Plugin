//Copyright SimBlocks LLC 2016-2026

#include "CigiView.h"
#include "Camera/CameraComponent.h"

ACigiView::ACigiView()
{
  PrimaryActorTick.bCanEverTick = true;

  // Create the default scene root
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  // Create the camera component for this pawn
  CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
  CameraComp->SetupAttachment(RootComponent);
  CameraComp->SetActive(true);

  AutoPossessPlayer = EAutoReceiveInput::Player0;
  // Make sure that the newly created camera component is used for the player's view
  bFindCameraComponentWhenViewTarget = true;
}

void ACigiView::Tick(float deltaSeconds)
{
  Super::Tick(deltaSeconds);

  // Update the controller's rotation to match the actor's rotation
  if (!IsValid(Controller))
  {
    return;
  }

  // Update the controller's rotation to match the actor's rotation
  Controller->SetControlRotation(GetActorRotation());
}

void ACigiView::UpdateTransform(FTransform transform)
{
  // Update the actor's transform
  RootComponent->SetRelativeTransform(transform);

  // Update the controller's rotation to match the new transform
  if (IsValid(Controller))
  {
    Controller->SetControlRotation(transform.Rotator());
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026