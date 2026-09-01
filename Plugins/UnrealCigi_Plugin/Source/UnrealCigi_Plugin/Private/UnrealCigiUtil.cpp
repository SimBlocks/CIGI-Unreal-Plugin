//Copyright SimBlocks LLC 2016-2026

#include "unrealcigiUtil.h"
#include "DrawDebugHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CigiEntity.h"
#include "Engine/StreamableManager.h"
#include "Runtime/Launch/Resources/Version.h"

DEFINE_LOG_CATEGORY(LogCigiUtil)

namespace sbio::unrealcigi::utils
{
  // To/From Vec3Body
  sbio::math::BodyCoordinates FVectorToBodyCoordinates(FVector fv)
  {
    // ***Flip X/Y*** (NEU -> ENU) and divide by 100 (centimeters -> meters)
    sbio::math::BodyCoordinates vec3Body;
    vec3Body[0] = fv.Y / 100.0;
    vec3Body[1] = fv.X / 100.0;
    vec3Body[2] = fv.Z / 100.0;
    return vec3Body;
  }

  FQuat BodyRotationToFQuat(sbio::ig::BodyRotation bodyRot)
  {
    return FRotationMatrix::MakeFromXZ(BodyCoordinatesToFVector(bodyRot.Forward), BodyCoordinatesToFVector(bodyRot.Up)).ToQuat();
  }

  FTransform BodyTransformToFTransform(sbio::math::BodyCoordinates bodyPos, sbio::ig::BodyRotation bodyRot)
  {
    return FTransform(BodyRotationToFQuat(bodyRot), BodyCoordinatesToFVector(bodyPos), FVector(1, 1, 1));
  }

  FVector BodyCoordinatesToFVector(sbio::math::BodyCoordinates vec3Body)
  {
    FVector fv;

    // ***Flip X/Y*** (ENU -> NEU) and multiply by 100 (meters -> centimeters)
    fv.Y = vec3Body[0] * 100.0;
    fv.X = vec3Body[1] * 100.0;
    fv.Z = vec3Body[2] * 100.0;
    return fv;
  }

  sbio::math::ReferencePlaneCoordinates FVectorToReferencePlaneCoordinates(const FVector& fv)
  {
    // ReferencePlaneCoordinates use north/east/down. UnrealCigi uses X=north, Y=east, Z=up.
    sbio::math::ReferencePlaneCoordinates referencePlaneCoords;
    referencePlaneCoords[0] = fv.X / 100.0;
    referencePlaneCoords[1] = fv.Y / 100.0;
    referencePlaneCoords[2] = -fv.Z / 100.0;
    return referencePlaneCoords;
  }

  FVector ReferencePlaneCoordinatesToFVector(const sbio::math::ReferencePlaneCoordinates& referencePlaneCoords)
  {
    FVector fv;

    // ReferencePlaneCoordinates use north/east/down. UnrealCigi uses X=north, Y=east, Z=up.
    fv.X = referencePlaneCoords[0] * 100.0;
    fv.Y = referencePlaneCoords[1] * 100.0;
    fv.Z = -referencePlaneCoords[2] * 100.0;
    return fv;
  }

  // To/From CigiBodyCoordinates
  sbio::cigi::CigiBodyCoordinates FVectorToCigiBodyCoordinates(FVector fv)
  {
    // Negate Z (FRU -> FRD) and divide by 100 (centimeters -> meters)
    return sbio::cigi::CigiBodyCoordinates(fv.X / 100.0, fv.Y / 100.0, -fv.Z / 100.0);
  }

  FVector CigiBodyCoordinatesToFVector(sbio::cigi::CigiBodyCoordinates bodyCoordinates)
  {
    FVector fv;

    // Negate Z (FRD -> FRU) and multiply by 100 (meters -> centimeters)
    fv.X = bodyCoordinates.toVec3().x() * 100.0;
    fv.Y = bodyCoordinates.toVec3().y() * 100.0;
    fv.Z = -bodyCoordinates.toVec3().z() * 100.0;
    return fv;
  }

  FString ObjName(UObject* obj)
  {
    if (!IsValid(obj))
    {
      return TEXT("NULL");
    }
    return obj->GetName();
  }

  FString TransformString(FTransform transform, uint8 precision)
  {
    return FString::Printf(TEXT("{%s %s %s}"), *VectorString(transform.GetLocation(), precision), *VectorString(transform.GetRotation().Euler(), precision), *VectorString(transform.GetScale3D(), precision));
  }

  FString VectorString(FVector vector, uint8 precision)
  {
    return FString::Printf(TEXT("(%.*f,%.*f,%.*f)"), precision, vector.X, precision, vector.Y, precision, vector.Z);
  }

  void DebugLine(const UWorld* InWorld, FVector start, FVector end, FColor color, float duration)
  {
    if (!ENABLE_DBG_DRAW)
    {
      return;
    }

    DrawDebugLine(InWorld, start, end, color, duration < 0, duration, 0, DBG_DRAW_THICKNESS);
  }

  void DebugSphere(const UWorld* InWorld, FVector point, double radius, FColor color, float duration)
  {
    if (!ENABLE_DBG_DRAW)
    {
      return;
    }

    DrawDebugSphere(InWorld, point, radius, 8, color, duration < 0, duration, 0, DBG_DRAW_THICKNESS);
  }

  FString GetParentFolder(FString inPath)
  {
    if (inPath.IsEmpty())
    {
      return FString();
    }

    // ONLY if there is a slash at the very end of the string, remove it: "C:/Foo/Bar/" becomes "C:/Foo/Bar"
    inPath.RemoveFromEnd("/");
    // Find the index of the last slash: "C:/Foo/Bar" returns 6
    int shashIndex = 0;
    bool foundSlash = inPath.FindLastChar('/', shashIndex);

    // If there is no valid slash, return an error message
    if (!foundSlash || shashIndex == 0)
    {
      return FString();
    }

    // If a slash was found, return everything to the left of the slash: "C:/Foo/Bar" returns the left 6 characters "C:/Foo"
    return inPath.Left(shashIndex);
  }

  bool IsValidIPv4Address(const FString& ipAddress)
  {
    TArray<FString> octets;
    if (ipAddress.ParseIntoArray(octets, TEXT("."), true) != 4)
    {
      return false;
    }

    // Check each octet to ensure it is a valid number between 0 and 255
    for (const FString& octet : octets)
    {
      if (octet.IsEmpty())
      {
        return false;
      }

      for (TCHAR ch : octet)
      {
        if (!FChar::IsDigit(ch))
        {
          return false;
        }
      }

      int32 value = 0;
      if (!LexTryParseString(value, *octet) || value < 0 || value > 255)
      {
        return false;
      }
    }

    return true;
  }

  TArray<UClass*> FindBlueprintAssets(UClass* base)
  {
    TArray<UClass*> results;
    if (base == nullptr)
    {
      return results;
    }

    // Setup the asset registry
    FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    IAssetRegistry& assetRegistry = assetRegistryModule.Get();
    TArray<FString> contentFolders;
    contentFolders.Add("/Game");
    assetRegistry.ScanPathsSynchronous(contentFolders);

    // Get the names of all subclasses that derive from the base.
#if ENGINE_MAJOR_VERSION < 5
    TArray<FName> baseNames;
    baseNames.Add(base->GetFName());
    TSet<FName> subclassNames;
    TSet<FName> excludedNames;
    assetRegistry.GetDerivedClassNames(baseNames, excludedNames, subclassNames);
#else
    TArray<FTopLevelAssetPath> baseNames;
    baseNames.Add(base->GetClassPathName());
    TSet<FTopLevelAssetPath> subclassNames;
    TSet<FTopLevelAssetPath> excludedNames;
    assetRegistry.GetDerivedClassNames(baseNames, excludedNames, subclassNames);
#endif

    // Retrieve all Blueprint assets
    FARFilter filter;
#if ENGINE_MAJOR_VERSION < 5
    filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
#else
    filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
#endif
    filter.bRecursiveClasses = true;
    // (Not sure if bRecursivePaths matters since I'm not specifying any paths)
    filter.bRecursivePaths = true;
    TArray<FAssetData> assetList;
    assetRegistry.GetAssets(filter, assetList);

    // Loop through the retrieved blueprints and find the ones with the correct names
    for (FAssetData const& asset : assetList)
    {
      // Find the path to the generated class property
      const FString gcPath = asset.TagsAndValues.FindTag(FName("GeneratedClass")).GetValue();
      // Get the name of the generated class
      const FString gcObjectPath = FPackageName::ExportTextPathToObjectPath(gcPath);
      const FString gcName = FPackageName::ObjectPathToObjectName(gcObjectPath);
      // Check if the name matches one of the subclass names that we are looking for
#if ENGINE_MAJOR_VERSION < 5
      bool isSubclassName = subclassNames.Contains(*gcName);
#else
      bool isSubclassName = subclassNames.Contains(FTopLevelAssetPath(gcObjectPath));
#endif
      if (!isSubclassName)
      {
        continue;
      }

      // Convert the string into a FSoftObjectPath, which is used to initialize the TSoftClassPtr
      // This TSoftClassPtr holds an UNLOADED reference to the asset at the specified path
      TSoftClassPtr<UObject> bpSoftClass = TSoftClassPtr<UObject>(FSoftObjectPath(gcObjectPath));
      // Use LoadSynchronous() to load the asset and give the TSoftClassPtr a valid pointer
      // After the asset has been loaded once, you can just use TSoftClassPtr::Get() to get the reference again
      UClass* bpClass = bpSoftClass.LoadSynchronous();
      if (bpClass == nullptr)
      {
        UE_LOG(LogCigiUtil, BP_WARNING, TEXT("FindBlueprintAssets: Could not retrieve UClass pointer to BP Class '%s'"), *gcName);
        continue;
      }
      // Turn the UClass* pointer into a TSharedPtr<UClass> so that it isn't randomly cleared from memory while we're still using it
      results.Add(bpClass);
    }

    return results;
  }

  TArray<UFont*> FindAllFontAssets()
  {
    TArray<UFont*> results;

    // Setup the asset registry
    FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    IAssetRegistry& assetRegistry = assetRegistryModule.Get();
    TArray<FString> contentFolders;
    contentFolders.Add("/Game");
    assetRegistry.ScanPathsSynchronous(contentFolders);

    // Retrieve all Blueprint assets
    FARFilter filter;
#if ENGINE_MAJOR_VERSION < 5
    filter.ClassNames.Add(UFont::StaticClass()->GetFName());
#else
    filter.ClassPaths.Add(UFont::StaticClass()->GetClassPathName());
#endif
    filter.bRecursiveClasses = true;
    // (Not sure if bRecursivePaths matters since I'm not specifying any paths)
    filter.bRecursivePaths = true;
    TArray<FAssetData> assetList;
    assetRegistry.GetAssets(filter, assetList);

    int count = 0;

    // Loop through the retrieved assets and collect all of the valid font references
    for (FAssetData const& asset : assetList)
    {
      count++;
      UE_LOG(LogTemp, Log, TEXT("util::allFonts: found asset #%d"), count);
      UObject* object = asset.GetAsset();
      if (!IsValid(object))
      {
        UE_LOG(LogTemp, Warning, TEXT("util::allFonts: asset #%d: object is NULL"), count);
        continue;
      }
      UE_LOG(LogTemp, Log, TEXT("util::allFonts: asset #%d: object is %s"), count, *object->GetName());
      UFont* font = Cast<UFont>(object);
      if (!IsValid(object))
      {
        UE_LOG(LogTemp, Warning, TEXT("util::allFonts: asset #%d: font is NULL"), count);
        continue;
      }
      UE_LOG(LogTemp, Log, TEXT("util::allFonts: asset #%d: font is %s"), count, *font->GetName());
      results.Add(font);
    }

    return results;
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026