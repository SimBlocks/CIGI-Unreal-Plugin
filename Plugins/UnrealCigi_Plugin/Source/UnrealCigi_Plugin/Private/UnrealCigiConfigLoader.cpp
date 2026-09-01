//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiConfigLoader.h"
#include "UnrealCigiEventHandler.h"
#include "UnrealCigiViewEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "EntityConfig.h"
#include "SymbolConfig.h"
#include "UnrealCigiUtil.h"
#include "Engine/Texture2D.h"
#include "IGCigiLib/CigiViewGroup.h"
#include "ViewLib/View.h"
#include "ViewLib/ViewManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace sbio
{
  namespace unrealcigi
  {
    void CUnrealCigiConfigLoader::LoadConfig(CUnrealCigiViewEventHandler& viewEventHandler)
    {
      FString filePath = FString();
      TSharedPtr<FJsonObject> ConfigObject = CUnrealCigiConfigLoader::LoadJsonConfig(filePath);
      if (!ConfigObject.IsValid())
      {
        return;
      }

      // --- Attempt to read "viewID" parameter ---
      int32 singleViewID = -1;
      const TArray<TSharedPtr<FJsonValue>>* p_viewIdValArray = nullptr;
      bool foundValidViewId = false;

      // Check if viewID is a single integer
      if (ConfigObject->TryGetNumberField(TEXT("viewID"), singleViewID) && singleViewID >= 0)
      {
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Found single integer \"viewID\"=%d. Creating view actor."), singleViewID);
        UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Note: You can make \"viewID\" into an array of integers >=0 to create multiple views."));
        viewEventHandler.SetupViewActor(ViewID(singleViewID));
        foundValidViewId = true;
      }
      // Check if viewID is an array of integers
      else if (ConfigObject->TryGetArrayField(TEXT("viewID"), p_viewIdValArray) && p_viewIdValArray != nullptr)
      {
        // If viewID is an array, loop through each element
        for (TSharedPtr<FJsonValue> viewIdVal : (*p_viewIdValArray))
        {
          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Found \"viewID\" array."));

          // The array element must exist
          if (!viewIdVal.IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: viewID array: Skipping null element. Must be an integer >= 0."));
            continue;
          }

          int viewId = -1;

          // The array element must be an integer
          if (!viewIdVal->TryGetNumber(viewId))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: viewID array: Skipping invalid element. Must be an integer >= 0."));
            continue;
          }

          // The integer must be >=0 (not negative)
          if (viewId < 0)
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: viewID array: Skipping invalid ID %d. Must be >= 0."), viewId);
            continue;
          }
          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: viewID array: Found valid ID %d. Creating view actor."), viewId);

          // For each valid viewID that was found in the array, create a view actor with that viewID
          viewEventHandler.SetupViewActor(ViewID(viewId));
          // Record if we have created at least one view actor
          foundValidViewId = true;
        }
      }
      // If viewID is neither or if it does not exist, default to using viewID=0. There must always be at least one view.
      else
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: \"viewID\" parameter was not defined. Must be an integer >= 0, or an array of integers >= 0."));
      }

      // There must be at least one view. If no views were initialized, create a default ACigiView with viewID=0
      if (!foundValidViewId)
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: No valid viewID was found. Creating a single default view with viewID=0."));
        viewEventHandler.SetupViewActor(ViewID(0));
      }

      // --- Attempt to read "viewGroups" array ---
      const TArray<TSharedPtr<FJsonValue>>* p_vgValArray;
      if (ConfigObject->TryGetArrayField(TEXT("viewGroups"), p_vgValArray))
      {
        for (TSharedPtr<FJsonValue> vgVal : (*p_vgValArray))
        {
          // Get the current vgObject: (curr item in "viewGroups" list)
          const TSharedPtr<FJsonObject>* p_vgObject;
          if (!vgVal->TryGetObject(p_vgObject))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped view group b/c dbVal->TryGetObject failed!"));
            continue;
          }
          TSharedPtr<FJsonObject> vgObject = *p_vgObject;
          if (!vgObject.IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped view group b/c dbObject is not valid!"));
            continue;
          }

          // Find the view group with the given groupID (fail if no ID was given)
          int32 groupID = -1;
          if (!vgObject->TryGetNumberField(TEXT("groupID"), groupID) || groupID <= 0)
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped view group b/c \"groupID\" parameter is not valid! (must be > 0)"));
            continue;
          }
          sbio::view::CViewGroup* pViewGroup = FUnrealCigi_PluginModule::globals.pViewManager->GetViewGroup(ViewGroupID(groupID));
          // If it doesn't already exist, create a new view group for this ID
          if (!pViewGroup)
          {
            FUnrealCigi_PluginModule::globals.pViewManager->AddViewGroup(std::make_unique<sbio::cigi::ig::CCigiViewGroup>(ViewGroupID(groupID)));
            pViewGroup = FUnrealCigi_PluginModule::globals.pViewManager->GetViewGroup(ViewGroupID(groupID));
          }

          // If any views were specified, add them to the group
          FString dbgViews = "";
          const TArray<TSharedPtr<FJsonValue>>* p_viewValArray;
          if (vgObject->TryGetArrayField(TEXT("views"), p_viewValArray) && (*p_viewValArray).Num() > 0)
          {
            for (TSharedPtr<FJsonValue> viewVal : (*p_viewValArray))
            {
              // Get the current view as a number
              int32 viewID = -1;
              if (!viewVal->TryGetNumber(viewID) || viewID < 0)
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: view group %d: skipping \"views\" element b/c the ViewID is not valid! (must be >= 0)"), groupID);
                continue;
              }

              // Check if the specified ViewID actually exists
              sbio::view::CView* pView = FUnrealCigi_PluginModule::globals.pViewManager->GetView(ViewID(viewID));
              if (!pView)
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: view group %d: skipping \"views\" element b/c ViewID %d does not exist!"), groupID, viewID);
                continue;
              }

              // Add the view to the group
              pViewGroup->AddViewID(ViewID(viewID));
              dbgViews = FString::Printf(TEXT("%s %d"), *dbgViews, viewID);
            }
          }

          // If a center ViewID was specified and the corresponding view is a member of the group, set that view as the new center
          int32 centerViewID = -1;
          if (vgObject->TryGetNumberField(TEXT("centerViewID"), centerViewID) && centerViewID >= 0 && pViewGroup->GetViewIDs().find(ViewID(centerViewID)) != pViewGroup->GetViewIDs().end())
          {
            pViewGroup->SetCenterViewID(ViewID(centerViewID));
          }
          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Initialized view group with groupID=%d, views=[%s ]%s"), groupID, *dbgViews, pViewGroup->GetCenterViewID() == UnknownViewID ? TEXT("") : *FString::Printf(TEXT(", centerViewID = %d"), pViewGroup->GetCenterViewID().Value()));
        }
      }

      // --- Attempt to read "databases" array ---
      const TArray<TSharedPtr<FJsonValue>>* p_dbValArray;
      if (!ConfigObject->TryGetArrayField(TEXT("databases"), p_dbValArray))
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: json file \"%s\" does not have a \"databases\" array!"), *filePath);
      }
      else
      {
        // Iterate over every database in the "databases" array, and attempt to load their IDs and file paths:
        for (TSharedPtr<FJsonValue> dbVal : (*p_dbValArray))
        {
          // Get the current dbObject: (curr item in "databases" list)
          const TSharedPtr<FJsonObject>* p_dbObject;
          if (!dbVal->TryGetObject(p_dbObject))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped database b/c dbVal->TryGetObject failed"));
            continue;
          }
          TSharedPtr<FJsonObject> dbObject = *p_dbObject;
          if (!dbObject.IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped database b/c dbObject is not valid"));
            continue;
          }

          // Read required properties:
          FString filepath;
          if (!dbObject->TryGetStringField(TEXT("filepath"), filepath))
          {
            // Intentionally empty file paths are allowed, but there MUST be a filepath property (it cannot be missing entirely)
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped database b/c filepath is not valid"));
            continue;
          }
          int32 id;
          if (!dbObject->TryGetNumberField(TEXT("databaseID"), id))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: skipped database \"%s\" b/c databaseID is not valid!"), *filepath);
            continue;
          }
          if (FUnrealCigi_PluginModule::globals.pDatabaseManager->Databases.Find(id) != nullptr)
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: skipped database \"%s\" b/c id %d is already taken"), *filepath, id);
            continue;
          }

          // Allow an empty filepath so CIGI can start without switching levels.
          if (filepath.IsEmpty())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING,
              TEXT("JSON: Database ID=%d has an empty filepath. No Unreal Level will be loaded for this database. "
                "Use this when your your level starts loaded or is loaded through non-CIGI means, like Blueprints."),
              id);
            // Set the filepath to "none" to indicate an intentionally blank path. This will not be modified by the FullAssetPath() function.
            filepath = "none";
          }

          // Try to read lat/lon/alt data
          double dbLat = 0;
          if (!dbObject->TryGetNumberField(TEXT("lat"), dbLat))
          {
            if (!dbObject->TryGetNumberField(TEXT("latitude"), dbLat))
            {
              dbLat = 0;
            }
          }

          double dbLon = 0;
          if (!dbObject->TryGetNumberField(TEXT("lon"), dbLon))
          {
            if (!dbObject->TryGetNumberField(TEXT("longitude"), dbLon))
            {
              dbLon = 0;
            }
          }

          double dbAlt = 0;
          if (!dbObject->TryGetNumberField(TEXT("alt"), dbAlt))
          {
            if (!dbObject->TryGetNumberField(TEXT("altitude"), dbAlt))
            {
              dbAlt = 0;
            }
          }

          // Store the database information in the DatabaseManager's Databases map
          DatabaseInfo newDatabase = DatabaseInfo();
          newDatabase.assetPath = CUnrealCigiConfigLoader::FullAssetPath(filepath);
          newDatabase.origin = FVector(dbLat, dbLon, dbAlt);
          FUnrealCigi_PluginModule::globals.pDatabaseManager->Databases.Add(id, newDatabase);
          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: saved database: id=%d, lat=%.1f, lon=%.1f, alt=%.1f, path=\"%s\""), id, dbLat, dbLon, dbAlt, *filepath);

          // Warn the user if a database does not have an origin
          if (newDatabase.origin == FVector::Zero())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: database id=%d has no origin or epsg. Set an origin with 'lat', 'lon', 'alt'."), id, dbLat, dbLon, dbAlt, *filepath);
          }
        }
      }

      // --- Attempt to read "fonts" array ---
      const TArray<TSharedPtr<FJsonValue>>* p_ftValArray;
      if (!ConfigObject->TryGetArrayField(TEXT("fonts"), p_ftValArray))
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: json file \"%s\" does not have a \"fonts\" array!"), *filePath);
      }
      else
      {
        // Iterate over every font name in the "font" array, and attempt to record their fontID with the matching UFont* object
        for (TSharedPtr<FJsonValue> ftVal : (*p_ftValArray))
        {
          // Get the current dbObject: (curr item in "databases" list)
          const TSharedPtr<FJsonObject>* p_ftObject;
          if (!ftVal->TryGetObject(p_ftObject))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped database b/c ftVal->TryGetObject failed"));
            continue;
          }
          TSharedPtr<FJsonObject> ftObject = *p_ftObject;
          if (!ftObject.IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped database b/c ftObject is not valid"));
            continue;
          }

          // Read required properties:
          FString name;
          if (!ftObject->TryGetStringField(TEXT("name"), name))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped font b/c name is not valid"));
            continue;
          }
          if (name.IsEmpty())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped font b/c name is empty"));
            continue;
          }
          int32 id;
          if (!ftObject->TryGetNumberField(TEXT("fontID"), id) || id <= 0 || id > MAX_uint8)
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: skipped font \"%s\" b/c fontID is not valid!"), *name);
            continue;
          }
          if (!FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RegisterFontByName(sbio::FontID(id), name))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: skipped font \"%s\" because the ID is taken or the project font was not found"), *name);
            continue;
          }
          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: successfully recorded fontID %d with UFont \"%s\""), id, *name);
        }
      }

      // --- Attempt to read "symbolTextures" array ---
      const TArray<TSharedPtr<FJsonValue>>* textureValues;
      if (ConfigObject->TryGetArrayField(TEXT("symbolTextures"), textureValues))
      {
        for (const TSharedPtr<FJsonValue>& textureValue : *textureValues)
        {
          const TSharedPtr<FJsonObject>* textureObjectPointer;
          if (!textureValue->TryGetObject(textureObjectPointer) || !textureObjectPointer->IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: skipped invalid texture entry"));
            continue;
          }

          const TSharedPtr<FJsonObject>& textureObject = *textureObjectPointer;
          int32 textureID;
          FString filepath;
          if (!textureObject->TryGetNumberField(TEXT("textureID"), textureID) || textureID <= 0 || textureID > MAX_uint16 || !textureObject->TryGetStringField(TEXT("filepath"), filepath) || filepath.IsEmpty())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: skipped texture with invalid textureID or filepath"));
            continue;
          }

          const FString assetPath = CUnrealCigiConfigLoader::FullAssetPath(filepath);
          UTexture2D* texture = LoadObject<UTexture2D>(nullptr, *assetPath);
          if (!IsValid(texture))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: failed to load textureID %d from '%s'"), textureID, *assetPath);
            continue;
          }

          if (!FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RegisterTexture(sbio::TextureID(textureID), texture))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: failed to register textureID %d from '%s'"), textureID, *assetPath);
            continue;
          }
          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: registered textureID %d from '%s'"), textureID, *assetPath);
        }
      }

      // --- Attempt to read "entities" array ---
      const TArray<TSharedPtr<FJsonValue>>* p_entityValArray;
      if (!ConfigObject->TryGetArrayField(TEXT("entities"), p_entityValArray))
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: json file \"%s\" does not have an \"entities\" array!"), *filePath);
      }
      else
      {
        // Iterate over every entity in the "entities" array, attempt to load their config info:
        for (TSharedPtr<FJsonValue> entityVal : (*p_entityValArray))
        {
          // Get the current EntityObject: (curr item in "entities" list)
          const TSharedPtr<FJsonObject>* p_entityObject;
          if (!entityVal->TryGetObject(p_entityObject))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped entity b/c entityVal->TryGetObject failed"));
            continue;
          }
          TSharedPtr<FJsonObject> entityObject = *p_entityObject;
          if (!entityObject.IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped entity b/c entityObject is not valid"));
            continue;
          }

          // Read required properties:
          FString name;
          if (!entityObject->TryGetStringField(TEXT("name"), name))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped entity b/c name is not valid"));
            continue;
          }
          FString sisoID;
          if (!entityObject->TryGetStringField(TEXT("sisoID"), sisoID))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped entity \"%s\" b/c sisoID is not valid"), *name);
            continue;
          }
          // (Make sure sisoID is unique)
          if (FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Configs.Contains(sisoID))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped entity \"%s\" because sisoID \"%s\" is already loaded. Please use a unique sisoID."), *name, *sisoID);
            continue;
          }

          FString filepath;
          if (!entityObject->TryGetStringField(TEXT("filepath"), filepath))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped entity \"%s\" b/c filepath is not valid"), *name);
            continue;
          }
          FVector offset = CUnrealCigiConfigLoader::ParseVector(entityObject, "offset");
          FVector rotation = CUnrealCigiConfigLoader::ParseVector(entityObject, "rotation");
          FVector scale = CUnrealCigiConfigLoader::ParseVector(entityObject, "scale");
          if (scale.Equals(FVector(0, 0, 0)))
          {
            scale = FVector(1, 1, 1);
          }

          // Read articulated parts, if any:
          TMap<int32, FArticulatedPartConfig> articulatedParts = TMap<int32, FArticulatedPartConfig>();
          const TArray<TSharedPtr<FJsonValue>>* p_partsArray;
          if (entityObject->TryGetArrayField(TEXT("articulatedParts"), p_partsArray))
          {
            for (TSharedPtr<FJsonValue> partVal : (*p_partsArray))
            {
              // Get the current PartObject: (curr item in "articulatedParts" list)
              const TSharedPtr<FJsonObject>* p_partObject;
              if (!partVal->TryGetObject(p_partObject))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Entity \"%s\": Skipped articulated part b/c TryGetObject failed!"), *name);
                continue;
              }
              TSharedPtr<FJsonObject> partObject = *p_partObject;
              if (!partObject.IsValid())
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Entity \"%s\": Skipped articulated part b/c partObject was not valid!"), *name);
                continue;
              }

              // Read articulated part properties:
              FString partName;
              if (!partObject->TryGetStringField(TEXT("name"), partName))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped part in entity \"%s\" b/c partName is not valid!"), *name);
                continue;
              }
              int32 partID;
              if (!partObject->TryGetNumberField(TEXT("partID"), partID))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped part \"%s\" in entity \"%s\" b/c partID is not valid!"), *partName, *name);
                continue;
              }
              // (Make sure that the partID is unique)
              if (articulatedParts.Contains(partID))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped part \"%s\" in entity \"%s\" b/c partID %d is a duplicate!"), *partName, *name, partID);
                continue;
              }
              FString partFilepath;
              if (!partObject->TryGetStringField(TEXT("filepath"), partFilepath))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped part \"%s\" in entity \"%s\" b/c partFilepath is not valid!"), *partName, *name);
                continue;
              }
              FVector partOrigin = CUnrealCigiConfigLoader::ParseVector(partObject, "origin");
              FVector partOffset = CUnrealCigiConfigLoader::ParseVector(partObject, "offset");
              FVector partRotation = CUnrealCigiConfigLoader::ParseVector(partObject, "rotation");
              FVector partScale = CUnrealCigiConfigLoader::ParseVector(partObject, "scale");
              if (partScale.Equals(FVector(0, 0, 0)))
              {
                partScale = FVector(1, 1, 1);
              }

              // Create a new ArticulatedPartConfig for the above info, and store it into the articulated parts map
              articulatedParts.Add(partID, FArticulatedPartConfig(partName, CUnrealCigiConfigLoader::FullAssetPath(partFilepath), partOrigin, FTransform(FQuat::MakeFromEuler(partRotation), partOffset, partScale)));
            }
            UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Entity \"%s\": Done loading articulated parts"), *name);
          }
          else
          {
            UE_LOG(LogCigiEventHandler, Verbose, TEXT("JSON: Entity \"%s\": No articulated parts detected"), *name);
          }

          // Read articulated bones, if any:
          TMap<FString, int32> articulatedBones = TMap<FString, int32>();
          TSet<int32> articulatedBoneIDs = TSet<int32>();
          const TArray<TSharedPtr<FJsonValue>>* p_bonesArray;
          if (entityObject->TryGetArrayField(TEXT("articulatedBones"), p_bonesArray))
          {
            for (TSharedPtr<FJsonValue> boneVal : (*p_bonesArray))
            {
              // Get the current boneObject: (curr item in "articulatedBones" list)
              const TSharedPtr<FJsonObject>* p_boneObject;
              if (!boneVal->TryGetObject(p_boneObject))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Entity \"%s\": Skipped articulated bone b/c TryGetObject failed!"), *name);
                continue;
              }
              TSharedPtr<FJsonObject> boneObject = *p_boneObject;
              if (!boneObject.IsValid())
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Entity \"%s\": Skipped articulated bone b/c boneObject was not valid!"), *name);
                continue;
              }

              // Read articulated bone properties:
              FString boneName;
              if (!boneObject->TryGetStringField(TEXT("name"), boneName))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped bone in entity \"%s\" b/c boneName is not valid!"), *name);
                continue;
              }
              if (articulatedBones.Contains(boneName))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped bone \"%s\" in entity \"%s\" b/c its name is a duplicate!"), *boneName, *name);
                continue;
              }
              int32 boneID;
              if (!boneObject->TryGetNumberField(TEXT("boneID"), boneID))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped bone \"%s\" in entity \"%s\" b/c boneID is not valid!"), *boneName, *name);
                continue;
              }
              if (articulatedBoneIDs.Contains(boneID) || articulatedParts.Contains(boneID))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped bone \"%s\" in entity \"%s\" b/c boneID %d is a duplicate!"), *boneName, *name, boneID);
                continue;
              }

              // Record the bone's name and id
              articulatedBones.Add(boneName, boneID);
              // Also record the id in a hash set to check for duplicates in future iterations of this loop
              articulatedBoneIDs.Add(boneID);
            }
            UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Entity \"%s\": Done loading articulated bones"), *name);
          }
          else
          {
            UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Entity \"%s\": No articulated bones detected"), *name);
          }

          // Record the short entity id
          bool shortTypeSuccessful = false;
          int32 nShortEntityTypeID;
          if (!entityObject->TryGetNumberField(TEXT("shortEntityTypeID"), nShortEntityTypeID))
          {
            if (FUnrealCigi_PluginModule::globals.pImageGenerator->GetSetupOptions().eCigiVersion < sbio::cigi::ECigiVersion::VERSION_4_0)
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Entity \"%s\": shortEntityTypeID is missing. This is required for CIGI 3.3 and is optional for CIGI 4.0."), *name);
            }
          }
          else
          {
            // Check if the shortEntityTypeID is already taken. If not, add it to the ShortTypeToExtended map
            if (!FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ShortTypeToExtended.Contains(nShortEntityTypeID))
            {
              FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ShortTypeToExtended.Add(nShortEntityTypeID, sisoID);
              shortTypeSuccessful = true;
            }
            else
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Entity \"%s\" has shortID %d, which is already taken. Failed to save shortID."), *name, nShortEntityTypeID);
            }
          }

          // Finally, create a new EntityConfig to store all of the above info
          UEntityConfig* ecPtr = NewObject<UEntityConfig>();
          ecPtr->AddToRoot();
          ecPtr->InitJSON(name, CUnrealCigiConfigLoader::FullAssetPath(filepath), FTransform(FQuat::MakeFromEuler(rotation), offset, scale), articulatedParts, articulatedBones);
          FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Configs.Emplace(sisoID, ecPtr);

          if (shortTypeSuccessful)
          {
            UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: saved id '%s' with shortID '%d' and ConfigInfo '%s'"), *sisoID, nShortEntityTypeID, ecPtr == nullptr ? TEXT("NULL") : *ecPtr->ToString());
          }
          else
          {
            UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: saved id '%s' with shortID 'NONE' and ConfigInfo '%s'"), *sisoID, ecPtr == nullptr ? TEXT("NULL") : *ecPtr->ToString());
          }
        }
      }

      // --- Attempt to read "symbolTemplates" array ---
      const TArray<TSharedPtr<FJsonValue>>* p_symbolValArray;
      if (!ConfigObject->TryGetArrayField(TEXT("symbolTemplates"), p_symbolValArray))
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: json file \"%s\" does not have a \"symbolTemplates\" array!"), *filePath);
      }
      else
      {
        // Iterate over every symbol template in the "symbolTemplates" array, attempt to load their config info:
        for (TSharedPtr<FJsonValue> symbolVal : (*p_symbolValArray))
        {
          // Get the current symbolObject: (curr item in "symbols" list)
          const TSharedPtr<FJsonObject>* p_symbolObject;

          // Check if the symbolVal is valid
          if (!symbolVal->TryGetObject(p_symbolObject))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped symbol b/c symbolVal->TryGetObject failed"));
            continue;
          }

          TSharedPtr<FJsonObject> symbolObject = *p_symbolObject;

          // Check if the symbolObject is valid
          if (!symbolObject.IsValid())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped symbol b/c symbolObject is not valid"));
            continue;
          }

          // Read required properties:
          USymbolConfig* symbolConfig = NewObject<USymbolConfig>();

          // (Note: TryGetNumberField(string,&int) will round a json double to an out int)
          if (!symbolObject->TryGetNumberField(TEXT("templateID"), symbolConfig->templateID))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped symbol b/c templateID is not valid"));
            continue;
          }

          // Make sure templateID is valid and unique
          if (symbolConfig->templateID > 0 && FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbolTemplate(symbolConfig->templateID) != nullptr)
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped symbol because templateID \"%d\" is already loaded. Please use a unique templateID."), symbolConfig->templateID);
            continue;
          }

          FString symbolString;
          if (!symbolObject->TryGetStringField(TEXT("symbol"), symbolString))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped symbol \"%d\" b/c symbol is not valid"), symbolConfig->templateID);
            continue;
          }

          // Ignore case and ignore spaces
          symbolString.ToLowerInline();
          symbolString.RemoveSpacesInline();

          // Find what kind of symbol this is. 1 = circle, 2 = polygon, 3 = textured circle, 4 = textured polygon
          if (symbolString.Equals("circle"))
          {
            symbolConfig->symbol = 1;
          }
          else if (symbolString.Equals("polygon"))
          {
            symbolConfig->symbol = 2;
          }
          else if (symbolString.Equals("texturedcircle"))
          {
            symbolConfig->symbol = 3;
          }
          else if (symbolString.Equals("texturedpolygon"))
          {
            symbolConfig->symbol = 4;
          }

          // Make sure the symbol type is valid. If not, skip this symbol template.
          if (symbolConfig->symbol <= 0)
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING,
              TEXT("JSON: Skipped symbol template \"%d\" b/c symbol \"%s\" is not valid."
                "Must be 'circle', 'polygon', 'textured circle', or 'textured polygon'"),
              symbolConfig->templateID, *symbolString);
            continue;
          }

          const sbio::symbol::ESymbolType symbolType = symbolConfig->GetSymbolType();
          if (symbolType == sbio::symbol::ESymbolType::CIRCLE)
          {
            FString drawingStyle;
            if (!symbolObject->TryGetStringField(TEXT("drawingStyle"), drawingStyle))
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped circle symbol \"%d\" b/c drawingStyle is not valid"), symbolConfig->templateID);
              continue;
            }
            drawingStyle.ToLowerInline();
            drawingStyle.RemoveSpacesInline();
            if (drawingStyle.Equals(TEXT("line")))
            {
              symbolConfig->drawingStyle = static_cast<int32>(sbio::symbol::EDrawingStyle::LINE);
            }
            else if (drawingStyle.Equals(TEXT("fill")))
            {
              symbolConfig->drawingStyle = static_cast<int32>(sbio::symbol::EDrawingStyle::FILL);
            }
            else
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped circle symbol \"%d\" b/c drawingStyle \"%s\" is invalid"), symbolConfig->templateID, *drawingStyle);
              continue;
            }
          }
          else if (symbolType == sbio::symbol::ESymbolType::POLYGON || symbolType == sbio::symbol::ESymbolType::TEXTURED_POLYGON)
          {
            FString primitiveType;
            if (!symbolObject->TryGetStringField(TEXT("primitiveType"), primitiveType))
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped polygon symbol \"%d\" b/c primitiveType is not valid"), symbolConfig->templateID);
              continue;
            }
            primitiveType.ToLowerInline();
            primitiveType.RemoveSpacesInline();
            if (primitiveType.Equals(TEXT("point")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::POINT);
            }
            else if (primitiveType.Equals(TEXT("line")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::LINE);
            }
            else if (primitiveType.Equals(TEXT("linestrip")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::LINE_STRIP);
            }
            else if (primitiveType.Equals(TEXT("lineloop")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::LINE_LOOP);
            }
            else if (primitiveType.Equals(TEXT("triangle")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::TRIANGLE);
            }
            else if (primitiveType.Equals(TEXT("trianglestrip")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::TRIANGLE_STRIP);
            }
            else if (primitiveType.Equals(TEXT("trianglefan")))
            {
              symbolConfig->primitiveType = static_cast<int32>(sbio::symbol::EPrimitiveType::TRIANGLE_FAN);
            }
            else
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped polygon symbol \"%d\" b/c primitiveType \"%s\" is invalid"), symbolConfig->templateID, *primitiveType);
              continue;
            }
          }

          // Only non-textured circle and polygon symbols use line attributes.
          if ((symbolType == sbio::symbol::ESymbolType::CIRCLE || symbolType == sbio::symbol::ESymbolType::POLYGON) && symbolConfig->IsLine())
          {
            // Line width is required for line symbols
            if (!symbolObject->TryGetNumberField(TEXT("lineWidth"), symbolConfig->lineWidth))
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped line symbol \"%d\" b/c lineWidth is not valid"), symbolConfig->templateID);
              continue;
            }

            // Stipple patterns are not required. If none are given, it defaults to a filled line
            if (!symbolObject->TryGetNumberField(TEXT("stipple"), symbolConfig->stipple))
            {
              symbolConfig->stipple = 0xFFFF;
            }
            if (!symbolObject->TryGetNumberField(TEXT("stippleLength"), symbolConfig->stippleLength))
            {
              symbolConfig->stippleLength = 0;
            }
          }

          if (symbolType == sbio::symbol::ESymbolType::TEXTURED_CIRCLE || symbolType == sbio::symbol::ESymbolType::TEXTURED_POLYGON)
          {
            if (!symbolObject->TryGetNumberField(TEXT("textureID"), symbolConfig->textureID) || symbolConfig->textureID < 0)
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped textured symbol \"%d\" b/c textureID is not valid"), symbolConfig->templateID);
              continue;
            }
            if (!symbolObject->TryGetNumberField(TEXT("textureFilterMode"), symbolConfig->textureFilterMode) || symbolConfig->textureFilterMode < 0 || symbolConfig->textureFilterMode > 1)
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped textured symbol \"%d\" b/c textureFilterMode is not 0 (nearest) or 1 (linear)"), symbolConfig->templateID);
              continue;
            }
            if (!symbolObject->TryGetNumberField(TEXT("textureWrapMode"), symbolConfig->textureWrapMode) || symbolConfig->textureWrapMode < 0 || symbolConfig->textureWrapMode > 1)
            {
              UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped textured symbol \"%d\" b/c textureWrapMode is not 0 (repeat) or 1 (clamp)"), symbolConfig->templateID);
              continue;
            }
          }

          // Read the shapes array
          const TArray<TSharedPtr<FJsonValue>>* p_shapesArray;
          if (symbolObject->TryGetArrayField(TEXT("shapes"), p_shapesArray))
          {
            int32 loadedShapeCount = 0;
            for (TSharedPtr<FJsonValue> shapeVal : (*p_shapesArray))
            {
              // Get the current PartObject: (curr item in "articulatedParts" list)
              const TSharedPtr<FJsonObject>* p_shapeObject;
              if (!shapeVal->TryGetObject(p_shapeObject))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: symbol \"%d\": Skipped shape b/c TryGetObject failed!"), symbolConfig->templateID);
                continue;
              }
              TSharedPtr<FJsonObject> shapeObject = *p_shapeObject;
              if (!shapeObject.IsValid())
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: symbol \"%d\": Skipped shape b/c shapeObject was not valid!"), symbolConfig->templateID);
                continue;
              }

              // Read shape position. (center or vertex is allowed for any shape)
              double centerU = 0;
              if (!shapeObject->TryGetNumberField(TEXT("centerU"), centerU))
              {
                if (!shapeObject->TryGetNumberField(TEXT("vertexU"), centerU))
                {
                  UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: symbol \"%d\": Skipped shape b/c there is no 'centerU' or 'vertexU'!"), symbolConfig->templateID);
                  continue;
                }
              }

              // Read shape position. (center or vertex is allowed for any shape)
              double centerV = 0;
              if (!shapeObject->TryGetNumberField(TEXT("centerV"), centerV))
              {
                if (!shapeObject->TryGetNumberField(TEXT("vertexV"), centerV))
                {
                  UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: symbol \"%d\": Skipped shape b/c there is no 'centerV' or 'vertexV'!"), symbolConfig->templateID);
                  continue;
                }
              }

              // If this shape is a polygon, stop here. Polygons only have a list of vertices.
              if (symbolType == sbio::symbol::ESymbolType::POLYGON)
              {
                symbolConfig->vertices.Add(FVector2D(centerU, centerV));
                ++loadedShapeCount;
                continue;
              }

              if (symbolType == sbio::symbol::ESymbolType::TEXTURED_POLYGON)
              {
                double textureS = 0;
                double textureT = 0;
                const bool hasTextureS = shapeObject->TryGetNumberField(TEXT("textureS"), textureS) || shapeObject->TryGetNumberField(TEXT("textureCoordinateS"), textureS);
                const bool hasTextureT = shapeObject->TryGetNumberField(TEXT("textureT"), textureT) || shapeObject->TryGetNumberField(TEXT("textureCoordinateT"), textureT);
                if (!hasTextureS || !hasTextureT)
                {
                  UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: textured polygon \"%d\": Skipped vertex b/c textureS or textureT is missing"), symbolConfig->templateID);
                  continue;
                }

                FTexturedPolygonTemplateVertex vertex;
                vertex.uv = FVector2D(centerU, centerV);
                vertex.textureCoordinateST = FVector2D(textureS, textureT);
                symbolConfig->texturedPolygonVertices.Add(vertex);
                ++loadedShapeCount;
                continue;
              }

              // If this shape is a circle, keep going. Radius is a required field.
              double radius;
              if (!shapeObject->TryGetNumberField(TEXT("radius"), radius))
              {
                UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: symbol \"%d\": Skipped circle shape b/c there is no 'radius'!"), symbolConfig->templateID);
                continue;
              }

              double innerRadius;
              if (!shapeObject->TryGetNumberField(TEXT("innerRadius"), innerRadius))
              {
                innerRadius = 0;
              }
              double startAngle;
              if (!shapeObject->TryGetNumberField(TEXT("startAngle"), startAngle))
              {
                startAngle = 0;
              }
              double endAngle;
              if (!shapeObject->TryGetNumberField(TEXT("endAngle"), endAngle))
              {
                endAngle = 360;
              }

              if (symbolType == sbio::symbol::ESymbolType::TEXTURED_CIRCLE)
              {
                double centerTextureS = 0;
                double centerTextureT = 0;
                double textureMapRadius = 0;
                double textureMapRotation = 0;
                const bool hasCenterTextureS = shapeObject->TryGetNumberField(TEXT("centerTextureS"), centerTextureS) || shapeObject->TryGetNumberField(TEXT("centerTextureU"), centerTextureS);
                const bool hasCenterTextureT = shapeObject->TryGetNumberField(TEXT("centerTextureT"), centerTextureT) || shapeObject->TryGetNumberField(TEXT("centerTextureV"), centerTextureT);
                if (!hasCenterTextureS || !hasCenterTextureT || !shapeObject->TryGetNumberField(TEXT("textureMapRadius"), textureMapRadius))
                {
                  UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: textured circle \"%d\": Skipped shape b/c texture center or textureMapRadius is missing"), symbolConfig->templateID);
                  continue;
                }
                shapeObject->TryGetNumberField(TEXT("textureMapRotation"), textureMapRotation);

                FTexturedCircleTemplateElement element;
                element.centerUV = FVector2D(centerU, centerV);
                element.radius = radius;
                element.innerRadius = innerRadius;
                element.angles = FVector2D(startAngle, endAngle);
                element.centerTextureST = FVector2D(centerTextureS, centerTextureT);
                element.textureMapRadius = textureMapRadius;
                element.textureMapRotation = textureMapRotation;
                symbolConfig->texturedCircles.Add(element);
              }
              else
              {
                symbolConfig->vertices.Add(FVector2D(centerU, centerV));
                symbolConfig->radii.Add(FVector2D(radius, innerRadius));
                symbolConfig->angles.Add(FVector2D(startAngle, endAngle));
              }
              ++loadedShapeCount;
            }
            UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: symbol \"%d\": Done loading %d shapes"), symbolConfig->templateID, loadedShapeCount);
          }
          else
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: symbol \"%d\": No shapes detected"), symbolConfig->templateID);
          }

          // Now that the symbol template is fully read from JSON, check that it is valid
          FString errMsg = symbolConfig->ValidateSymbol();
          if (!errMsg.IsEmpty())
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Symbol \"%d\": Validate Error: %s"), symbolConfig->templateID, *errMsg);
            continue;
          }

          if (!FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RegisterSymbolTemplate(symbolConfig->templateID, symbolConfig))
          {
            UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Failed to register symbol template '%d'"), symbolConfig->templateID);
            continue;
          }

          UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: saved template '%d' with SymbolConfig '%s'"), symbolConfig->templateID, *symbolConfig->ToString());
        }
      }
    }

    void CUnrealCigiConfigLoader::ReleaseRootedConfigObjects()
    {
      if (FUnrealCigi_PluginModule::globals.pUnrealEntityManager != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pUnrealEntityManager->ReleaseConfigs();
      }

      if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->ClearSymbolTemplates();
      }
    }

    TSharedPtr<FJsonObject> CUnrealCigiConfigLoader::LoadJsonConfig(FString& filePath)
    {
      // Start searching for the config file in the project directory
      FString folder = FPaths::GetPath(FPaths::GetProjectFilePath());
      folder = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*folder);
      FString contents;
      while (true)
      {
        // If the folder is empty, we have reached the root and cannot find the config file
        if (folder.IsEmpty())
        {
          UE_LOG(LogCigiEventHandler, Error, TEXT("JSON: Could not find UnrealCigi.config.json in a parent folder."));
          return nullptr;
        }

        // Check if the config file exists in the current folder
        filePath = folder / TEXT("UnrealCigi.config.json");
        if (FFileHelper::LoadFileToString(contents, *filePath))
        {
          TSharedPtr<FJsonObject> object = MakeShared<FJsonObject>();
          const TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(contents);
          if (FJsonSerializer::Deserialize(reader, object) && object.IsValid())
          {
            UE_LOG(LogCigiEventHandler, Log, TEXT("JSON: Loaded config file '%s'"), *filePath);
            return object;
          }
          UE_LOG(LogCigiEventHandler, Error, TEXT("JSON: Could not parse config file '%s'"), *filePath);
        }

        // Move up to the parent folder and try again
        folder = utils::GetParentFolder(folder);
      }
    }

    FVector CUnrealCigiConfigLoader::ParseVector(const TSharedPtr<FJsonObject>& object, const FString& fieldName)
    {
      // Initialize the result vector to zero
      FVector result = FVector::ZeroVector;

      // Attempt to retrieve the array field from the JSON object
      const TArray<TSharedPtr<FJsonValue>>* values = nullptr;
      if (object.IsValid() && object->TryGetArrayField(fieldName, values) && values != nullptr)
      {
        double value = 0;
        if (values->Num() > 0 && (*values)[0].IsValid() && (*values)[0]->TryGetNumber(value))
        {
          result.X = value;
        }
        if (values->Num() > 1 && (*values)[1].IsValid() && (*values)[1]->TryGetNumber(value))
        {
          result.Y = value;
        }
        if (values->Num() > 2 && (*values)[2].IsValid() && (*values)[2]->TryGetNumber(value))
        {
          result.Z = value;
        }
      }
      return result;
    }

    FString CUnrealCigiConfigLoader::FullAssetPath(const FString& filepath)
    {
      return filepath.Equals(TEXT("none"), ESearchCase::IgnoreCase) ? filepath : FString::Printf(TEXT("/Game/%s"), *filepath);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026