//Copyright SimBlocks LLC 2016-2026

#include "EntityConfig.h"

FArticulatedPartConfig::FArticulatedPartConfig(FString partName, FString partFilepath, FVector partOrigin, FTransform partTransform)
{
  name = partName;
  filepath = partFilepath;
  origin = partOrigin;
  transform = partTransform;
}

FArticulatedPartConfig::FArticulatedPartConfig()
{
  name = "";
  filepath = "";
  origin = FVector();
  transform = FTransform();
}

void UEntityConfig::InitJSON(FString _name, FString _filepath, FTransform _transform, TMap<int32, FArticulatedPartConfig> _articulatedParts, TMap<FString, int32> _articulatedBones)
{
  blueprint = nullptr;
  name = _name;
  filepath = _filepath;
  transform = _transform;
  articulatedParts = _articulatedParts;
  articulatedBones = _articulatedBones;
}

void UEntityConfig::InitBP(TSubclassOf<ACigiEntity> _blueprint)
{
  blueprint = _blueprint;
  name = "";
  filepath = "";
  transform = FTransform(FQuat::MakeFromEuler(FVector(0, 0, 0)), FVector(0, 0, 0), FVector(0, 0, 0));
  articulatedParts = TMap<int32, FArticulatedPartConfig>();
  articulatedBones = TMap<FString, int32>();
}

UEntityConfig::UEntityConfig()
{
  blueprint = nullptr;
  name = "";
  filepath = "";
  transform = FTransform(FQuat::MakeFromEuler(FVector(0, 0, 0)), FVector(0, 0, 0), FVector(0, 0, 0));
  articulatedParts = TMap<int32, FArticulatedPartConfig>();
  articulatedBones = TMap<FString, int32>();
}

FString UEntityConfig::ToString() const
{
  if (blueprint != nullptr)
  {
    return FString::Printf(TEXT("{BP=\"%s\"}"), *blueprint->GetName());
  }
  else
  {
    return FString::Printf(TEXT("{name=\"%s\", filepath=\"%s\", transform=\"%s\"}"), *name, *filepath, *transform.ToString());
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026