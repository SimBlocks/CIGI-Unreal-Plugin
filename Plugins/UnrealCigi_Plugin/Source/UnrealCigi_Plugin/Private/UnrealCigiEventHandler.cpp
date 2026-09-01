//Copyright SimBlocks LLC 2016-2026

// Unreal
#include "UnrealCigiEventHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "unrealcigiUtil.h"
#include "CigiBPLib.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiSymbolManager.h"
#include "UnrealCigiSymbolSurfacePresenter.h"
#include "UnrealCigiConfigLoader.h"
#include "UnrealCigiEntityManager.h"
#include "UnrealCigiDatabaseManager.h"
#include "UnrealCigiComponentDispatcher.h"
#include "UnrealCigiEnvironmentManager.h"
#include "UnrealCigiViewManager.h"
#include "UnrealCigiEnvironmentEventHandler.h"
#include "UnrealCigiDatabaseEventHandler.h"
#include "UnrealCigiViewEventHandler.h"
#include "UnrealCigiSymbolEventHandler.h"
#include "UnrealCigiEntityEventHandler.h"
#include "UnrealCigiTerrainEventHandler.h"
#include "UnrealCigiPhysicsEventHandler.h"
#include "UnrealCigiSensorEventHandler.h"
#include "UnrealCigiSystemEventHandler.h"
#undef UpdateResource
#include "unrealcigiPlayerController.h"
#include "CigiWidget.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "Engine/World.h"
#include "UObject/UObjectBaseUtility.h"
#include "EntityConfig.h"
#include "SymbolConfig.h"
#include "CigiView.h"
#include "CigiController.h"
#include "Engine/LevelStreamingDynamic.h"

// simulationsdk
#include "EngineLib/ImageGeneratorMessages.h"
#include "ViewLib/View.h"
#include "ViewLib/ViewManager.h"
#include "IGCigiLib/IGCigiLib.h"
#include "IGCigiLib/CigiView.h"
#include "IGCigiLib/CigiViewGroup.h"
#include "IGCigiLib/CigiEvent.h"
#include "IGCigiLib/ImageGenerator.h"
#include "unrealcigiCelestialEventHandler.h"
#include "CigiCoordinates.h"

#include "EngineUtils.h"

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include <MathLib/CoordinateConversions.h>
#include <unordered_map>
#include <memory>

DEFINE_LOG_CATEGORY(LogCigiEventHandler)

using namespace sbio;
using namespace sbio::cigi;
using namespace sbio::ig;
using namespace sbio::symbol;
using namespace sbio::unrealcigi;
using namespace sbio::unrealcigi::utils;

CUnrealCigiEventHandler::CUnrealCigiEventHandler()
{
  m_pCelestialEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiCelestialEventHandler>();
  m_pEnvironmentEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiEnvironmentEventHandler>();
  m_pDatabaseEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiDatabaseEventHandler>();
  m_pViewEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiViewEventHandler>(*this);
  m_pSymbolEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiSymbolEventHandler>(*this);
  m_pEntityEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiEntityEventHandler>(*this);
  m_pTerrainEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiTerrainEventHandler>(*this);
  m_pPhysicsEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiPhysicsEventHandler>(*this);
  m_pSensorEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiSensorEventHandler>(*this);
  m_pSystemEventHandler = std::make_unique<sbio::unrealcigi::CUnrealCigiSystemEventHandler>(*this);
}

UWorld* CUnrealCigiEventHandler::GetWorld() const
{
  return m_World.Get();
}

CUnrealCigiEventHandler::~CUnrealCigiEventHandler()
{
  FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
  CUnrealCigiConfigLoader::ReleaseRootedConfigObjects();
}

// NOTE: This must be called ONCE in unreal's Tick() function (doesn't work properly if called in BeginPlay())
void CUnrealCigiEventHandler::Initialize(UWorld* world)
{
  m_World = world;

  // Clear all maps, pointers and other generated data
  CUnrealCigiConfigLoader::ReleaseRootedConfigObjects();

  FUnrealCigi_PluginModule::globals.pDatabaseManager->Reset();
  FUnrealCigi_PluginModule::globals.pUnrealViewManager->Reset();
  FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Reset();
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->ClearFonts();
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->ClearSurfaces();
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->ClearTextures();
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->ClearRenderCaches();
  m_World = nullptr;

  // If it is not specified later, the origin defaults to 0Â°N 0Â°E. Make sure this resets for each Initialize.
  CigiCoordinates::SetGeodeticOrigin(SGeodeticCoordinates());

  // Store world ref, crash if world ref is invalid (since nothing in the project will work without it)
  m_World = world;
  if (!IsValid(GetWorld()))
  {
    UE_LOG(LogCigiEventHandler, Fatal, TEXT("CIGI Initialize: Invalid world reference! EventHandler must receive a valid world reference to interact with the scene!"));
  }

  FUnrealCigi_PluginModule::globals.pDatabaseManager->SetWorld(world);
  FUnrealCigi_PluginModule::globals.pUnrealEntityManager->SetWorld(world);
  FUnrealCigi_PluginModule::globals.pUnrealViewManager->SetWorld(world);

  // Find all of the CigiController actors in the world and store them in the CigiControllers array.
  // This is done here because it is possible that the controllers are not yet spawned when BeginPlay() is called, so wait until Tick() to find them.
  TArray<AActor*> controllerActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACigiController::StaticClass(), controllerActors);
  for (AActor* actor : controllerActors)
  {
    ACigiController* controller = Cast<ACigiController>(actor);
    if (!IsValid(controller))
    {
      continue;
    }

    FUnrealCigi_PluginModule::globals.pComponentDispatcher->AddController(controller);
  }

  // Since we are in Tick, the world is loaded. Look for a GeoReferencingSystem. (This uses WorldRef)
  CigiCoordinates::TryFindGeoReferencingSystem();

  // Retrieve all of the fonts that exist in this project
  TArray<UFont*> projectFonts = sbio::unrealcigi::utils::FindAllFontAssets();
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->SetProjectFonts(projectFonts);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("Initialize: Loaded %d fonts"), projectFonts.Num());

  // Try to find the json config file and try to load entity configs, databases
  CUnrealCigiConfigLoader::LoadConfig(*m_pViewEventHandler);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("CIGI Initialize: Finished loading JSON data"));

  // Find any Cigi Entity Blueprints and try to load entity configs
  FUnrealCigi_PluginModule::globals.pUnrealEntityManager->LoadBlueprints();
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("CIGI Initialize: Finished loading CigiEntity Blueprints"));

  FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
  FWorldDelegates::LevelAddedToWorld.AddRaw(this, &CUnrealCigiEventHandler::OnLevelAddedToWorld);

  if (FUnrealCigi_PluginModule::globals.pImageGenerator->GetSetupOptions().bDatabaseControlledByIG)
  {
    m_pDatabaseEventHandler->LoadDatabase(FUnrealCigi_PluginModule::globals.pImageGenerator->GetSetupOptions().defaultIGControlledDatabaseID.Value(), GetWorld(), *m_pCelestialEventHandler);
  }
}

void CUnrealCigiEventHandler::OnLevelAddedToWorld(ULevel* pLevel, UWorld* pWorld)
{
  m_pDatabaseEventHandler->OnLevelAddedToWorld(pLevel, pWorld);
}

void CUnrealCigiEventHandler::OnCreateEntityMessage(const sbio::ig::entity::SCreateEntityMessage& data)
{
  m_pEntityEventHandler->OnCreateEntityMessage(data);
}

void CUnrealCigiEventHandler::OnDestroyEntityMessage(const sbio::ig::entity::SDestroyEntityMessage& data)
{
  m_pEntityEventHandler->OnDestroyEntityMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateTopLevelEntityTransformMessage(const sbio::ig::entity::SUpdateTopLevelEntityTransformMessage& data)
{
  m_pEntityEventHandler->OnUpdateTopLevelEntityTransformMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateChildEntityTransformMessage(const sbio::ig::entity::SUpdateChildEntityTransformMessage& data)
{
  m_pEntityEventHandler->OnUpdateChildEntityTransformMessage(data);
}

void CUnrealCigiEventHandler::OnSetEntityAttachedMessage(const sbio::ig::entity::SSetEntityAttachedMessage& data)
{
  m_pEntityEventHandler->OnSetEntityAttachedMessage(data);
}

void CUnrealCigiEventHandler::OnSetEntityActiveMessage(const sbio::ig::entity::SSetEntityActiveMessage& data)
{
  m_pEntityEventHandler->OnSetEntityActiveMessage(data);
}

void CUnrealCigiEventHandler::OnSetEntityUnattachedMessage(const sbio::ig::entity::SSetEntityUnattachedMessage& data)
{
  m_pEntityEventHandler->OnSetEntityUnattachedMessage(data);
}

void CUnrealCigiEventHandler::OnSetEntityComponentStateMessage(const sbio::ig::entity::SSetEntityComponentStateMessage& data)
{
  m_pEntityEventHandler->OnSetEntityComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateArticulatedPartTransformMessage(const sbio::ig::entity::SUpdateArticulatedPartTransformMessage& data)
{
  m_pEntityEventHandler->OnUpdateArticulatedPartTransformMessage(data);
}

void CUnrealCigiEventHandler::OnSetArticulatedPartVisibleMessage(const sbio::ig::entity::SSetArticulatedPartVisibleMessage& data)
{
  m_pEntityEventHandler->OnSetArticulatedPartVisibleMessage(data);
}

void CUnrealCigiEventHandler::OnSetEntityAlphaMessage(const sbio::ig::entity::SSetEntityAlphaMessage& data)
{
  m_pEntityEventHandler->OnSetEntityAlphaMessage(data);
}

void CUnrealCigiEventHandler::OnSetEntityCollisionDetectionEnabledMessage(const sbio::ig::entity::SSetEntityCollisionDetectionEnabledMessage& data)
{
  m_pEntityEventHandler->OnSetEntityCollisionDetectionEnabledMessage(data);
}

bool CUnrealCigiEventHandler::IsPointInEntityVolume(const sbio::math::GeocentricCoordinates& point, sbio::EntityID entityID) const
{
  return m_pEntityEventHandler->IsPointInEntityVolume(point, entityID);
}

void CUnrealCigiEventHandler::OnLineOfSightSegmentRequestBasicMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestBasicMessage& data)
{
  m_pTerrainEventHandler->OnLineOfSightSegmentRequestBasicMessage(data);
}

void CUnrealCigiEventHandler::OnLineOfSightSegmentRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestExtendedMessage& data)
{
  m_pTerrainEventHandler->OnLineOfSightSegmentRequestExtendedMessage(data);
}

void CUnrealCigiEventHandler::OnLineOfSightVectorRequestBasicMessage(const sbio::ig::terrain::SLineOfSightVectorRequestBasicMessage& data)
{
  m_pTerrainEventHandler->OnLineOfSightVectorRequestBasicMessage(data);
}

void CUnrealCigiEventHandler::OnLineOfSightVectorRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightVectorRequestExtendedMessage& data)
{
  m_pTerrainEventHandler->OnLineOfSightVectorRequestExtendedMessage(data);
}

void CUnrealCigiEventHandler::OnHeightAboveTerrainRequestMessage(const sbio::ig::terrain::SHeightAboveTerrainRequestMessage& data)
{
  m_pTerrainEventHandler->OnHeightAboveTerrainRequestMessage(data);
}

void CUnrealCigiEventHandler::OnHeightOfTerrainRequestMessage(const sbio::ig::terrain::SHeightOfTerrainRequestMessage& data)
{
  m_pTerrainEventHandler->OnHeightOfTerrainRequestMessage(data);
}

void CUnrealCigiEventHandler::OnTerrestrialSurfaceConditionsChangedMessage()
{
  m_pTerrainEventHandler->OnTerrestrialSurfaceConditionsChangedMessage();
}

void CUnrealCigiEventHandler::OnSetRegionalTerrainSurfaceComponentStateMessage(const sbio::ig::terrain::SSetRegionalTerrainSurfaceComponentStateMessage& data)
{
  m_pTerrainEventHandler->OnSetRegionalTerrainSurfaceComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetGlobalTerrainComponentStateMessage(const sbio::ig::terrain::SSetGlobalTerrainComponentStateMessage& data)
{
  m_pTerrainEventHandler->OnSetGlobalTerrainComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnCreateCollisionDetectionSegmentMessage(const sbio::ig::physics::SCreateCollisionDetectionSegmentMessage& data)
{
  m_pPhysicsEventHandler->OnCreateCollisionDetectionSegmentMessage(data);
}

void CUnrealCigiEventHandler::OnSetCollisionDetectionSegmentMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentMessage& data)
{
  m_pPhysicsEventHandler->OnSetCollisionDetectionSegmentMessage(data);
}

void CUnrealCigiEventHandler::OnSetCollisionDetectionSegmentEnabledMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentEnabledMessage& data)
{
  m_pPhysicsEventHandler->OnSetCollisionDetectionSegmentEnabledMessage(data);
}

void CUnrealCigiEventHandler::OnSetCollisionVolumeMessage(const sbio::ig::physics::SSetCollisionVolumeMessage& data)
{
  m_pPhysicsEventHandler->OnSetCollisionVolumeMessage(data);
}

void CUnrealCigiEventHandler::OnCreateCollisionVolumeSphereMessage(const sbio::ig::physics::SCreateCollisionVolumeSphereMessage& data)
{
  m_pPhysicsEventHandler->OnCreateCollisionVolumeSphereMessage(data);
}

void CUnrealCigiEventHandler::OnCreateCollisionVolumeCuboidMessage(const sbio::ig::physics::SCreateCollisionVolumeCuboidMessage& data)
{
  m_pPhysicsEventHandler->OnCreateCollisionVolumeCuboidMessage(data);
}

void CUnrealCigiEventHandler::OnSetCollisionVolumeEnabledMessage(const sbio::ig::physics::SSetCollisionVolumeEnabledMessage& data)
{
  m_pPhysicsEventHandler->OnSetCollisionVolumeEnabledMessage(data);
}

void CUnrealCigiEventHandler::OnDestroyCollisionVolumeMessage(const sbio::ig::physics::SDestroyCollisionVolumeMessage& data)
{
  m_pPhysicsEventHandler->OnDestroyCollisionVolumeMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateAttachedCameraTransformMessage(const sbio::ig::view::SUpdateAttachedCameraTransformMessage& data)
{
  m_pViewEventHandler->OnUpdateAttachedCameraTransformMessage(data);
}

void CUnrealCigiEventHandler::OnSetCameraAttachedToEntityMessage(const sbio::ig::view::SSetCameraAttachedToEntityMessage& data)
{
  m_pViewEventHandler->OnSetCameraAttachedToEntityMessage(data);
}

void CUnrealCigiEventHandler::OnSetCameraUnattachedMessage(const sbio::ig::view::SSetCameraUnattachedMessage& data)
{
  m_pViewEventHandler->OnSetCameraUnattachedMessage(data);
}

void CUnrealCigiEventHandler::OnSetCameraProjectionMessage(const sbio::ig::view::SSetCameraProjectionMessage& data)
{
  m_pViewEventHandler->OnSetCameraProjectionMessage(data);
}

void CUnrealCigiEventHandler::OnSetViewComponentStateMessage(const sbio::ig::view::SSetViewComponentStateMessage& data)
{
  m_pViewEventHandler->OnSetViewComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetViewGroupComponentStateMessage(const sbio::ig::view::SSetViewGroupComponentStateMessage& data)
{
  m_pViewEventHandler->OnSetViewGroupComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnUnloadDatabaseMessage()
{
  m_pDatabaseEventHandler->OnUnloadDatabase(GetWorld(), *m_pCelestialEventHandler);
}

void CUnrealCigiEventHandler::OnLoadDatabaseMessage(const sbio::ig::database::SLoadDatabaseMessage& data)
{
  m_pDatabaseEventHandler->LoadDatabase(data.DatabaseID.Value(), GetWorld(), *m_pCelestialEventHandler);
}

void CUnrealCigiEventHandler::OnSetAnimationDirectionMessage(const sbio::ig::animation::SSetAnimationDirectionMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnSetAnimationLoopModeMessage(const sbio::ig::animation::SSetAnimationLoopModeMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnSetAnimationSpeedMessage(const sbio::ig::animation::SSetAnimationSpeedMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnStopEntityAnimationMessage(const sbio::ig::animation::SStopEntityAnimationMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnStopAtCurrentFrameEntityAnimationMessage(const sbio::ig::animation::SStopAtCurrentFrameEntityAnimationMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnPauseEntityAnimationMessage(const sbio::ig::animation::SPauseEntityAnimationMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnPlayEntityAnimationMessage(const sbio::ig::animation::SPlayEntityAnimationMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnRestartEntityAnimationMessage(const sbio::ig::animation::SRestartEntityAnimationMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Animations are not currently supported"));
}

void CUnrealCigiEventHandler::OnCreateSymbolTextMessage(const sbio::ig::symbol::SCreateSymbolTextMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolTextMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolTextMessage(const sbio::ig::symbol::SUpdateSymbolTextMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolTextMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolColorMessage(const sbio::ig::symbol::SSetSymbolColorMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolColorMessage(data);
}

void CUnrealCigiEventHandler::OnDestroySymbolMessage(const sbio::ig::symbol::SDestroySymbolMessage& data)
{
  m_pSymbolEventHandler->OnDestroySymbolMessage(data);
}

void CUnrealCigiEventHandler::OnCreateSymbolCircleMessage(const sbio::ig::symbol::SCreateSymbolCircleMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolCircleMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolCircleMessage(const sbio::ig::symbol::SUpdateSymbolCircleMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolCircleMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolCircleElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleElementMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolCircleElementMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolCircleFilledMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolCircleFilledMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolCircleFilledElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledElementMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolCircleFilledElementMessage(data);
}

void CUnrealCigiEventHandler::OnCreateSymbolTexturedCircleMessage(const sbio::ig::symbol::SCreateSymbolTexturedCircleMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolTexturedCircleMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolTexturedCircleMessage(const sbio::ig::symbol::SUpdateSymbolTexturedCircleMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolTexturedCircleMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateTexturedCircleMessage(const sbio::ig::symbol::SUpdateTexturedCircleMessage& data)
{
  m_pSymbolEventHandler->OnUpdateTexturedCircleMessage(data);
}

void CUnrealCigiEventHandler::OnCreateSymbolPolygonMessage(const sbio::ig::symbol::SCreateSymbolPolygonMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolPolygonMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolPolygonMessage(const sbio::ig::symbol::SUpdateSymbolPolygonMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolPolygonMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolPolygonVertexMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolPolygonVertexMessage(data);
}

void CUnrealCigiEventHandler::OnCreateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SCreateSymbolTexturedPolygonMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolTexturedPolygonMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SUpdateSymbolTexturedPolygonMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolTexturedPolygonMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolTexturedPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolTexturedPolygonVertexMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolTexturedPolygonVertexMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateEntityBillboardSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateEntityBillboardSymbolSurfaceMessage& data)
{
  m_pSymbolEventHandler->OnUpdateEntityBillboardSymbolSurfaceMessage(data);
}

void CUnrealCigiEventHandler::OnCreateSymbolSurfaceMessage(const sbio::ig::symbol::SCreateSymbolSurfaceMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolSurfaceMessage(data);
}

void CUnrealCigiEventHandler::OnDestroySymbolSurfaceMessage(const sbio::ig::symbol::SDestroySymbolSurfaceMessage& data)
{
  m_pSymbolEventHandler->OnDestroySymbolSurfaceMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateSymbolSurfaceMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolSurfaceMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateViewSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateViewSymbolSurfaceMessage& data)
{
  m_pSymbolEventHandler->OnUpdateViewSymbolSurfaceMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolVisibleMessage(const sbio::ig::symbol::SSetSymbolVisibleMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolVisibleMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolAttachedMessage(const sbio::ig::symbol::SSetSymbolAttachedMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolAttachedMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolUnattachedMessage(const sbio::ig::symbol::SSetSymbolUnattachedMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolUnattachedMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolSurfaceMessage(const sbio::ig::symbol::SSetSymbolSurfaceMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolSurfaceMessage(data);
}

void CUnrealCigiEventHandler::OnSetTopLevelSymbolTransformMessage(const sbio::ig::symbol::SSetTopLevelSymbolTransformMessage& data)
{
  m_pSymbolEventHandler->OnSetTopLevelSymbolTransformMessage(data);
}

void CUnrealCigiEventHandler::OnSetChildSymbolTransformMessage(const sbio::ig::symbol::SSetChildSymbolTransformMessage& data)
{
  m_pSymbolEventHandler->OnSetChildSymbolTransformMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSymbolMessage(const sbio::ig::symbol::SUpdateSymbolMessage& data)
{
  m_pSymbolEventHandler->OnUpdateSymbolMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolComponentStateMessage(const sbio::ig::symbol::SSetSymbolComponentStateMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetSymbolSurfaceComponentStateMessage(const sbio::ig::symbol::SSetSymbolSurfaceComponentStateMessage& data)
{
  m_pSymbolEventHandler->OnSetSymbolSurfaceComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnCreateSymbolFromTemplateMessage(const sbio::ig::symbol::SCreateSymbolFromTemplateMessage& data)
{
  m_pSymbolEventHandler->OnCreateSymbolFromTemplateMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateCelestialSphereMessage(const sbio::ig::celestial::SUpdateCelestialSphereMessage& data)
{
  m_pCelestialEventHandler->UpdateCelestialSphereMessage(data, GetWorld());
}

void CUnrealCigiEventHandler::OnUpdateDateTimeMessage(const sbio::ig::celestial::SUpdateDateTimeMessage& data)
{
  m_pCelestialEventHandler->UpdateDateTimeMessage(data, GetWorld());
}

void CUnrealCigiEventHandler::OnSetCelestialSphereComponentStateMessage(const sbio::ig::celestial::SSetCelestialSphereComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::CELESTIAL_SPHERE, data.ComponentID.Value(), data.ComponentState, data.CelestialSphereID.Value(), data.ComponentData);
}

void CUnrealCigiEventHandler::OnUpdateSensorMessage(const sbio::ig::sensor::SUpdateSensorMessage& data)
{
  m_pSensorEventHandler->OnUpdateSensorMessage(data);
}

void CUnrealCigiEventHandler::OnUpdateSensorComponentMessage(const sbio::ig::sensor::SUpdateSensorComponentMessage& data)
{
  m_pSensorEventHandler->OnUpdateSensorComponentMessage(data);
}

void CUnrealCigiEventHandler::OnCreateMotionTrackerViewMessage(const sbio::ig::sensor::SCreateMotionTrackerViewMessage& data)
{
  m_pSensorEventHandler->OnCreateMotionTrackerViewMessage(data);
}

void CUnrealCigiEventHandler::OnCreateMotionTrackerViewGroupMessage(const sbio::ig::sensor::SCreateMotionTrackerViewGroupMessage& data)
{
  m_pSensorEventHandler->OnCreateMotionTrackerViewGroupMessage(data);
}

void CUnrealCigiEventHandler::OnSetMotionTrackerMessage(const sbio::ig::sensor::SSetMotionTrackerMessage& data)
{
  m_pSensorEventHandler->OnSetMotionTrackerMessage(data);
}

void CUnrealCigiEventHandler::OnSetAtmosphereEnabledMessage(const sbio::ig::atmosphere::SSetAtmosphereEnabledMessage& data)
{
  m_pEnvironmentEventHandler->OnSetAtmosphereEnabledMessage(data);
}

void CUnrealCigiEventHandler::OnSetAtmosphereMessage(const sbio::ig::atmosphere::SSetAtmosphereMessage& data)
{
  m_pEnvironmentEventHandler->OnSetAtmosphereMessage(data);
}

void CUnrealCigiEventHandler::OnSetWeatherMessage(const sbio::ig::atmosphere::SSetWeatherMessage& data)
{
  m_pEnvironmentEventHandler->OnSetWeatherMessage(data);
}

void CUnrealCigiEventHandler::OnSetRegionalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetRegionalLayeredWeatherComponentStateMessage& data)
{
  m_pEnvironmentEventHandler->OnSetRegionalLayeredWeatherComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetGlobalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetGlobalLayeredWeatherComponentStateMessage& data)
{
  m_pEnvironmentEventHandler->OnSetGlobalLayeredWeatherComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetAtmosphereComponentStateMessage(const sbio::ig::atmosphere::SSetAtmosphereComponentStateMessage& data)
{
  m_pEnvironmentEventHandler->OnSetAtmosphereComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetMaritimeSurfaceConditionsMessage(const sbio::ig::ocean::SSetMaritimeSurfaceConditionsMessage& data)
{
  m_pEnvironmentEventHandler->OnSetMaritimeSurfaceConditionsMessage(data);
}

void CUnrealCigiEventHandler::OnSetRegionMaritimeComponentStateMessage(const sbio::ig::ocean::SSetRegionMaritimeComponentStateMessage& data)
{
  m_pEnvironmentEventHandler->OnSetRegionMaritimeComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetGlobalMaritimeComponentStateMessage(const sbio::ig::ocean::SSetGlobalMaritimeComponentStateMessage& data)
{
  m_pEnvironmentEventHandler->OnSetGlobalMaritimeComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetEarthReferenceModelMessage(const sbio::ig::earth::SSetEarthReferenceModelMessage& data)
{
  m_pEnvironmentEventHandler->OnSetEarthReferenceModelMessage(data);
}

void CUnrealCigiEventHandler::OnSetEventComponentStateMessage(const sbio::ig::system::SSetEventComponentStateMessage& data)
{
  m_pSystemEventHandler->OnSetEventComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetSystemComponentStateMessage(const sbio::ig::system::SSetSystemComponentStateMessage& data)
{
  m_pSystemEventHandler->OnSetSystemComponentStateMessage(data);
}

void CUnrealCigiEventHandler::OnSetHostConnectedMessage(const sbio::ig::network::SHostConnectedMessage& data)
{
  m_pSystemEventHandler->OnSetHostConnectedMessage(data);
}

void CUnrealCigiEventHandler::OnSetHostDisconnectedMessage(const sbio::ig::network::SHostDisconnectedMessage& data)
{
  m_pSystemEventHandler->OnSetHostDisconnectedMessage(data);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026