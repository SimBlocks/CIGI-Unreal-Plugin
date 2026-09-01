//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"

class ACigiView;

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiViewEventHandler
     * @brief Handles CIGI view messages and updates the corresponding view actors.
     */
    class CUnrealCigiViewEventHandler
    {
    public:
      /**
       * @brief Constructor for CUnrealCigiViewEventHandler.
       * @param eventHandler - Reference to the event handler instance.
       */
      explicit CUnrealCigiViewEventHandler(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Gets the player ID from the view ID.
       * @param viewID - The ID of the view.
       * @return The player ID associated with the view ID.
       */
      int32 GetPlayerIDFromViewID(ViewID viewID) const;

      /**
       * @brief Gets the player controller from the view ID.
       * @param viewID - The ID of the view.
       * @return Pointer to the player controller associated with the view ID.
       */
      APlayerController* GetPlayerControllerFromViewID(ViewID viewID) const;

      /**
       * @brief Gets the first view.
       * @return Pointer to the first CigiView instance.
       */
      ACigiView* GetFirstView();

      /**
       * @brief Removes a view based on the view ID.
       * @param viewID - The ID of the view to be removed.
       * @return True if the view was successfully removed, false otherwise.
       */
      bool RemoveView(ViewID viewID);

      /**
       * @brief Sets up the view actor for the given view ID.
       * @param viewID - The ID of the view.
       */
      void SetupViewActor(ViewID viewID);

      /**
       * @brief Gets the view from the given view ID.
       * @param viewID - The ID of the view.
       * @return Pointer to the CigiView instance associated with the view ID.
       */
      ACigiView* GetViewFromID(ViewID viewID);

      /**
       * @brief Handles the update of the attached camera transform.
       * @param data - The message data containing the new camera transform.
       */
      void OnUpdateAttachedCameraTransformMessage(const sbio::ig::view::SUpdateAttachedCameraTransformMessage& data);

      /**
       * @brief Handles the message to attach the camera to an entity.
       * @param data - The message data containing the attachment information.
       */
      void OnSetCameraAttachedToEntityMessage(const sbio::ig::view::SSetCameraAttachedToEntityMessage& data);

      /**
       * @brief Handles the message to unattach the camera.
       * @param data - The message data for the unattachment.
       */
      void OnSetCameraUnattachedMessage(const sbio::ig::view::SSetCameraUnattachedMessage& data);

      /**
       * @brief Handles the message to set the camera projection.
       * @param data - The message data containing the projection settings.
       */
      void OnSetCameraProjectionMessage(const sbio::ig::view::SSetCameraProjectionMessage& data);

      /**
       * @brief Handles the message to set the view component state.
       * @param data - The message data containing the view component state.
       */
      void OnSetViewComponentStateMessage(const sbio::ig::view::SSetViewComponentStateMessage& data);

      /**
       * @brief Handles the message to set the view group component state.
       * @param data - The message data containing the view group component state.
       */
      void OnSetViewGroupComponentStateMessage(const sbio::ig::view::SSetViewGroupComponentStateMessage& data);

    private:
      CUnrealCigiEventHandler& EventHandler; ///< Reference to the event handler instance.
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026