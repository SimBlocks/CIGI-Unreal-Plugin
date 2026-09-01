//Copyright SimBlocks LLC 2016-2026
/**
 * @file unrealcigiEventHandler.h
 * @brief Defines the CUnrealCigiEventHandler class and supporting types for handling CIGI IG events in Unreal Engine.
 *
 * This header provides:
 * - DatabaseInfo: Structure for tracking loaded database assets and their origins.
 * - CUnrealCigiEventHandler: Main event handler class for processing CIGI IG messages and managing entities, views, symbols, collision, weather, and database loading.
 * - Utility functions for asset path resolution, coordinate conversion, and symbol/entity/view lookup.
 *
 * Usage:
 * - CUnrealCigiEventHandler receives and processes IG messages, updating the Unreal scene accordingly.
 * - Provides access to entity, view, symbol, and weather management, as well as coordinate conversion utilities.
 * - Used as the central integration point between SimBlocks CIGI IG and Unreal Engine.
 */

#pragma once

#include "ModuleAPI.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include "EngineLib/IImageGeneratorEventMessenger.h"
#include "CigiSymbol.h"
#include "GlobalHeaders/CommonTypes.h"
#include "CigiBPLib.h"
#include "EntityConfig.h"
#include "SymbolConfig.h"
#include "UnrealCigi_Declarations.h"
#include <unordered_map>
#include <memory>
#include <vector>

class ACigiController;
class ACigiEntity;
class ACigiView;
class UWorld;
class ULevelStreamingDynamic;
class UTexture2D;
enum class ComponentClass : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogCigiEventHandler, Log, All)

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiDatabaseEventHandler;
    class CUnrealCigiEntityEventHandler;
    class CUnrealCigiEnvironmentEventHandler;
    class CUnrealCigiPhysicsEventHandler;
    class CUnrealCigiViewEventHandler;
    class CUnrealCigiSensorEventHandler;
    class CUnrealCigiSymbolEventHandler;
    class CUnrealCigiSystemEventHandler;
    class CUnrealCigiTerrainEventHandler;

    /**
     * @class CUnrealCigiEventHandler
     * @brief Main event handler for CIGI IG messages and Unreal scene management.
     *
     * Handles entity, view, symbol, collision, weather, and database events.
     * Provides utility functions for asset path resolution, coordinate conversion, and object lookup.
     */
    class CUnrealCigiEventHandler : public sbio::ig::IImageGeneratorEventHandler
    {
    public:
      /**
       * @brief Default constructor.
       */
      CUnrealCigiEventHandler();

      /**
       * @brief Destructor.
       */
      virtual ~CUnrealCigiEventHandler() override;

      /**
       * @brief Initializes the event handler with the given world. Should be called once on first update.
       * @param world Pointer to the Unreal world.
       */
      void Initialize(UWorld* world);

      /**
       * @brief Called when a level is added to the world.
       * @param pLevel Pointer to the level.
       * @param pWorld Pointer to the world.
       */
      UFUNCTION()
      void OnLevelAddedToWorld(ULevel* pLevel, UWorld* pWorld);

      // --- IG Message Handlers ---
      // Each override handles a specific IG message type.
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateEntityMessage(const sbio::ig::entity::SCreateEntityMessage& data) override;

      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnDestroyEntityMessage(const sbio::ig::entity::SDestroyEntityMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateTopLevelEntityTransformMessage(const sbio::ig::entity::SUpdateTopLevelEntityTransformMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateChildEntityTransformMessage(const sbio::ig::entity::SUpdateChildEntityTransformMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEntityAttachedMessage(const sbio::ig::entity::SSetEntityAttachedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEntityActiveMessage(const sbio::ig::entity::SSetEntityActiveMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEntityUnattachedMessage(const sbio::ig::entity::SSetEntityUnattachedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEntityComponentStateMessage(const sbio::ig::entity::SSetEntityComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateArticulatedPartTransformMessage(const sbio::ig::entity::SUpdateArticulatedPartTransformMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetArticulatedPartVisibleMessage(const sbio::ig::entity::SSetArticulatedPartVisibleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEntityAlphaMessage(const sbio::ig::entity::SSetEntityAlphaMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEntityCollisionDetectionEnabledMessage(const sbio::ig::entity::SSetEntityCollisionDetectionEnabledMessage& data) override;
      
      /**
       * @brief Determines if a point is within the volume of an entity.
       * @param point The point in geocentric coordinates to check.
       * @param entityID The ID of the entity to check against.
       * @return True if the point is within the entity's volume, false otherwise.
       */
      virtual bool IsPointInEntityVolume(const sbio::math::GeocentricCoordinates& point, sbio::EntityID entityID) const override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnLineOfSightSegmentRequestBasicMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestBasicMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnLineOfSightSegmentRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestExtendedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnLineOfSightVectorRequestBasicMessage(const sbio::ig::terrain::SLineOfSightVectorRequestBasicMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnLineOfSightVectorRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightVectorRequestExtendedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnHeightAboveTerrainRequestMessage(const sbio::ig::terrain::SHeightAboveTerrainRequestMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnHeightOfTerrainRequestMessage(const sbio::ig::terrain::SHeightOfTerrainRequestMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       */
      virtual void OnTerrestrialSurfaceConditionsChangedMessage() override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetRegionalTerrainSurfaceComponentStateMessage(const sbio::ig::terrain::SSetRegionalTerrainSurfaceComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetGlobalTerrainComponentStateMessage(const sbio::ig::terrain::SSetGlobalTerrainComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCollisionDetectionSegmentEnabledMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentEnabledMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCollisionDetectionSegmentMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateCollisionDetectionSegmentMessage(const sbio::ig::physics::SCreateCollisionDetectionSegmentMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateCollisionVolumeSphereMessage(const sbio::ig::physics::SCreateCollisionVolumeSphereMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateCollisionVolumeCuboidMessage(const sbio::ig::physics::SCreateCollisionVolumeCuboidMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCollisionVolumeEnabledMessage(const sbio::ig::physics::SSetCollisionVolumeEnabledMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCollisionVolumeMessage(const sbio::ig::physics::SSetCollisionVolumeMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnDestroyCollisionVolumeMessage(const sbio::ig::physics::SDestroyCollisionVolumeMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateAttachedCameraTransformMessage(const sbio::ig::view::SUpdateAttachedCameraTransformMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCameraAttachedToEntityMessage(const sbio::ig::view::SSetCameraAttachedToEntityMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCameraUnattachedMessage(const sbio::ig::view::SSetCameraUnattachedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCameraProjectionMessage(const sbio::ig::view::SSetCameraProjectionMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetViewComponentStateMessage(const sbio::ig::view::SSetViewComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetViewGroupComponentStateMessage(const sbio::ig::view::SSetViewGroupComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       */
      virtual void OnUnloadDatabaseMessage() override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnLoadDatabaseMessage(const sbio::ig::database::SLoadDatabaseMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetAnimationDirectionMessage(const sbio::ig::animation::SSetAnimationDirectionMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetAnimationLoopModeMessage(const sbio::ig::animation::SSetAnimationLoopModeMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetAnimationSpeedMessage(const sbio::ig::animation::SSetAnimationSpeedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnStopEntityAnimationMessage(const sbio::ig::animation::SStopEntityAnimationMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnStopAtCurrentFrameEntityAnimationMessage(const sbio::ig::animation::SStopAtCurrentFrameEntityAnimationMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnPauseEntityAnimationMessage(const sbio::ig::animation::SPauseEntityAnimationMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnPlayEntityAnimationMessage(const sbio::ig::animation::SPlayEntityAnimationMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnRestartEntityAnimationMessage(const sbio::ig::animation::SRestartEntityAnimationMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolTextMessage(const sbio::ig::symbol::SCreateSymbolTextMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolTextMessage(const sbio::ig::symbol::SUpdateSymbolTextMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolColorMessage(const sbio::ig::symbol::SSetSymbolColorMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnDestroySymbolMessage(const sbio::ig::symbol::SDestroySymbolMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolCircleMessage(const sbio::ig::symbol::SCreateSymbolCircleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolCircleMessage(const sbio::ig::symbol::SUpdateSymbolCircleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolCircleElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleElementMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolCircleFilledMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolCircleFilledElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledElementMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolTexturedCircleMessage(const sbio::ig::symbol::SCreateSymbolTexturedCircleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolTexturedCircleMessage(const sbio::ig::symbol::SUpdateSymbolTexturedCircleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateTexturedCircleMessage(const sbio::ig::symbol::SUpdateTexturedCircleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolPolygonMessage(const sbio::ig::symbol::SCreateSymbolPolygonMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolPolygonMessage(const sbio::ig::symbol::SUpdateSymbolPolygonMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolPolygonVertexMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SCreateSymbolTexturedPolygonMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SUpdateSymbolTexturedPolygonMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolTexturedPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolTexturedPolygonVertexMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateEntityBillboardSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateEntityBillboardSymbolSurfaceMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolSurfaceMessage(const sbio::ig::symbol::SCreateSymbolSurfaceMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnDestroySymbolSurfaceMessage(const sbio::ig::symbol::SDestroySymbolSurfaceMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateSymbolSurfaceMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateViewSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateViewSymbolSurfaceMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolVisibleMessage(const sbio::ig::symbol::SSetSymbolVisibleMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolAttachedMessage(const sbio::ig::symbol::SSetSymbolAttachedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolUnattachedMessage(const sbio::ig::symbol::SSetSymbolUnattachedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolSurfaceMessage(const sbio::ig::symbol::SSetSymbolSurfaceMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetTopLevelSymbolTransformMessage(const sbio::ig::symbol::SSetTopLevelSymbolTransformMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetChildSymbolTransformMessage(const sbio::ig::symbol::SSetChildSymbolTransformMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSymbolMessage(const sbio::ig::symbol::SUpdateSymbolMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolComponentStateMessage(const sbio::ig::symbol::SSetSymbolComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSymbolSurfaceComponentStateMessage(const sbio::ig::symbol::SSetSymbolSurfaceComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateSymbolFromTemplateMessage(const sbio::ig::symbol::SCreateSymbolFromTemplateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateCelestialSphereMessage(const sbio::ig::celestial::SUpdateCelestialSphereMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateDateTimeMessage(const sbio::ig::celestial::SUpdateDateTimeMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetCelestialSphereComponentStateMessage(const sbio::ig::celestial::SSetCelestialSphereComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSensorMessage(const sbio::ig::sensor::SUpdateSensorMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnUpdateSensorComponentMessage(const sbio::ig::sensor::SUpdateSensorComponentMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateMotionTrackerViewMessage(const sbio::ig::sensor::SCreateMotionTrackerViewMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnCreateMotionTrackerViewGroupMessage(const sbio::ig::sensor::SCreateMotionTrackerViewGroupMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetMotionTrackerMessage(const sbio::ig::sensor::SSetMotionTrackerMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetAtmosphereEnabledMessage(const sbio::ig::atmosphere::SSetAtmosphereEnabledMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetAtmosphereMessage(const sbio::ig::atmosphere::SSetAtmosphereMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetWeatherMessage(const sbio::ig::atmosphere::SSetWeatherMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetRegionalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetRegionalLayeredWeatherComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetGlobalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetGlobalLayeredWeatherComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetAtmosphereComponentStateMessage(const sbio::ig::atmosphere::SSetAtmosphereComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetMaritimeSurfaceConditionsMessage(const sbio::ig::ocean::SSetMaritimeSurfaceConditionsMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetRegionMaritimeComponentStateMessage(const sbio::ig::ocean::SSetRegionMaritimeComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetGlobalMaritimeComponentStateMessage(const sbio::ig::ocean::SSetGlobalMaritimeComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEarthReferenceModelMessage(const sbio::ig::earth::SSetEarthReferenceModelMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetEventComponentStateMessage(const sbio::ig::system::SSetEventComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetSystemComponentStateMessage(const sbio::ig::system::SSetSystemComponentStateMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetHostConnectedMessage(const sbio::ig::network::SHostConnectedMessage& data) override;
      
      /**
       * @brief Processes the corresponding CIGI message and updates the Unreal scene or simulation state.
       * @param data Message payload supplied by the image-generator event dispatcher.
       */
      virtual void OnSetHostDisconnectedMessage(const sbio::ig::network::SHostDisconnectedMessage& data) override;

    public:
      /**
       * @brief Returns the Unreal world used by the event handler.
       * @return Associated world, or nullptr when no world is available.
       */
      UWorld* GetWorld() const;

    private:
      /** @brief Celestial event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiCelestialEventHandler> m_pCelestialEventHandler;
      /** @brief Environment event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiEnvironmentEventHandler> m_pEnvironmentEventHandler;
      /** @brief Database event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiDatabaseEventHandler> m_pDatabaseEventHandler;
      /** @brief View event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiViewEventHandler> m_pViewEventHandler;
      /** @brief Symbol event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiSymbolEventHandler> m_pSymbolEventHandler;
      /** @brief Entity event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiEntityEventHandler> m_pEntityEventHandler;
      /** @brief Terrain event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiTerrainEventHandler> m_pTerrainEventHandler;
      /** @brief Physics event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiPhysicsEventHandler> m_pPhysicsEventHandler;
      /** @brief Sensor event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiSensorEventHandler> m_pSensorEventHandler;
      /** @brief System event handler, owned by this instance. */
      std::unique_ptr<CUnrealCigiSystemEventHandler> m_pSystemEventHandler;
      /** @brief Pointer to the Unreal world, weak referenced. */
      TWeakObjectPtr<UWorld> m_World;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026