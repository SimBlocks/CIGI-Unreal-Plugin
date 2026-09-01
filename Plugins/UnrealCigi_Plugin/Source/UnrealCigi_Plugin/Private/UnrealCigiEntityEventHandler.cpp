//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiEntityEventHandler.h"

#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiEntityManager.h"
#include "UnrealCigiPhysicsManager.h"
#include "CigiEntity.h"
#include "CigiController.h"
#include "EntityConfig.h"
#include "unrealcigiUtil.h"
#include "CigiCoordinates.h"
#include "UnrealCoordinates.h"
#include "CoreMinimal.h"

using namespace sbio;
using namespace sbio::unrealcigi;
using namespace sbio::unrealcigi::utils;

CUnrealCigiEntityEventHandler::CUnrealCigiEntityEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
{
}

void CUnrealCigiEntityEventHandler::OnCreateEntityMessage(const sbio::ig::entity::SCreateEntityMessage& data)
{
  // Check if the entity manager is available
  FString sEntityEnumeration;
  if (FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ShortTypeToExtended.Contains(data.ShortEntityTypeID.Value()))
  {
    sEntityEnumeration = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ShortTypeToExtended[data.ShortEntityTypeID.Value()];
  }
  else
  {
    // If the short entity type ID is not found, use the full entity type enumeration string as the sEntityEnumeration
    sEntityEnumeration = UTF8_TO_TCHAR(data.EntityType.ToEnumerationString().c_str());
    if (!FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Configs.Contains(sEntityEnumeration) && FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ShortTypeToExtended.Contains(data.ShortEntityTypeID.Value()))
    {
      sEntityEnumeration = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ShortTypeToExtended[data.ShortEntityTypeID.Value()];
    }
  }

  // Check if the entity configuration exists for the given sEntityEnumeration
  if (!FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Configs.Contains(sEntityEnumeration))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("CreateEntity: FAILED: No EntityConfig with enumeration=%s or shortID=%d! (make sure it's defined in the .config.json file!)"), *sEntityEnumeration, data.ShortEntityTypeID.Value());
    return;
  }

  UEntityConfig* config = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Configs[sEntityEnumeration];

  // Check if the config is valid and if an entity with the same ID already exists
  if (config == nullptr || !IsValid(config) || FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID) != nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("CreateEntity: FAILED: Invalid config or EntityID %d already exists"), data.EntityID.Value());
    return;
  }

  UWorld* world = EventHandler.GetWorld();
  if (!IsValid(world))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("CreateEntity: FAILED: Invalid world for sisoID='%s'"), *sEntityEnumeration);
    return;
  }

  ACigiEntity* newEntity = nullptr;
  if (config->blueprint != nullptr)
  {
    newEntity = world->SpawnActor<ACigiEntity>(config->blueprint);
    if (IsValid(newEntity))
    {
      newEntity->FindArticulatedParts();
    }
  }
  else if (!config->filepath.IsEmpty())
  {
    // If the config has a valid filepath, spawn a new ACigiEntity actor in the world
    newEntity = world->SpawnActor<ACigiEntity>(ACigiEntity::StaticClass());

    // If the new entity was successfully spawned, set its visual representation and articulated parts/bones based on the config
    if (IsValid(newEntity))
    {
      const bool articulateBones = config->articulatedBones.Num() > 0;
      newEntity->SetVisual(*config->filepath, config->transform, articulateBones);
#if WITH_EDITOR
      newEntity->SetActorLabel(config->name);
#endif
      // If the config has articulated parts, add them to the new entity
      for (TPair<int32, FArticulatedPartConfig>& partPair : config->articulatedParts)
      {
        const FArticulatedPartConfig& partConfig = partPair.Value;
        newEntity->AddArticulatedPart(ArticulatedPartID(partPair.Key), partConfig.origin, *partConfig.filepath, partConfig.transform);
      }

      // If the config has articulated bones, add them to the new entity
      if (articulateBones)
      {
        newEntity->AddArticulatedBones(config->articulatedBones);
      }
    }
  }
  else
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("CreateEntity: FAILED: sisoID='%s' has no Blueprint class and no JSON filepath"), *sEntityEnumeration);
    return;
  }

  // Check if the new entity was successfully spawned
  if (!IsValid(newEntity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("CreateEntity: FAILED: Could not spawn entity for sisoID='%s'"), *sEntityEnumeration);
    return;
  }

  FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Actors.Add(data.EntityID.Value(), newEntity);
  newEntity->EntityID = data.EntityID.Value();

  // Notify all controllers that a new entity has been created
  for (const TWeakObjectPtr<ACigiController>& controllerReference : FUnrealCigi_PluginModule::globals.pComponentDispatcher->Controllers)
  {
    if (ACigiController* controller = controllerReference.Get())
    {
      // Notify the controller that a new entity has been created
      if (IsValid(controller))
      {
         controller->OnCreateEntity(FSisoID(data.EntityType), data.EntityID.Value());
      }
    }
  }
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("CreateEntity: Spawned actor: name=%s"), *ObjName(newEntity));
}

void CUnrealCigiEntityEventHandler::OnUpdateTopLevelEntityTransformMessage(const sbio::ig::entity::SUpdateTopLevelEntityTransformMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateTopLevelEntityTransformMessage: FAILED: No Entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  // Convert the CIGI Euler angles to Unreal Engine rotation and the CIGI geodetic coordinates to Unreal Engine world coordinates
  const FUEWorldRotation unrealRot = FUEWorldRotation::From(FQuat::MakeFromEuler(FVector(data.EulerRotation.roll.Value(), data.EulerRotation.pitch.Value(), data.EulerRotation.yaw.Value())));
  FUEWorldCoordinates unrealXYZ(CigiCoordinates::GeodeticToEngine(data.Coordinate));

  // If the clamp flag is set, perform a line trace to find the terrain height at the entity's location and adjust the Z coordinate accordingly
  if (data.Clamp != 0 && IsValid(EventHandler.GetWorld()))
  {
    auto GetHeightAboveTerrainAtLocation = [this](const FUEWorldCoordinates& location, double& heightAboveTerrain) -> bool
    {
      // Convert the FUEWorldCoordinates to an FVector for the line trace
      const FVector locationVector = location.ToFVector();
      const FVector traceStart = locationVector + FVector(0.0, 0.0, 1000000.0);
      const FVector traceEnd = locationVector - FVector(0.0, 0.0, 1000000.0);
      FHitResult terrainResult;

      // Perform a line trace to find the terrain height at the entity's location
      if (!EventHandler.GetWorld()->LineTraceSingleByObjectType(terrainResult, traceStart, traceEnd, FCollisionObjectQueryParams::AllObjects))
      {
        return false;
      }

      // Calculate the height above terrain by converting both the entity's location and the terrain hit location to geodetic coordinates and subtracting their altitudes
      heightAboveTerrain = CigiCoordinates::EngineToGeodetic(location).altitude.Value() - CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(terrainResult.Location)).altitude.Value();
      return true;
    };

    // Perform a line trace to find the terrain height at the entity's location
    const FVector traceStart = unrealXYZ.ToFVector() + FVector(0.0, 0.0, 1000000.0);
    const FVector traceEnd = unrealXYZ.ToFVector() - FVector(0.0, 0.0, 1000000.0);
    FHitResult terrainHit;

    // Perform a line trace to find the terrain height at the entity's location
    if (EventHandler.GetWorld()->LineTraceSingleByObjectType(terrainHit, traceStart, traceEnd, FCollisionObjectQueryParams::AllObjects))
    {
      double desiredHeightAboveTerrain = data.Coordinate.altitude.Value();
      if (FMath::IsNearlyZero(desiredHeightAboveTerrain))
      {
        double currentHeightAboveTerrain = 0.0;
        if (GetHeightAboveTerrainAtLocation(entity->GetEngineLocation(), currentHeightAboveTerrain))
        {
          desiredHeightAboveTerrain = currentHeightAboveTerrain;
        }
      }
      desiredHeightAboveTerrain = FMath::Max(0.0, desiredHeightAboveTerrain);
      unrealXYZ = FUEWorldCoordinates(terrainHit.Location + FVector(0.0, 0.0, desiredHeightAboveTerrain * 100.0));
    }
    else
    {
      UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateTopLevelEntityTransformMessage: Clamp requested for entity %d but no terrain was found at pos=\"%s\""), data.EntityID.Value(), *unrealXYZ.ToFVector().ToString());
    }
  }

  // Update the entity's transform
  entity->SetEngineLocation(unrealXYZ);
  entity->SetEngineRotation(unrealRot);
  UE_LOG(LogCigiEventHandler, CIGI_VELOCITY, TEXT("OnUpdateTopLevelEntityTransformMessage: Updated entity \"%s\" with id=%d (clamp=%d) with pos=\"%s\" and euler=(roll=%.2f,pitch=%.2f,yaw=%.2f)"), *ObjName(entity), data.EntityID.Value(), data.Clamp, *unrealXYZ.ToFVector().ToString(),
         data.EulerRotation.roll.Value(), data.EulerRotation.pitch.Value(), data.EulerRotation.yaw.Value());
}

void CUnrealCigiEntityEventHandler::OnDestroyEntityMessage(const sbio::ig::entity::SDestroyEntityMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnDestroyEntityMessage: FAILED: No Entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  // Remove the entity from the manager's list before destroying it
  FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Actors.Remove(data.EntityID.Value());

  // Destroy the entity actor safely
  if (IsValid(entity))
  {
    entity->SetEnabled(false);
    if (IsValid(entity))
    {
      entity->K2_DestroyActor();
    }
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnDestroyEntityMessage: Destroyed entity with id=%d"), data.EntityID.Value());
}

void CUnrealCigiEntityEventHandler::OnUpdateChildEntityTransformMessage(const sbio::ig::entity::SUpdateChildEntityTransformMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity and its root component are valid before proceeding
  if (!IsValid(entity) || !IsValid(entity->GetRootComponent()))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateChildEntityTransformMessage: FAILED: No valid entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  // Check if the entity has a valid parent before updating its transform
  if (!IsValid(entity->GetAttachParentActor()))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateChildEntityTransformMessage: FAILED: Target entity \"%s\" does not have a parent"), *ObjName(entity));
    return;
  }

  // Update the relative transform of the child entity based on the provided offset and rotation
  entity->GetRootComponent()->SetRelativeTransform(BodyTransformToFTransform(data.Offset, data.Rotation));
  UE_LOG(LogCigiEventHandler, CIGI_VELOCITY, TEXT("OnUpdateChildEntityTransformMessage: Child=\"%s\", Parent=\"%s\""), *ObjName(entity), *entity->GetAttachParentActor()->GetName());
  UE_LOG(LogCigiEventHandler, CIGI_VELOCITY, TEXT("OnUpdateChildEntityTransformMessage: RELATIVE pos=\"%s\", rot=\"%s\""), *entity->GetRootComponent()->GetRelativeLocation().ToString(), *entity->GetRootComponent()->GetRelativeRotation().Euler().ToString());
}

void CUnrealCigiEntityEventHandler::OnSetEntityAttachedMessage(const sbio::ig::entity::SSetEntityAttachedMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityAttachedMessage: FAILED: No Entity with EntityID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  // Find the parent entity by its ID
  ACigiEntity* parent = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.ParentID);

  // Check if the parent entity is valid before proceeding
  if (!IsValid(parent))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityAttachedMessage: FAILED: No Entity with ParentID=%d exists in the scene!"), data.ParentID.Value());
    return;
  }

  entity->AttachToActor(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetEntityAttachedMessage: attached entity \"%s\" to parent \"%s\""), *ObjName(entity), *ObjName(parent));
}

void CUnrealCigiEntityEventHandler::OnSetEntityActiveMessage(const sbio::ig::entity::SSetEntityActiveMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("SetEntityActive: FAILED: No Entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  entity->SetEnabled(data.isActive);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("SetEntityActive: entity ID=%d is now %s"), data.EntityID.Value(), data.isActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
}

void CUnrealCigiEntityEventHandler::OnSetEntityUnattachedMessage(const sbio::ig::entity::SSetEntityUnattachedMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityUnattachedMessage: FAILED: No Entity with EntityID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  entity->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetEntityUnattachedMessage: entity \"%s\" is now unattached! (parent=null)"), *ObjName(entity));
}

void CUnrealCigiEntityEventHandler::OnSetEntityComponentStateMessage(const sbio::ig::entity::SSetEntityComponentStateMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityComponentStateMessage: FAILED: No Entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  const sbio::ig::SComponentData& compData = data.ComponentData;
  entity->OnComponentMessage(data.ComponentID.Value(), data.ComponentState, FComponentData(compData.ComponentData0, compData.ComponentData1, compData.ComponentData2, compData.ComponentData3, compData.ComponentData4, compData.ComponentData5));
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetEntityComponentStateMessage: found Entity \"%s\", sent OnComponentMessage(ID=%d, State=%d, Data=[%d,%d,%d,%d,%d,%d]) to it's Blueprint"), *ObjName(entity), data.ComponentID.Value(), data.ComponentState, compData.ComponentData0,
         compData.ComponentData1, compData.ComponentData2, compData.ComponentData3, compData.ComponentData4, compData.ComponentData5);
}

void CUnrealCigiEntityEventHandler::OnUpdateArticulatedPartTransformMessage(const sbio::ig::entity::SUpdateArticulatedPartTransformMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateArticulatedPartTransformMessage: FAILED: No Entity with EntityID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  FTransform transform = BodyTransformToFTransform(data.Offset, data.Rotation);
  entity->UpdateArticulatedPartTransform(data.ArticulatedPartID, transform);
  UE_LOG(LogCigiEventHandler, CIGI_VELOCITY, TEXT("OnUpdateArticulatedPartTransformMessage: in entity \"%s\" updated AP %d with transform \"%s\", Rot={%.3f,%.3f,%.3f}"), *ObjName(entity), data.ArticulatedPartID.Value(), *transform.ToString(), transform.Rotator().Euler().X,
         transform.Rotator().Euler().Y, transform.Rotator().Euler().Z);
}

void CUnrealCigiEntityEventHandler::OnSetArticulatedPartVisibleMessage(const sbio::ig::entity::SSetArticulatedPartVisibleMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetArticulatedPartVisibleMessage: FAILED: No Entity with EntityID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  entity->SetArticulatedPartEnabled(data.ArticulatedPartID, data.Visible);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetArticulatedPartVisibleMessage: Set AP %d to %s"), data.ArticulatedPartID.Value(), data.Visible ? TEXT("Enabled") : TEXT("Disabled"));
}

void CUnrealCigiEntityEventHandler::OnSetEntityAlphaMessage(const sbio::ig::entity::SSetEntityAlphaMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityAlphaMessage: FAILED: No Entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  entity->EntityAlpha = data.Alpha;
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetEntityAlphaMessage: Entity \"%s\" with id %d now has Alpha=%f"), *ObjName(entity), entity->EntityID, entity->EntityAlpha);
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityAlphaMessage: Changes to the alpha currently have no visible effect."));
}

void CUnrealCigiEntityEventHandler::OnSetEntityCollisionDetectionEnabledMessage(const sbio::ig::entity::SSetEntityCollisionDetectionEnabledMessage& data)
{
  // Find the entity by its ID
  ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

  // Check if the entity is valid before proceeding
  if (!IsValid(entity))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityCollisionDetectionEnabledMessage: FAILED: No Entity with ID=%d exists in the scene!"), data.EntityID.Value());
    return;
  }

  entity->CollisionEnabled = data.Enabled;
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetEntityCollisionDetectionEnabledMessage: Entity \"%s\" with id %d now has CollisionEnabled=%d"), *ObjName(entity), entity->EntityID, entity->CollisionEnabled);
}

bool CUnrealCigiEntityEventHandler::IsPointInEntityVolume(const sbio::math::GeocentricCoordinates& point, sbio::EntityID entityID) const
{
  return FUnrealCigi_PluginModule::globals.pPhysicsManager->IsPointInEntityVolume(point, entityID);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026