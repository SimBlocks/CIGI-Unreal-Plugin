//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiViewEventHandler.h"
#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiViewManager.h"
#include "UnrealCigiComponentDispatcher.h"
#include "CigiController.h"
#include "UnrealCigiEntityManager.h"
#include "Kismet/GameplayStatics.h"
#include "CigiView.h"
#include "ViewLib/ViewManager.h"
#include "IGCigiLib/CigiView.h"
#include "IGCigiLib/CigiViewGroup.h"
#include "CigiView.h"
#include "CigiEntity.h"
#include "Camera/CameraComponent.h"
#include "unrealcigiUtil.h"
#include "unrealcigiPlayerController.h"

namespace sbio
{
  namespace unrealcigi
  {
    using namespace utils;

    CUnrealCigiViewEventHandler::CUnrealCigiViewEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
    {
    }

    void CUnrealCigiViewEventHandler::OnSetViewComponentStateMessage(const sbio::ig::view::SSetViewComponentStateMessage& data)
    {
      if (data.ComponentID.Value() == 0)
      {
        APlayerController* pc = GetPlayerControllerFromViewID(data.ViewID);
        if (data.ComponentState == 0)
        {
          if (!IsValid(pc))
          {
            SetupViewActor(data.ViewID);
          }

          return;
        }
        if (data.ComponentState == 1)
        {
          // Remove the player controller associated with the view ID and destroy the view
          if (IsValid(pc) && GetPlayerIDFromViewID(data.ViewID) != 0)
          {
            UGameplayStatics::RemovePlayer(pc, true);
            FUnrealCigi_PluginModule::globals.pUnrealViewManager->Destroy(data.ViewID.Value());
            RemoveView(data.ViewID);
          }
          return;
        }
        if (data.ComponentState == 2)
        {
          // Detach the view from its view group and remove it from the view group
          sbio::cigi::ig::CCigiView* view = static_cast<sbio::cigi::ig::CCigiView*>(FUnrealCigi_PluginModule::globals.pViewManager->GetView(data.ViewID));
          if (view != nullptr)
          {
            sbio::view::CViewGroup* group = FUnrealCigi_PluginModule::globals.pViewManager->GetViewGroup(view->GetViewGroupID());
            if (group != nullptr)
            {
              view->SetViewGroupID(UnknownViewGroupID);
              group->RemoveViewID(view->GetViewID());
            }
          }
          return;
        }
      }

      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::VIEW, data.ComponentID.Value(), data.ComponentState, data.ViewID.Value(), data.ComponentData);
    }

    void CUnrealCigiViewEventHandler::OnSetViewGroupComponentStateMessage(const sbio::ig::view::SSetViewGroupComponentStateMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::VIEW_GROUP, data.ComponentID.Value(), data.ComponentState, data.ViewGroupID.Value(), data.ComponentData);
    }

    ACigiView* CUnrealCigiViewEventHandler::GetViewFromID(ViewID viewID)
    {
      // Get the view from the Unreal View Manager using the provided view ID
      ACigiView* view = FUnrealCigi_PluginModule::globals.pUnrealViewManager->Find(viewID.Value());

      // If the view is not valid, remove it from the Unreal View Manager and return nullptr
      if (!IsValid(view))
      {
        return nullptr;
      }

      return view;
    }

    ACigiView* CUnrealCigiViewEventHandler::GetFirstView()
    {
      ACigiView* view = nullptr;
      int32 lowestViewID = TNumericLimits<int>::Max();
      TArray<int32> invalidViewIds;

      // Iterate through the Unreal View Manager's actors to find the first valid view and collect invalid view IDs
      for (const TPair<int32, ACigiView*>& viewPair : FUnrealCigi_PluginModule::globals.pUnrealViewManager->Actors)
      {
        if (!IsValid(viewPair.Value))
        {
          invalidViewIds.Add(viewPair.Key);
          continue;
        }

        if (viewPair.Key < lowestViewID)
        {
          lowestViewID = viewPair.Key;
          view = viewPair.Value;
        }
      }

      // Remove any invalid view IDs from the Unreal View Manager
      for (int32 invalidViewId : invalidViewIds)
      {
        FUnrealCigi_PluginModule::globals.pUnrealViewManager->Remove(invalidViewId);
      }

      return view;
    }

    bool CUnrealCigiViewEventHandler::RemoveView(ViewID viewID)
    {
      // Get the view manager from the plugin globals
      sbio::view::CViewManager* viewManager = FUnrealCigi_PluginModule::globals.pViewManager.get();
      if (viewManager == nullptr)
      {
        return false;
      }

      // Get the view from the view manager using the provided view ID
      sbio::view::CView* baseView = viewManager->GetView(viewID);
      if (baseView == nullptr)
      {
        return false;
      }

      sbio::cigi::ig::CCigiView* view = dynamic_cast<sbio::cigi::ig::CCigiView*>(baseView);

      // If the view is valid, remove it from its view group and reset its attached entity ID
      if (view != nullptr)
      {
        if (sbio::view::CViewGroup* viewGroup = viewManager->GetViewGroup(view->GetViewGroupID()))
        {
          viewGroup->RemoveViewID(view->GetViewID());
        }
        view->SetAttachedEntityID(UnknownEntityID);
      }

      return viewManager->RemoveView(viewID);
    }

    void CUnrealCigiViewEventHandler::SetupViewActor(ViewID viewID)
    {
      if (FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerViewIDs.IsEmpty())
      {
        FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerViewIDs.Add(viewID.Value());
      }

      APlayerController* playerController = GetPlayerControllerFromViewID(viewID);
      if (IsValid(playerController))
      {
        AunrealcigiPlayerController* cigiPlayerController = Cast<AunrealcigiPlayerController>(playerController);
        if (!IsValid(cigiPlayerController))
        {
          UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("SetupViewActor: Player Controller found for ViewID=%d is not an AUnrealCigiPlayerController!"), viewID.Value());
          return;
        }
        cigiPlayerController->SetViewID(viewID);
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("SetupViewActor: Found existing Cigi Player Controller %d: \"%s\" from ViewID=%d"), UGameplayStatics::GetPlayerControllerID(playerController), *ObjName(playerController), viewID.Value());
      }
      else
      {
        playerController = UGameplayStatics::CreatePlayer(EventHandler.GetWorld(), FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerViewIDs.Num());
        if (!IsValid(playerController))
        {
          UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("SetupViewActor: Failed to create Player Controller %d!"), viewID.Value());
          return;
        }

        AunrealcigiPlayerController* cigiPlayerController = Cast<AunrealcigiPlayerController>(playerController);
        if (!IsValid(cigiPlayerController))
        {
          UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("SetupViewActor: Player Controller created for ViewID=%d is not an AUnrealCigiPlayerController!"), viewID.Value());
          return;
        }

        FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerViewIDs.Add(viewID.Value());
        cigiPlayerController->SetViewID(viewID);
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("SetupViewActor: Created new Player Controller %d: \"%s\" from ViewID=%d"), UGameplayStatics::GetPlayerControllerID(playerController), *ObjName(playerController), viewID.Value());
      }

      ACigiView* viewActor = FUnrealCigi_PluginModule::globals.pUnrealViewManager->Find(viewID.Value());
      ACigiView* currentPawn = playerController->GetPawn<ACigiView>();

      // If the view actor already exists, possess it and set it as the view target
      if (IsValid(viewActor))
      {
        playerController->Possess(viewActor);
        playerController->SetViewTargetWithBlend(viewActor, 0);
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("SetupViewActor: PC %d is setup with ViewActor \"%s\""), viewID.Value(), *ObjName(viewActor));
      }
      else if (IsValid(currentPawn))
      {
        // If the player controller already has a pawn, use that as the view actor
        FUnrealCigi_PluginModule::globals.pUnrealViewManager->Add(viewID.Value(), currentPawn);
        currentPawn->ViewID = viewID.Value();
        playerController->Possess(currentPawn);
        playerController->SetViewTargetWithBlend(currentPawn, 0);
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("SetupViewActor: Found current CigiView pawn \"%s\", setting this as ViewActor %d"), *ObjName(currentPawn), viewID.Value());
      }
      else
      {
        ACigiView* newPawn = EventHandler.GetWorld()->SpawnActor<ACigiView>(ACigiView::StaticClass(), FVector(0, 0, 0), FRotator(0, 0, 0));

        // Check if the new pawn was successfully spawned
        if (!IsValid(newPawn))
        {
          UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("SetupViewActor: Failed to spawn CigiView pawn for ViewActor %d"), viewID.Value());
          return;
        }

        // Add the new pawn to the UnrealViewManager and set its ViewID
        FUnrealCigi_PluginModule::globals.pUnrealViewManager->Add(viewID.Value(), newPawn);
        newPawn->ViewID = viewID.Value();
        playerController->Possess(newPawn);
        playerController->SetViewTargetWithBlend(newPawn, 0);
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("SetupViewActor: Created a new CigiView pawn \"%s\", setting this as ViewActor %d"), *ObjName(newPawn), viewID.Value());
      }

      // Set the default camera projection for the view 
      sbio::ig::view::SSetCameraProjectionMessage projection;
      projection.ViewID = viewID;
      projection.ProjectionMode = DEFAULT_VIEWPROJ_PROJMODE;
      projection.MirrorMode = DEFAULT_VIEWPROJ_MIRROR;
      projection.Near = DEFAULT_VIEWPROJ_NEAR;
      projection.Far = DEFAULT_VIEWPROJ_FAR;
      projection.LeftHalfAngle = -DEFAULT_VIEWPROJ_LR;
      projection.RightHalfAngle = DEFAULT_VIEWPROJ_LR;
      projection.BottomHalfAngle = -DEFAULT_VIEWPROJ_TB;
      projection.TopHalfAngle = DEFAULT_VIEWPROJ_TB;
      OnSetCameraProjectionMessage(projection);

      // Ensure that the view is registered with the ViewManager
      if (!FUnrealCigi_PluginModule::globals.pViewManager->HasView(viewID))
      {
        FUnrealCigi_PluginModule::globals.pViewManager->AddView(std::make_unique<sbio::cigi::ig::CCigiView>(viewID));
      }

      // If this is not the first view, ensure that the first view is set up as well
      if (!FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerViewIDs.IsEmpty())
      {
        const int32 playerControllerZeroViewID = FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerViewIDs[0];
        if (viewID.Value() != playerControllerZeroViewID)
        {
          SetupViewActor(ViewID(playerControllerZeroViewID));
        }
      }
    }

    int32 CUnrealCigiViewEventHandler::GetPlayerIDFromViewID(ViewID viewID) const
    {
      return FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerIndex(viewID.Value());
    }

    APlayerController* CUnrealCigiViewEventHandler::GetPlayerControllerFromViewID(ViewID viewID) const
    {
      const int32 playerControllerIndex = GetPlayerIDFromViewID(viewID);

      // If the player controller index is invalid, return nullptr
      if (playerControllerIndex < 0)
      {
        return nullptr;
      }

      return UGameplayStatics::GetPlayerController(EventHandler.GetWorld(), playerControllerIndex);
    }

    void CUnrealCigiViewEventHandler::OnUpdateAttachedCameraTransformMessage(const sbio::ig::view::SUpdateAttachedCameraTransformMessage& data)
    {
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateAttachedCameraTransformMessage: CameraID=%d"), data.ViewID.Value());
      ACigiView* view = GetViewFromID(data.ViewID);

      // Validate the view actor before proceeding
      if (!IsValid(view))
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateAttachedCameraTransformMessage: FAILED: No ViewActor with ViewID=%d exists in the scene!"), data.ViewID.Value());
        return;
      }

      // Validate that the view actor has a parent before proceeding
      if (!IsValid(view->GetAttachParentActor()))
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateAttachedCameraTransformMessage: FAILED: Target ViewActor \"%s\" does not have a parent!"), *utils::ObjName(view));
        return;
      }

      FTransform newCameraTransform = utils::BodyTransformToFTransform(data.Offset, data.Rotation);
      view->GetRootComponent()->SetRelativeTransform(newCameraTransform);

      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateAttachedCameraTransformMessage: Camera=\"%s\", Parent=\"%s\""), *utils::ObjName(view), *view->GetAttachParentActor()->GetName());
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateAttachedCameraTransformMessage: RELATIVE pos=\"%s\", rot=\"%s\""), *view->GetRootComponent()->GetRelativeLocation().ToString(), *view->GetRootComponent()->GetRelativeRotation().Euler().ToString());
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateAttachedCameraTransformMessage: WORLD pos=\"%s\", rot=\"%s\""), *view->GetActorLocation().ToString(), *view->GetActorRotation().Euler().ToString());
    }

    void CUnrealCigiViewEventHandler::OnSetCameraAttachedToEntityMessage(const sbio::ig::view::SSetCameraAttachedToEntityMessage& data)
    {
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCameraAttachedToEntityMessage: CameraID=%d"), data.ViewID.Value());
      ACigiView* view = GetViewFromID(data.ViewID);

      // Validate the view actor before proceeding
      if (!IsValid(view))
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraAttachedToEntityMessage: FAILED: No ViewActor with ViewID=%d exists in the scene!"), data.ViewID.Value());
        return;
      }

      ACigiEntity* parent = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(data.EntityID);

      // Validate the parent entity before proceeding
      if (!IsValid(parent))
      {
        if (data.EntityID.Value() == USHORT_MAX)
        {
          UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCameraAttachedToEntityMessage: EntityID=%d indicating attached entity was destroyed. Ignoring message."), data.EntityID.Value());
        }
        else
        {
          UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraAttachedToEntityMessage: FAILED: No Entity with EntityID=%d exists in the scene!"), data.EntityID.Value());
        }
        return;
      }

      view->AttachToActor(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCameraAttachedToEntityMessage: attached ViewActor \"%s\" to entity \"%s\""), *utils::ObjName(view), *utils::ObjName(parent));
    }

    void CUnrealCigiViewEventHandler::OnSetCameraUnattachedMessage(const sbio::ig::view::SSetCameraUnattachedMessage& data)
    {
      ACigiView* view = GetViewFromID(data.ViewID);

      // Validate the view actor before proceeding
      if (!IsValid(view))
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEntityUnattachedMessage: FAILED: No ViewActor with ViewID=%d exists in the scene!"), data.ViewID.Value());
        return;
      }

      view->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetEntityUnattachedMessage: ViewActor \"%s\" is now unattached! (parent=null)"), *utils::ObjName(view));
    }

    void CUnrealCigiViewEventHandler::OnSetCameraProjectionMessage(const sbio::ig::view::SSetCameraProjectionMessage& data)
    {
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetCameraProjectionMessage: CameraID=%d"), data.ViewID.Value());
      ACigiView* view = GetViewFromID(data.ViewID);

      // Validate the view actor before proceeding
      if (!IsValid(view))
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: FAILED: No ViewActor with ViewID=%d exists in the scene!"), data.ViewID.Value());
        return;
      }

      UCameraComponent* camera = view->CameraComp;

      // Validate the camera component before proceeding
      if (!IsValid(camera))
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: FAILED: ViewActor \"%s\" has a null camera component!"), *ObjName(view));
        return;
      }

      if (data.ProjectionMode == sbio::EProjectionMode::PERSPECTIVE)
      {
        camera->SetProjectionMode(ECameraProjectionMode::Perspective);
      }

      if (data.ProjectionMode == sbio::EProjectionMode::ORTHOGRAPHIC)
      {
        camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
      }

      if (data.Near < 0)
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: Near plane %f must be >=0!"), data.Near);
      }
      else
      {
        camera->SetOrthoNearClipPlane(data.Near);
      }
      if (data.Far < 0 || data.Far < data.Near)
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: Far plane %f must be >=0 and >=%f (Near plane)!"), data.Far, data.Near);
      }
      else
      {
        camera->SetOrthoFarClipPlane(data.Far);
      }

      const bool disableAspectRatio = data.BottomHalfAngle == 0 && data.TopHalfAngle == 0;
      if (disableAspectRatio)
      {
        camera->SetConstraintAspectRatio(false);
      }
      if (-data.LeftHalfAngle != data.RightHalfAngle)
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: -1*LeftHalfAngle %f must equal RightHalfAngle %f!"), -data.LeftHalfAngle, data.RightHalfAngle);
      }
      else if (data.RightHalfAngle < 0.1 || data.RightHalfAngle >= 90)
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: RightHalfAngle %f must be in range (0.1, 90)!"), data.RightHalfAngle);
      }
      else
      {
        camera->SetFieldOfView(data.RightHalfAngle * 2.0f);
        if (!disableAspectRatio && -data.BottomHalfAngle != data.TopHalfAngle)
        {
          UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: -1*BottomHalfAngle %f must equal TopHalfAngle %f!"), -data.BottomHalfAngle, data.TopHalfAngle);
        }
        else if (!disableAspectRatio && (data.TopHalfAngle < 0.1 || data.TopHalfAngle >= 90))
        {
          UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetCameraProjectionMessage: TopHalfAngle %f must be in range (0.1, 90)!"), data.TopHalfAngle);
        }
        else if (!disableAspectRatio)
        {
          camera->SetConstraintAspectRatio(true);
          camera->SetAspectRatio(FMath::Tan(FMath::DegreesToRadians(data.RightHalfAngle)) / FMath::Tan(FMath::DegreesToRadians(data.TopHalfAngle)));
        }
      }
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026