//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"

class ACigiView;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiViewManager
     * @brief Tracks CIGI view actors and their associated player-controller indices.
     */
    class CUnrealCigiViewManager
    {
    public:
      /**
       * @brief Construct a new CUnrealCigiViewManager object.
       *
       * @param world A pointer to the UWorld context. Default is nullptr.
       */
      explicit CUnrealCigiViewManager(UWorld* world = nullptr);

      /**
       * @brief Set the world context for the manager.
       *
       * @param world A pointer to the UWorld to be set.
       */
      void SetWorld(UWorld* world);

      /**
       * @brief Reset the view manager, clearing all tracked views.
       */
      void Reset();

      /**
       * @brief Find a view by its ID.
       *
       * @param viewID The ID of the view to find.
       * @return ACigiView* A pointer to the found ACigiView, or nullptr if not found.
       */
      ACigiView* Find(int32 viewID) const;

      /**
       * @brief Add a new view with the associated view ID.
       *
       * @param viewID The ID to associate with the view.
       * @param view A pointer to the ACigiView to be added.
       */
      void Add(int32 viewID, ACigiView* view);

      /**
       * @brief Remove a view by its ID.
       *
       * @param viewID The ID of the view to remove.
       */
      void Remove(int32 viewID);

      /**
       * @brief Destroy a view and remove it from tracking.
       *
       * @param viewID The ID of the view to destroy.
       */
      void Destroy(int32 viewID);

      /**
       * @brief Get the first view in the manager.
       *
       * @return ACigiView* A pointer to the first ACigiView, or nullptr if none exist.
       */
      ACigiView* First() const;

      /**
       * @brief Get the player controller index associated with a view ID.
       *
       * @param viewID The ID of the view.
       * @return int32 The player controller index associated with the view ID.
       */
      int32 PlayerControllerIndex(int32 viewID) const;

      UWorld* World = nullptr; /**< Pointer to the world context. */
      TMap<int32, ACigiView*> Actors; /**< Map of view IDs to ACigiView pointers. */
      TArray<int32> PlayerControllerViewIDs; /**< Array of player controller indices associated with view IDs. */
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026