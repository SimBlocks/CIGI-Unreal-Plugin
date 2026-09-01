//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiPhysicsEventHandler.h"

#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiPhysicsManager.h"
#include "CoreMinimal.h"

using namespace sbio;
using namespace sbio::unrealcigi;

CUnrealCigiPhysicsEventHandler::CUnrealCigiPhysicsEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
{
}

void CUnrealCigiPhysicsEventHandler::OnCreateCollisionDetectionSegmentMessage(const sbio::ig::physics::SCreateCollisionDetectionSegmentMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->CreateSegment(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateCollisionDetectionSegmentMessage: Failed for entity=%d segment=%d"), data.EntityID.Value(), data.SegmentID.Value());
  }
}

void CUnrealCigiPhysicsEventHandler::OnSetCollisionDetectionSegmentMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->UpdateSegment(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCollisionDetectionSegmentMessage: Failed for entity=%d segment=%d"), data.EntityID.Value(), data.SegmentID.Value());
  }
}

void CUnrealCigiPhysicsEventHandler::OnSetCollisionDetectionSegmentEnabledMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentEnabledMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->SetSegmentEnabled(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCollisionDetectionSegmentEnabledMessage: FAILED: Entity %d does not have a collision segment with id=%d! Create it first."), data.EntityID.Value(), data.SegmentID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCollisionDetectionSegmentEnabledMessage: Updated segment %d in entity %d: enabled=%d"), data.SegmentID.Value(), data.EntityID.Value(), data.Enabled);
}

void CUnrealCigiPhysicsEventHandler::OnSetCollisionVolumeMessage(const sbio::ig::physics::SSetCollisionVolumeMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->UpdateVolume(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCollisionVolumeMessage: FAILED: Entity %d does not have a collision volume with id=%d! Create it first."), data.EntityID.Value(), data.VolumeID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCollisionVolumeMessage: Updated collision volume %d within entity %d"), data.VolumeID.Value(), data.EntityID.Value());
}

void CUnrealCigiPhysicsEventHandler::OnCreateCollisionVolumeSphereMessage(const sbio::ig::physics::SCreateCollisionVolumeSphereMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->CreateVolumeSphere(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateCollisionVolumeSphereMessage: FAILED: Entity %d already has a collision volume with id=%d!"), data.EntityID.Value(), data.VolumeID.Value());
    return;
  }
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateCollisionVolumeSphereMessage: Created collision sphere %d within entity %d"), data.VolumeID.Value(), data.EntityID.Value());
}

void CUnrealCigiPhysicsEventHandler::OnCreateCollisionVolumeCuboidMessage(const sbio::ig::physics::SCreateCollisionVolumeCuboidMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->CreateVolumeCuboid(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateCollisionVolumeCuboidMessage: FAILED: Entity %d already has a collision volume with id=%d!"), data.EntityID.Value(), data.VolumeID.Value());
    return;
  }
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateCollisionVolumeCuboidMessage: Created collision cuboid %d within entity %d"), data.VolumeID.Value(), data.EntityID.Value());
}

void CUnrealCigiPhysicsEventHandler::OnSetCollisionVolumeEnabledMessage(const sbio::ig::physics::SSetCollisionVolumeEnabledMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->SetVolumeEnabled(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCollisionVolumeEnabledMessage: FAILED: Entity %d does not have a collision volume with id=%d!"), data.EntityID.Value(), data.VolumeID.Value());
    return;
  }
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCollisionVolumeEnabledMessage: %s collision volume %d within entity %d"), data.Enabled ? TEXT("Enabled") : TEXT("Disabled"), data.VolumeID.Value(), data.EntityID.Value());
}

void CUnrealCigiPhysicsEventHandler::OnDestroyCollisionVolumeMessage(const sbio::ig::physics::SDestroyCollisionVolumeMessage& data)
{
  if (!FUnrealCigi_PluginModule::globals.pPhysicsManager->DestroyVolume(data))
  {
    UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnDestroyCollisionVolumeMessage: Success? Entity %d did not have a collision volume with id=%d"), data.EntityID.Value(), data.VolumeID.Value());
    return;
  }
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnDestroyCollisionVolumeMessage: Destroyed collision volume %d within entity %d"), data.VolumeID.Value(), data.EntityID.Value());
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026