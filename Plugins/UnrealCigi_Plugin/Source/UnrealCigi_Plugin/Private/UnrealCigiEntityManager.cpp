//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiEntityManager.h"
#include "UnrealCigiEventHandler.h"
#include "CigiEntity.h"
#include "UnrealCigi_PluginPrivate.h"
#include "unrealcigiUtil.h"

namespace sbio
{
  namespace unrealcigi
  {
    CUnrealCigiEntityManager::CUnrealCigiEntityManager(UWorld* world) : World(world)
    {
    }

    void CUnrealCigiEntityManager::LoadBlueprints()
    {
      // Find all Blueprint classes that derive from ACigiEntity
      TArray<UClass*> bpClasses = utils::FindBlueprintAssets(ACigiEntity::StaticClass());
      int count = 0;

      // Iterate through the found Blueprint classes
      for (UClass* bpClass : bpClasses)
      {
        // Check if the bpClass is valid before proceeding
        if (bpClass == nullptr)
        {
          UE_LOG(LogCigiEventHandler, BP_WARNING, TEXT("LoadBP: UClass* reference was null"));
          continue;
        }

        TSubclassOf<ACigiEntity> bpEntityClass = TSubclassOf<ACigiEntity>(bpClass);

        // Check if the bpEntityClass is valid before proceeding
        if (bpEntityClass == nullptr)
        {
          UE_LOG(LogCigiEventHandler, BP_WARNING, TEXT("LoadBP: UClass* reference '%s' does not derive from ACigiEntity"), *bpClass->GetFName().ToString());
          continue;
        }

        ACigiEntity* bpEntity = bpEntityClass.GetDefaultObject();

        // Check if the default object is valid before proceeding
        if (bpEntity == nullptr)
        {
          UE_LOG(LogCigiEventHandler, BP_WARNING, TEXT("LoadBP: Failed to get default object for class '%s'"), *bpClass->GetFName().ToString());
          continue;
        }

        FString sEntityEnumeration = bpEntity->sisoID.ToString();

        // Check if the sEntityEnumeration is valid before adding it to the Configs map
        if (sEntityEnumeration == TEXT("0.0.0.0.0.0.0"))
        {
          UE_LOG(LogCigiEventHandler, BP_WARNING, TEXT("LoadBP: Skipped entity \"%s\" because sisoID \"%s\" was not set!"), *utils::ObjName(bpEntity), *sEntityEnumeration);
          continue;
        }

        // Check if the sEntityEnumeration is already in use before adding it to the Configs map  
        if (Configs.Contains(sEntityEnumeration))
        {
          UE_LOG(LogCigiEventHandler, BP_WARNING, TEXT("LoadBP: Skipped entity \"%s\" because sisoID \"%s\" is already loaded"), *utils::ObjName(bpEntity), *sEntityEnumeration);
          continue;
        }

        bool shortTypeSuccessful = false;
        int32 shortID = bpEntity->shortEntityTypeID;

        // Check if the shortID is already in use before adding it to the ShortTypeToExtended map
        if (!ShortTypeToExtended.Contains(shortID))
        {
          ShortTypeToExtended.Add(shortID, sEntityEnumeration);
          shortTypeSuccessful = true;
        }
        else
        {
          UE_LOG(LogCigiEventHandler, BP_WARNING, TEXT("LoadBP: Entity \"%s\" has shortID %d, which is already taken. Failed to save shortID."), *utils::ObjName(bpEntity), shortID);
        }

        // Create a new UEntityConfig object for this Blueprint entity
        UEntityConfig* ecPtr = NewObject<UEntityConfig>();
        ecPtr->AddToRoot();
        ecPtr->InitBP(bpEntityClass);
        Configs.Emplace(sEntityEnumeration, ecPtr);
        count++;
        FString bpEntityClassName = bpEntityClass.Get() == nullptr ? FString("NULL") : (bpEntityClass.Get()->GetName().IsEmpty() ? FString("NoName") : bpEntityClass.Get()->GetName());

        if (shortTypeSuccessful)
        {
          UE_LOG(LogCigiEventHandler, BP_LOG, TEXT("LoadBP: saved class '%s' with sisoID '%s', shortID '%d', and ConfigInfo '%s'"), *bpEntityClassName, *sEntityEnumeration, shortID, ecPtr == nullptr ? TEXT("NULL") : *ecPtr->ToString());
        }
        else
        {
          UE_LOG(LogCigiEventHandler, BP_LOG, TEXT("LoadBP: saved class '%s' with sisoID '%s', shortID 'NONE', and ConfigInfo '%s'"), *bpEntityClassName, *sEntityEnumeration, ecPtr == nullptr ? TEXT("NULL") : *ecPtr->ToString());
        }
      }
      UE_LOG(LogCigiEventHandler, BP_LOG, TEXT("LoadBP: Finished: Found %d entity BPs"), count);
    }

    bool CUnrealCigiEntityManager::Attach(sbio::EntityID entityID, sbio::EntityID parentID)
    {
      // Find the entity and parent using their IDs
      ACigiEntity* entity = Find(entityID);
      ACigiEntity* parent = Find(parentID);

      // Check if both the entity and parent are valid before proceeding
      if (!IsValid(entity) || !IsValid(parent))
      {
        return false;
      }

      entity->AttachToActor(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
      return true;
    }

    bool CUnrealCigiEntityManager::SetActive(sbio::EntityID entityID, bool active)
    {
      ACigiEntity* entity = Find(entityID);

      // Check if the entity is valid before proceeding
      if (!IsValid(entity))
      {
        return false;
      }

      entity->SetEnabled(active);
      return true;
    }

    bool CUnrealCigiEntityManager::Detach(sbio::EntityID entityID)
    {
      ACigiEntity* entity = Find(entityID);

      // Check if the entity is valid before proceeding
      if (!IsValid(entity))
      {
        return false;
      }

      entity->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      return true;
    }

    bool CUnrealCigiEntityManager::SetAlpha(sbio::EntityID entityID, float alpha)
    {
      ACigiEntity* entity = Find(entityID);

      // Check if the entity is valid before proceeding
      if (!IsValid(entity))
      {
        return false;
      }

      entity->EntityAlpha = alpha;
      return true;
    }

    bool CUnrealCigiEntityManager::SetCollisionEnabled(sbio::EntityID entityID, bool enabled)
    {
      ACigiEntity* entity = Find(entityID);

      // Check if the entity is valid before proceeding
      if (!IsValid(entity))
      {
        return false;
      }

      entity->CollisionEnabled = enabled;
      return true;
    }

    void CUnrealCigiEntityManager::SetWorld(UWorld* world)
    {
      World = world;
    }

    void CUnrealCigiEntityManager::Reset()
    {
      Actors.Empty();
      Configs.Empty();
      ShortTypeToExtended.Empty();
    }

    void CUnrealCigiEntityManager::ReleaseConfigs()
    {
      // Remove all configs from root and clear the map
      for (TPair<FString, UEntityConfig*>& pair : Configs)
      {
        if (IsValid(pair.Value))
        {
          pair.Value->RemoveFromRoot();
        }
      }
      Configs.Empty();
    }

    // Find an entity by its ID. Returns nullptr if not found.
    ACigiEntity* CUnrealCigiEntityManager::Find(sbio::EntityID entityID) const
    {
      ACigiEntity* const* entity = Actors.Find(entityID.Value());
      if (entity == nullptr)
      {
        return nullptr;
      }
      return *entity;
    }

    // Find the ID of an entity by its pointer. Returns UnknownEntityID if not found.
    sbio::EntityID CUnrealCigiEntityManager::FindID(const ACigiEntity* entity) const
    {
      const uint32* id = Actors.FindKey(const_cast<ACigiEntity*>(entity));
      if (id == nullptr)
      {
        return sbio::UnknownEntityID;
      }
      return sbio::EntityID(*id);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026