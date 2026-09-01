//Copyright SimBlocks LLC 2016-2026
/**
 * @file CigiController.h
 * @brief Defines the ACigiController actor and related types for handling CIGI component and entity messages in Unreal Engine.
 *
 * This header provides:
 * - The ACigiController actor class, which exposes Blueprint events for handling CIGI entity and component control packets.
 * - The FComponentMessage struct, representing a CIGI component control message.
 * - The ComponentClass enum, describing supported CIGI component classes.
 *
 * Usage:
 * - Derive a Blueprint from ACigiController and place it in the level to receive CIGI events.
 * - Implement Blueprint events to respond to entity creation and component control packets.
 */

#pragma once

#include "ModuleAPI.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include "CigiEntity.h"
#include "CigiController.generated.h"

/**
 * @brief Represents a CIGI component control message.
 */
USTRUCT(BlueprintType)

struct FComponentMessage
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Cigi|ComponentMessage")
  int ComponentId;///< Component identifier
  UPROPERTY(BlueprintReadWrite, Category = "Cigi|ComponentMessage")
  int ComponentState;///< Component state value
  UPROPERTY(BlueprintReadWrite, Category = "Cigi|ComponentMessage")
  int InstanceId;///< Instance identifier
  UPROPERTY(BlueprintReadWrite, Category = "Cigi|ComponentMessage")
  FComponentData Data;///< Component data payload

  /**
   * @brief Constructs a component message.
   * @param componentId The component identifier.
   * @param componentState The state of the component.
   * @param instanceId The instance identifier.
   * @param data The component data payload.
   */
  FComponentMessage(int componentId, int componentState, int instanceId, FComponentData data);

  /**
   * @brief Constructs an empty component message.
   */
  FComponentMessage();

  /**
   * @brief Returns a string representation of the component message.
   * @return String describing the component message.
   */
  FString ToString();
};

/**
 * @brief Enumerates supported CIGI component classes.
 */
UENUM(BlueprintType)
enum class ComponentClass : uint8
{
  ENTITY UMETA(DisplayName = "Entity"),
  VIEW UMETA(DisplayName = "View"),
  VIEW_GROUP UMETA(DisplayName = "View Group"),
  SENSOR UMETA(DisplayName = "Sensor"),
  REGIONAL_MARITIME UMETA(DisplayName = "Regional Sea"),
  REGIONAL_TERRAIN UMETA(DisplayName = "Regional Terrain"),
  REGIONAL_WEATHER UMETA(DisplayName = "Regional Weather"),
  GLOBAL_MARITIME UMETA(DisplayName = "Global Sea"),
  GLOBAL_TERRAIN UMETA(DisplayName = "Global Terrain"),
  GLOBAL_WEATHER UMETA(DisplayName = "Global Weather"),
  ATMOSPHERE UMETA(DisplayName = "Atmosphere"),
  CELESTIAL_SPHERE UMETA(DisplayName = "Celestial Sphere"),
  EVENT UMETA(DisplayName = "Event"),
  SYSTEM UMETA(DisplayName = "System"),
  SYMBOL_SURFACE UMETA(DisplayName = "Symbol Surface"),
  SYMBOL UMETA(DisplayName = "Symbol")
};

/**
 * @brief Actor that exposes Blueprint events for handling CIGI entity and component control messages.
 *
 * Derive a Blueprint from this class and place it in the level to receive CIGI events.
 * Implement the Blueprint events to respond to entity creation and component control packets.
 */
UCLASS()

class MODULE_API ACigiController : public AActor
{
  GENERATED_BODY()

public:
  /**
   * @brief Constructs the controller actor.
   */
  ACigiController();

  /**
   * @brief Called when a CIGI entity control packet is received that creates a new entity.
   * Note: Ignore the warning. BlueprintImplementableEvents will not compile if they have function definitions.
   * @param entityType The SISO entity type.
   * @param entityId The unique entity identifier.
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Cigi|Events")
  void OnCreateEntity(FSisoID entityType, int entityId);

  /**
   * @brief Called when a CIGI component control packet is received for any non-Entity component class.
   * This allows CIGI messages to directly run Blueprint events in Unreal.
   * All fields from the CIGI packet are passed along to this Blueprint event.
   * (If the component class is Entity, then the packet is handled by CigiEntity's OnComponentMessage event)
   * Note 1: Ignore the warning. BlueprintImplementableEvents will not compile if they have function definitions.
   * Note 2: The FComponentMessage must be passed using UPARAM(ref) so that Unreal does not garbage collect it or its internal FComponentData.
   * @param componentType The type of component.
   * @param message The component message data.
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "Cigi|Events")
  void OnComponentMessage(ComponentClass componentType, UPARAM(ref) FComponentMessage& message);

protected:
  /**
   * @brief Called when play begins for this actor.
   */
  virtual void BeginPlay() override;
  /**
   * @brief Called when play ends for this actor.
   * @param EndPlayReason Reason the actor stopped playing.
   */
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026