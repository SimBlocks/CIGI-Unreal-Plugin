//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiViewManager.h"
#include "CigiView.h"

namespace sbio
{
  namespace unrealcigi
  {
    CUnrealCigiViewManager::CUnrealCigiViewManager(UWorld* world) : World(world)
    {
    }

    void CUnrealCigiViewManager::SetWorld(UWorld* world)
    {
      World = world;
    }

    void CUnrealCigiViewManager::Reset()
    {
      Actors.Empty();
      PlayerControllerViewIDs.Empty();
    }

    ACigiView* CUnrealCigiViewManager::Find(int32 viewID) const
    {
      ACigiView* const* view = Actors.Find(viewID);

      // Check if the view pointer is null before dereferencing it
      if (view == nullptr)
      {
        return nullptr;
      }

      return *view;
    }

    void CUnrealCigiViewManager::Add(int32 viewID, ACigiView* view)
    {
      if (view != nullptr)
      {
        Actors.Add(viewID, view);
      }
    }

    void CUnrealCigiViewManager::Remove(int32 viewID)
    {
      Actors.Remove(viewID);
    }

    void CUnrealCigiViewManager::Destroy(int32 viewID)
    {
      // Find the index of the player controller associated with the view ID
      const int32 playerControllerIndex = PlayerControllerIndex(viewID);

      // If the view is associated with a player controller, reset the corresponding index in PlayerControllerViewIDs
      if (playerControllerIndex >= 0 && playerControllerIndex < PlayerControllerViewIDs.Num())
      {
        PlayerControllerViewIDs[playerControllerIndex] = -999;
      }

      // Remove the view from the Actors map
      Remove(viewID);
    }

    ACigiView* CUnrealCigiViewManager::First() const
    {
      int32 lowestViewID = TNumericLimits<int32>::Max();
      ACigiView* firstView = nullptr;

      // Iterate through the Actors map to find the view with the lowest ID
      for (const TPair<int32, ACigiView*>& pair : Actors)
      {
        if (pair.Key < lowestViewID && IsValid(pair.Value))
        {
          lowestViewID = pair.Key;
          firstView = pair.Value;
        }
      }

      return firstView;
    }

    int32 CUnrealCigiViewManager::PlayerControllerIndex(int32 viewID) const
    {
      return PlayerControllerViewIDs.Find(viewID);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026