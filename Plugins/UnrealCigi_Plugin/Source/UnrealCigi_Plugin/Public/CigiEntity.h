//Copyright SimBlocks LLC 2016-2026
/**
 * @file CigiEntity.h
 * @brief Defines the ACigiEntity actor and related types for representing and controlling CIGI entities in Unreal Engine.
 *
 * This header provides:
 * - The ACigiEntity actor class, which manages entity visuals, articulated parts/bones, collision segments/volumes, and widget components.
 * - Data structures for SISO entity identification, articulated bone data, component data, and collision segments.
 * - Blueprint-accessible properties and events for entity configuration and control.
 *
 * Usage:
 * - Derive a Blueprint from ACigiEntity to create custom entity types.
 * - Use provided functions to set up visuals, articulated parts/bones, and collision handling.
 * - Respond to CIGI component control packets via Blueprint events.
 */

#pragma once

// (windows definition of min/max macros conflicts with Slate/SSpinBox)
#define NOMINMAX

#include "ModuleAPI.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "unrealcigiUtil.h"
#include "UnrealCoordinates.h"

#include "CigiEntity.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCigiEntity, Log, All)

/**
 * @struct FSisoID
 * @brief SISO entity enumeration structure.
 * @details Contains entity enumeration kind, domain, country, category, subcategory, specific, and extra fields.
 * See SISO-REF-010 for details on these fields and their usage in entity identification.
 */
USTRUCT(BlueprintType)

struct FSisoID
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Kind;///< entity enumeration kind
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Domain;///< entity enumeration domain
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Country;///< entity enumeration country
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Category;///< entity enumeration category
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Subcategory;///< entity enumeration subcategory
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Specific;///< entity enumeration specific
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|SisoEntityEnumeration")
  int Extra;///< entity enumeration extra

  /**
   * @brief Initializes the SISO entity enumeration fields.
   * @param kind entity enumeration kind
   * @param domain entity enumeration domain
   * @param country entity enumeration country
   * @param category entity enumeration category
   * @param subcategory entity enumeration subcategory
   * @param specific entity enumeration specific
   * @param extra entity enumeration extra
   */
  void Init(int kind, int domain, int country, int category, int subcategory, int specific, int extra);

  /**
   * @brief Constructs a SISO ID with all fields.
   */
   FSisoID(int kind, int domain, int country, int category, int subcategory, int specific, int extra);

  /**
   * @brief Constructs a SISO ID from an SEntityType.
   */
   FSisoID(sbio::entity::SEntityType original);

  /**
   * @brief Default constructor.
   */
   FSisoID();

  /**
   * @brief Returns a string representation of the SISO entity enumeration.
   * @return String describing the SISO entity enumeration.
   */
  FString ToString() const;
};

/**
 * @struct FArticulatedBoneData
 * @brief Data for an articulated bone in an entity.
 * @details Contains bone name and origin transform.
 */
USTRUCT(BlueprintType)

struct FArticulatedBoneData
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ArticulatedBoneData")
  FName boneName;///< Bone name
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ArticulatedBoneData")
  FTransform originTransform;///< Bone origin transform

  /**
   * @brief Default constructor.
   */
  FArticulatedBoneData();

  /**
   * @brief Returns true if the bone has a valid name.
   * @return True if boneName is valid.
   */
  bool HasName() const;
};

/**
 * @struct FComponentData
 * @brief Data for a CIGI component.
 * @details Contains six 64-bit data fields.
 */
USTRUCT(BlueprintType)

struct FComponentData
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ComponentData")
  int64 Data1;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ComponentData")
  int64 Data2;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ComponentData")
  int64 Data3;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ComponentData")
  int64 Data4;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ComponentData")
  int64 Data5;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|ComponentData")
  int64 Data6;

  /**
   * @brief Default constructor.
   */
  FComponentData();

  /**
   * @brief Constructs a FComponentData with all fields.
   */
  FComponentData(int64 data1, int64 data2, int64 data3, int64 data4, int64 data5, int64 data6);

  /**
   * @brief Constructs a FComponentData from an SComponentData.
   */
  FComponentData(sbio::ig::SComponentData original);

  /**
   * @brief Returns a string representation of the component data.
   * @return String describing the component data.
   */
  FString ToString() const;
};

/**
 * @struct FCollisionSegment
 * @brief Represents a collision segment for CIGI entity collision handling.
 * @details Contains start/end points and enabled state.
 */
USTRUCT(BlueprintType)

struct FCollisionSegment
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|CollisionSegment")
  FVector start;///< Segment start point
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|CollisionSegment")
  FVector end;///< Segment end point
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cigi|CollisionSegment")
  bool enable;///< Is segment enabled

  /**
   * @brief Default constructor.
   */
  FCollisionSegment();

  /**
   * @brief Constructs a FCollisionSegment with all fields.
   * @param _start Start point.
   * @param _end End point.
   * @param _enable Enabled state.
   */
  FCollisionSegment(FVector _start, FVector _end, bool _enable);

  /**
   * @brief Returns a string representation of the collision segment.
   * @return String describing the segment.
   */
  FString ToString() const;
};

/**
 * @class ACigiEntity
 * @brief Actor class representing a CIGI entity in Unreal Engine.
 * @details Manages entity visuals, articulated parts/bones, collision segments/volumes, and widget components.
 *          Exposes Blueprint-accessible properties and events for entity configuration and control.
 */
UCLASS()

class MODULE_API ACigiEntity : public AActor
{
  GENERATED_BODY()

public:
  /**
   * @brief Default constructor.
   */
  ACigiEntity();

  /**
   * @brief Sets the visual asset and transform for this entity.
   * @param path Path to the asset.
   * @param transform Transform to apply.
   * @param articulateBones Whether to articulate bones.
   */
  void SetVisual(const TCHAR* path, FTransform transform, bool articulateBones);

  /**
   * @brief Adds an articulated part to the entity.
   * @param id Articulated part ID.
   * @param origin Origin position.
   * @param path Path to the asset.
   * @param transform Transform to apply.
   */
  void AddArticulatedPart(sbio::ArticulatedPartID id, FVector origin, const TCHAR* path, FTransform transform);

  /**
   * @brief Adds articulated bones to the entity.
   * @param articulatedBoneNames Map of bone names to IDs.
   */
  void AddArticulatedBones(TMap<FString, int32> articulatedBoneNames);

  /**
   * @brief Finds and registers articulated parts in the entity.
   */
  void FindArticulatedParts();

  /**
   * @brief Enables or disables the entity.
   * @param enabled True to enable, false to disable.
   */
  void SetEnabled(bool enabled);

  /**
   * @brief Sets the actor location from strong Unreal engine coordinates.
   * @param location Strongly typed Unreal world-space location.
   */
  void SetEngineLocation(const sbio::unrealcigi::FUEWorldCoordinates& location);

  /**
   * @brief Gets the actor location as strong Unreal world-space coordinates.
   * @return Strongly typed Unreal world-space location.
   */
  sbio::unrealcigi::FUEWorldCoordinates GetEngineLocation() const;

  /**
   * @brief Sets the actor rotation from strong Unreal world-space rotation.
   * @param rotation Strongly typed Unreal world-space rotation.
   */
  void SetEngineRotation(const sbio::unrealcigi::FUEWorldRotation& rotation);

  /**
   * @brief Gets the actor rotation as strong Unreal world-space rotation.
   * @return Strongly typed Unreal world-space rotation.
   */
  sbio::unrealcigi::FUEWorldRotation GetEngineRotation() const;

  /**
   * @brief Enables or disables an articulated part.
   * @param apID Articulated part ID.
   * @param enabled True to enable, false to disable.
   */
  void SetArticulatedPartEnabled(sbio::ArticulatedPartID apID, bool enabled);

  /**
   * @brief Updates the transform of an articulated part.
   * @param apID Articulated part ID.
   * @param transform New transform.
   */
  void UpdateArticulatedPartTransform(sbio::ArticulatedPartID apID, FTransform transform);

  /**
   * @brief Creates a collision segment.
   * @param segID Segment ID.
   * @return True if created successfully.
   */
  bool CreateCollisionSegment(sbio::SegmentID segID);

  /**
   * @brief Updates a collision segment's start and end points.
   * @param segID Segment ID.
   * @param start Start point.
   * @param end End point.
   */
  bool UpdateCollisionSegment(sbio::SegmentID segID, FVector start, FVector end);

  /**
   * @brief Enables or disables a collision segment.
   * @param segID Segment ID.
   * @param enabled True to enable, false to disable.
   */
  bool UpdateCollisionSegment(sbio::SegmentID segID, bool enabled);

  /**
   * @brief Creates a collision volume.
   * @param volID Volume ID.
   * @param sphere True for sphere, false for box.
   * @return True if created successfully.
   */
  bool CreateCollisionVolume(sbio::VolumeID volID, bool sphere);

  /**
   * @brief Enables or disables a collision volume.
   * @param volID Volume ID.
   * @param enabled True to enable, false to disable.
   * @return True if successful.
   */
  bool SetCollisionVolumeEnabled(sbio::VolumeID volID, bool enabled);

  /**
   * @brief Sets the offset for a collision volume.
   * @param volID Volume ID.
   * @param offset Offset vector.
   * @return True if successful.
   */
  bool SetCollisionVolumeOffset(sbio::VolumeID volID, FVector offset);

  /**
   * @brief Sets the rotation for a collision volume.
   * @param volID Volume ID.
   * @param rot Rotation quaternion.
   * @return True if successful.
   */
  bool SetCollisionVolumeRotation(sbio::VolumeID volID, FQuat rot);

  /**
   * @brief Sets the size for a collision volume.
   * @param volID Volume ID.
   * @param size Size vector.
   * @return True if successful.
   */
  bool SetCollisionVolumeSize(sbio::VolumeID volID, FVector size);

  /**
   * @brief Destroys a collision volume.
   * @param volID Volume ID.
   * @return True if destroyed successfully.
   */
  bool DestroyCollisionVolume(sbio::VolumeID volID);

  /**
   * @brief Finds the collision volume for a given component.
   * @param volume Component to search for.
   * @return Volume ID if found, -1 otherwise.
   */
  int32 FindCollisionVolume(UPrimitiveComponent* volume);
  
  /**
   * @brief Updates the widget component for a symbol surface.
   * @param surfaceID Symbol surface ID.
   */
  void UpdateWidgetComponent(sbio::symbol::SymbolSurfaceID surfaceID);

  /**
   * @brief Removes the widget component for a symbol surface.
   * @param surfaceID Symbol surface ID.
   */
  void RemoveWidgetComponent(sbio::symbol::SymbolSurfaceID surfaceID);

  // Blueprint Class Defaults:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "cigi")
   FSisoID sisoID;///< SISO entity enumeration used to spawn entity
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "cigi")
  int32 shortEntityTypeID = 0;///< Short entity type ID
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "cigi")
  TMap<FString, int32> ArticulatedPartNames;///< Map of articulated part names to IDs
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "cigi")
  FString ArticulatedMeshName;///< Name of articulated mesh
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "cigi")
  TMap<FString, int32> ArticulatedBoneNames;///< Map of articulated bone names to IDs

  // Blueprint Read/Write Instance Variables:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "cigi")
  float EntityAlpha;///< Entity alpha (opacity)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "cigi")
  bool CollisionEnabled;///< Is collision enabled

  // Blueprint Read-Only Instance Variables:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi")
  int64 EntityID = 0;///< Unique CIGI entity ID

  /**
   * @brief Called when a CIGI component control packet is sent to this entity.
   * Allows CIGI packets to send information and commands directly to an entity's Blueprint.
   * @param ComponentID The component identifier.
   * @param ComponentState The state of the component.
   * @param ComponentData The component data payload.
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Cigi|Events")
  void OnComponentMessage(int32 ComponentID, int32 ComponentState, FComponentData ComponentData);

  /**
   * @brief Returns the component in this Entity that corresponds to the given Articulated Part ID.
   * @param ArticulatedPartID The articulated part ID.
   * @return Pointer to the scene component, or nullptr if not found.
   */
  UFUNCTION(BlueprintPure, Category = "cigi")
  USceneComponent* GetArticulatedPart(int32 ArticulatedPartID);

protected:
  /**
   * @brief Called when the game starts or when spawned.
   */
  virtual void BeginPlay() override;

  /**
   * @brief Called every frame.
   * @param deltaSeconds Time since last tick.
   */
  virtual void Tick(float deltaSeconds) override;

private:
  // Hide AActor's location and rotation functions because we are overriding them with CigiEntity engine-specific versions
  using AActor::GetActorLocation;
  using AActor::GetActorRotation;
  using AActor::SetActorLocation;
  using AActor::SetActorRotation;

protected:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  UStaticMeshComponent* StaticMeshComponent;///< Static mesh component
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  USkeletalMeshComponent* SkeletalMeshComponent;///< Skeletal mesh component
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  UPoseableMeshComponent* ArticulatedMesh;///< Poseable mesh component

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  TMap<int32, USceneComponent*> ArticulatedParts;///< Map of articulated part IDs to components
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  TMap<int32, FArticulatedBoneData> ArticulatedBones;///< Map of articulated bone IDs to data

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  TMap<int32, FCollisionSegment> CollisionSegments;///< Map of collision segment IDs to segments
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  TMap<int32, UShapeComponent*> CollisionVolumes;///< Map of collision volume IDs to shape components

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  TMap<int32, UWidgetComponent*> Widgets;///< Map of widget IDs to widget components

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "cigi private")
  TMap<int32, bool> IsBillboard;///< Map of widget IDs to billboard state
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026