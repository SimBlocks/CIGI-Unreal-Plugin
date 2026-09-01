//Copyright SimBlocks LLC 2016-2026

/**
 * @file CigiView.h
 * @brief Defines the ACigiView pawn class for representing a CIGI view in Unreal Engine.
 *
 * This header provides:
 * - The ACigiView class, derived from APawn, which represents a camera view for CIGI integration.
 * - Blueprint-accessible properties for camera and view identification.
 * - Functions for updating the view's transform and handling per-frame updates.
 *
 * Usage:
 * - ACigiView is used to represent a CIGI view and its camera in the Unreal scene.
 * - The CameraComp property provides access to the camera component.
 * - The ViewID property identifies the view for Blueprint logic.
 * - Call UpdateTransform to update the pawn and controller transform.
 *
 * @copyright SimBlocks LLC 2016-2026
 */

#pragma once

#include "ModuleAPI.h"
#include "UnrealCigi_Plugin.h"
#include "CoreMinimal.h"
#include "CigiView.generated.h"

class UCameraComponent;

/**
 * @class ACigiView
 * @brief Pawn class representing a CIGI camera view.
 *
 * ACigiView manages a camera component and exposes a ViewID for Blueprint logic.
 * Use UpdateTransform to update the pawn's transform and controller.
 */
UCLASS()

class MODULE_API ACigiView : public APawn
{
  GENERATED_BODY()

public:
  /**
   * @brief Camera component for this view.
   */
  UPROPERTY(EditAnywhere)
  UCameraComponent* CameraComp;

  /**
   * @brief Sets default values for this actor's properties.
   */
  ACigiView();

  /**
   * @brief Updates the transform of this actor and its controller.
   * @param transform The new transform to apply.
   */
  void UpdateTransform(FTransform transform);

  /**
   * @brief Read-only copy of the view's ViewID (for Blueprint convenience).
   * ViewIDs do not change.
   */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi")
  int32 ViewID = 0;

protected:
  /**
   * @brief Called every frame to update the view.
   * @param deltaSeconds Time since last frame.
   */
  virtual void Tick(float deltaSeconds) override;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026