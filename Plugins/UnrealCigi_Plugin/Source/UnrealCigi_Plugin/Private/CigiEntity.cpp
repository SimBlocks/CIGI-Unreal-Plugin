//Copyright SimBlocks LLC 2016-2026

#include "CigiEntity.h"
#include "CigiView.h"

// simulation sdk headers
#include "IGCigiLib/CigiEvent.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiViewManager.h"
#include "UtilitiesLib/EventDispatcher.h"
#include "IGCigiLib/IGResponseEventDispatcher.h"

// unrealcigi headers
#include "unrealcigiEventHandler.h"
#include "CigiWidget.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogCigiEntity)

using namespace sbio;
using namespace sbio::symbol;
using namespace sbio::unrealcigi::utils;
using namespace sbio::unrealcigi;

#define COLLISION_ENABLE ECollisionEnabled::QueryOnly
#define COLLISION_CHANNEL ECollisionChannel::ECC_WorldDynamic
#define COLLISION_RESPONSE ECollisionResponse::ECR_Block

// Set this to true to draw collision volumes, set this to 0 to keep them invisible
const bool DrawDebugVolumes = false;

void FSisoID::Init(int kind, int domain, int country, int category, int subcategory, int specific, int extra)
{
  Kind = kind;
  Domain = domain;
  Country = country;
  Category = category;
  Subcategory = subcategory;
  Specific = specific;
  Extra = extra;
}

FSisoID::FSisoID(int kind, int domain, int country, int category, int subcategory, int specific, int extra)
{
  Init(kind, domain, country, category, subcategory, specific, extra);
}

FSisoID::FSisoID(sbio::entity::SEntityType original)
{
  Init(original.entityKindID.Value(), original.entityDomainID.Value(), original.entityCountryID.Value(), original.entityCategoryID.Value(), original.entitySubCategoryID.Value(), original.entitySpecificID.Value(), original.entityExtraID.Value());
}

FSisoID::FSisoID()
{
  Init(0, 0, 0, 0, 0, 0, 0);
}

FString FSisoID::ToString() const
{
  return FString::Printf(TEXT("%d.%d.%d.%d.%d.%d.%d"), Kind, Domain, Country, Category, Subcategory, Specific, Extra);
}

FArticulatedBoneData::FArticulatedBoneData()
{
  boneName = "";
  originTransform = FTransform(FQuat::MakeFromEuler(FVector(0, 0, 0)), FVector(0, 0, 0), FVector(1, 1, 1));
}

bool FArticulatedBoneData::HasName() const
{
  return !boneName.ToString().IsEmpty();
}

FComponentData::FComponentData()
{
  Data1 = 0;
  Data2 = 0;
  Data3 = 0;
  Data4 = 0;
  Data5 = 0;
  Data6 = 0;
}

FComponentData::FComponentData(int64 data1, int64 data2, int64 data3, int64 data4, int64 data5, int64 data6)
{
  Data1 = data1;
  Data2 = data2;
  Data3 = data3;
  Data4 = data4;
  Data5 = data5;
  Data6 = data6;
}

FComponentData::FComponentData(sbio::ig::SComponentData original)
{
  Data1 = original.ComponentData0;
  Data2 = original.ComponentData1;
  Data3 = original.ComponentData2;
  Data4 = original.ComponentData3;
  Data5 = original.ComponentData4;
  Data6 = original.ComponentData5;
}

FString FComponentData::ToString() const
{
  FString dataText = FString::Printf(TEXT("[%lld,%lld,%lld,%lld,%lld,%lld]"), Data1, Data2, Data3, Data4, Data5, Data6);
  return dataText;
}

FCollisionSegment::FCollisionSegment()
{
  start = FVector(0, 0, 0);
  end = FVector(0, 0, 0);
  // If a newly created segment has enable=true, the IgCigiLib does not send a separate SetEnable(true) command
  // So by default we should assume that all newly created segments are enabled
  enable = true;
}

FCollisionSegment::FCollisionSegment(FVector _start, FVector _end, bool _enable)
{
  start = _start;
  end = _end;
  enable = _enable;
}

FString FCollisionSegment::ToString() const
{
  return FString::Printf(TEXT("{en=%d,start=%s,end=%s}"), enable, *VectorString(start), *VectorString(end));
}

// ----- ACigiEntity -----

ACigiEntity::ACigiEntity()
{
  // Reset all variables
  StaticMeshComponent = nullptr;
  SkeletalMeshComponent = nullptr;
  ArticulatedParts.Empty();
  ArticulatedMesh = nullptr;
  ArticulatedBones.Empty();
  CollisionSegments.Empty();
  CollisionVolumes.Empty();
  Widgets.Empty();
  EntityAlpha = 255;
  CollisionEnabled = true;

  // Enable collisions, widget drawing, and ticking
  SetActorEnableCollision(true);
  PrimaryActorTick.bCanEverTick = true;
  SetActorTickEnabled(true);

  // If this is a derived class of CigiEntity (aka if this is a Blueprint), do NOT setup any JSON components
  bool isBP = GetClass()->GetSuperClass()->IsChildOf<ACigiEntity>();
  if (!isBP)
  {
    // This code only runs for the base class CigiEntity, where meshes are setup so that they can be assigned from JSON-specified assets
    USceneComponent* SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Generated Root"));
    SetRootComponent(SceneRootComponent);
  }
}

USceneComponent* ACigiEntity::GetArticulatedPart(int32 ArticulatedPartID)
{
  // Search the TMap of parts for the given ID
  USceneComponent** result = ArticulatedParts.Find(ArticulatedPartID);

  // If no entry was found for that ID, return null
  if (result == nullptr)
  {
    return nullptr;
  }

  // If a match was found, return the component. If the component is null, this will return null.
  return *result;
}

void ACigiEntity::BeginPlay()
{
  // Make sure that Blueprints receive their BeginPlay event
  Super::BeginPlay();
  PrimaryActorTick.bCanEverTick = true;
}

void ACigiEntity::Tick(float deltaSeconds)
{
  // Make sure that Blueprints receive their Tick event
  Super::Tick(deltaSeconds);

  // If host collisions are enabled for this entity, check its collision segments (if any)
  if (CollisionEnabled)
  {
    for (const TPair<int32, FCollisionSegment>& pairSeg : CollisionSegments)
    {
      if (!pairSeg.Value.enable)
      {
        continue;
      }

      // Transform the segment positions into this actor's local space so they pivot around the actor when it rotates
      FTransform entityTransform = GetActorTransform();
      FVector start = UKismetMathLibrary::TransformLocation(entityTransform, pairSeg.Value.start);
      FVector end = UKismetMathLibrary::TransformLocation(entityTransform, pairSeg.Value.end);

      TArray<FHitResult> hits;
      FCollisionQueryParams params;
      params.AddIgnoredActor(this);
      GetWorld()->LineTraceMultiByObjectType(hits, start, end, FCollisionObjectQueryParams::AllObjects, params);
      DebugLine(GetWorld(), start, end, FColor::Purple, 0.05);
      UE_LOG(LogCigiEntity, ENTITY_TICK, TEXT("\"%s\": Tick: Segment Collision: Tracing from %s to %sS, found %d hits"), *GetName(), *VectorString(start), *VectorString(end), hits.Num());

      for (const FHitResult& hit : hits)
      {
        ACigiEntity* entity = Cast<ACigiEntity>(hit.GetActor());
        bool colType = IsValid(entity);

        if (!FUnrealCigi_PluginModule::globals.pImageGenerator || !FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher)
        {
          UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": Tick: Segment Collision: FAILED: Cannot send response because g_CigiLibGlobals->pImageGenerator is NULL!"), *GetName());
          break;
        }

        sbio::cigi::SCollisionDetectionSegmentNotification data;
        data.entityID = sbio::EntityID(EntityID);
        data.segmentID = sbio::SegmentID(pairSeg.Key);
        data.materialCode = sbio::MaterialID(0);
        data.fIntersectionDistance = hit.Distance / 100.0;

        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendCollisionDetectionSegmentNotification(data);

        UE_LOG(LogCigiEntity, ENTITY_TICK, TEXT("\"%s\": Tick: Segment Collision: EntityID=%d, SegID=%d, ContactEntityID=%d, ColType=%d, Distance=%.2f"), *GetName(), EntityID, pairSeg.Key, colType ? entity->EntityID : -1, colType, hit.Distance / 100.0);
      }
    }

    for (TPair<int32, UShapeComponent*> pairVol : CollisionVolumes)
    {

      if (pairVol.Value == nullptr)
      {
        continue;
      }

      if (DrawDebugVolumes)
      {
        FVector volPos = pairVol.Value->GetComponentLocation();
        FVector volScale = pairVol.Value->GetComponentScale();
        FQuat volRot = pairVol.Value->GetComponentRotation().Quaternion();
        bool volEnable = pairVol.Value->GetCollisionEnabled() == ECollisionEnabled::QueryOnly;
        FColor volColor = volEnable ? FColor::Blue : FColor::Yellow;
        if (Cast<USphereComponent>(pairVol.Value) != nullptr)
        {
          if (ENABLE_DBG_DRAW)
          {
            DrawDebugSphere(GetWorld(), volPos, volScale.GetAbsMax(), 16, volColor, false, deltaSeconds * 2.0, 0, 4);
          }
        }
        else
        {
          if (ENABLE_DBG_DRAW)
          {
            DrawDebugBox(GetWorld(), volPos, volScale, volRot, volColor, false, deltaSeconds * 2.0, 0, 4);
          }
        }
      }

      if (!pairVol.Value->GetCollisionEnabled())
      {
        continue;
      }

      TArray<UPrimitiveComponent*> overlapComps = TArray<UPrimitiveComponent*>();
      pairVol.Value->GetOverlappingComponents(overlapComps);

      for (UPrimitiveComponent* overlapComp : overlapComps)
      {
        if (overlapComp == nullptr || overlapComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
          continue;
        }
        ACigiEntity* overlapEntity = Cast<ACigiEntity>(overlapComp->GetAttachmentRootActor());
        if (overlapEntity == nullptr || overlapEntity == this)
        {
          continue;
        }
        int32 overlapVolID = overlapEntity->FindCollisionVolume(overlapComp);
        if (overlapVolID < 0)
        {
          continue;
        }

        if (!FUnrealCigi_PluginModule::globals.pImageGenerator || !FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher)
        {
          UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": Tick: Segment Collision: FAILED: Cannot send response because g_CigiLibGlobals->pImageGenerator is NULL!"), *GetName());
          break;
        }

        if (DrawDebugVolumes)
        {
          // (This is broken out into variables for debug step-through)
          FVector overlapPos = overlapComp->GetComponentLocation();
          FVector overlapScale = overlapComp->GetComponentScale();
          FQuat overlapRot = overlapComp->GetComponentRotation().Quaternion();
          FColor overlapColor = FColor::Red;
          if (Cast<USphereComponent>(pairVol.Value) != nullptr)
          {
            DrawDebugSphere(GetWorld(), overlapPos, overlapScale.GetAbsMax(), 16, overlapColor, false, deltaSeconds * 2.0, 0, 8);
          }
          else
          {
            DrawDebugBox(GetWorld(), overlapPos, overlapScale, overlapRot, overlapColor, false, deltaSeconds * 2.0, 0, 8);
          }
        }

        sbio::cigi::SCollisionDetectionVolumeEntityNotification data;
        data.entityID = sbio::EntityID(EntityID);
        data.volumeID = sbio::VolumeID(pairVol.Key);
        data.contactedVolumeID = sbio::VolumeID(overlapVolID);
        data.contactedEntityID = sbio::EntityID(overlapEntity->EntityID);
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendCollisionDetectionVolumeEntityNotification(data);

        UE_LOG(LogCigiEntity, ENTITY_TICK, TEXT("\"%s\": Tick: Volume Collision: EntityID=%d,VolID=%d reporting collision with EntityID=%d,VolID=%d"), *GetName(), EntityID, pairVol.Key, overlapEntity->EntityID, overlapVolID);
      }
    }
  }

  ACigiView* view = nullptr;
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler != nullptr)
  {
    view = nullptr;
    if (FUnrealCigi_PluginModule::globals.pUnrealViewManager != nullptr)
    {
      view = FUnrealCigi_PluginModule::globals.pUnrealViewManager->First();
    }
  }

  // If there are billboard widgets attached to this entity, have them track the camera
  if (IsValid(view))
  {
    for (const TPair<int32, bool>& pair : IsBillboard)
    {
      // If the billboard tag is marked as False, skip this surface
      if (!pair.Value)
      {
        continue;
      }

      // Find the widget component for this surface. If one cannot be found, skip it
      UWidgetComponent** p_widgetComp = Widgets.Find(pair.Key);
      UWidgetComponent* widgetComp = NULL;

      if (p_widgetComp != NULL)
      {
        widgetComp = *p_widgetComp;
      }

      if (widgetComp == NULL)
      {
        continue;
      }

      // Rotate this widget so that it is always parallel to the view plane (appears "flat" on the screen)
      const FVector oldRot = FUEWorldRotation::From(view->GetActorRotation()).ToFRotator().Euler();
      FVector newRot = FVector(-oldRot.X, -oldRot.Y, oldRot.Z + 180.0f);
      widgetComp->SetWorldRotation(FQuat::MakeFromEuler(newRot));
    }
  }
}

void ACigiEntity::SetVisual(const TCHAR* path, FTransform transform, bool articulateBones)
{
  // If the path is intentionally marked as "none", do not render a visual for this entity
  // (empty entities are useful for positioning other elements like views, symbols, or child entities)
  if (FString(path).Equals(TEXT("none"), ESearchCase::IgnoreCase))
  {
    UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": SetVisual: Filepath string was 'none', spawning blank entity with no mesh component"), *GetName());
    return;
  }

  // Try to read path as a StaticMesh asset
  UStaticMesh* staticMesh = LoadObject<UStaticMesh>(nullptr, path);
  if (staticMesh != nullptr)
  {
    StaticMeshComponent = NewObject<UStaticMeshComponent>(this, "Generated StaticMeshComp");
    StaticMeshComponent->RegisterComponent();
    StaticMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
    StaticMeshComponent->SetSimulatePhysics(false);
    StaticMeshComponent->SetStaticMesh(staticMesh);
    StaticMeshComponent->SetRelativeTransform(transform);
    StaticMeshComponent->SetCollisionEnabled(COLLISION_ENABLE);
    StaticMeshComponent->SetCollisionObjectType(COLLISION_CHANNEL);
    StaticMeshComponent->SetCollisionResponseToAllChannels(COLLISION_RESPONSE);
    UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": SetVisual: Created new Static Mesh \"%s\" from path \"%s\""), *GetName(), *(StaticMeshComponent->GetName()), path);
    return;
  }

  // Try to read path as a SkeletalMesh asset
  USkeletalMesh* skeletalMesh = LoadObject<USkeletalMesh>(nullptr, path);
  if (skeletalMesh != nullptr)
  {
    // If articulateBones, then the ArticulatedMesh is the main visible component and an invisible SkeletalMeshComponent must also be created for collisions
    if (articulateBones)
    {
      ArticulatedMesh = NewObject<UPoseableMeshComponent>(this, "Generated ArticulatedMesh");
      ArticulatedMesh->RegisterComponent();
      ArticulatedMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
      ArticulatedMesh->SetSimulatePhysics(false);
#if ENGINE_MAJOR_VERSION < 5
      ArticulatedMesh->SetSkeletalMesh(skeletalMesh);
#else
      ArticulatedMesh->SetSkinnedAsset(skeletalMesh);
#endif
      ArticulatedMesh->SetRelativeTransform(transform);

      // Ensure that the mesh is visible
      ArticulatedMesh->SetHiddenInGame(false);
      ArticulatedMesh->SetVisibility(true);
      // Force the bone transforms to refresh (fixes bug where the bones are sometimes in the wrong position)
      ArticulatedMesh->AllocateTransformData();
      // Force the mesh to re-render itself (fixes bug where the mesh is sometimes invisible)
      ArticulatedMesh->MarkRenderStateDirty();

      UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": SetVisual: Created new Poseable Mesh \"%s\" from path \"%s\""), *GetName(), *(ArticulatedMesh->GetName()), path);
    }

    SkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this, articulateBones ? "Generated Collision for ArticulatedMesh (JSON)" : "Generated SkeletalMeshComp");
    SkeletalMeshComponent->RegisterComponent();
    SkeletalMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
    SkeletalMeshComponent->SetRelativeTransform(transform);
    SkeletalMeshComponent->SetCollisionEnabled(COLLISION_ENABLE);
    SkeletalMeshComponent->SetCollisionObjectType(COLLISION_CHANNEL);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(COLLISION_RESPONSE);
    SkeletalMeshComponent->SetHiddenInGame(articulateBones);
    if (articulateBones && IsValid(ArticulatedMesh))
    {
      SkeletalMeshComponent->SetLeaderPoseComponent(ArticulatedMesh, true, false);
    }

    UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": SetVisual: Created new Skeletal Mesh \"%s\" from path \"%s\""), *GetName(), *(SkeletalMeshComponent->GetName()), path);
    return;
  }

  // FAILED: Asset doesn't exist or isn't a valid type
  UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": SetVisual: FAILED: Asset \"%s\" doesn't exist or is not a valid type (StaticMesh, SkeletalMesh)"), *GetName(), path);
}

void ACigiEntity::AddArticulatedPart(ArticulatedPartID id, FVector origin, const TCHAR* path, FTransform transform)
{
  if (ArticulatedParts.Contains(id.Value()) || ArticulatedBones.Contains(id.Value()))
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": AddArticulatedPart: FAILED: id %d is a duplicate id!"), *GetName(), id.Value());
    return;
  }

  // Create the origin SceneComponent: (must be separate from the main AP comp b/c the main comp's transform is overwritten)
  USceneComponent* originComp = NewObject<USceneComponent>(this, *FString::Printf(TEXT("Generated ArticulatedPart%d OriginComponent"), id.Value()));
  originComp->RegisterComponent();
  originComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
  originComp->SetRelativeLocation(origin);

  // Create the main AP SceneComponent:
  USceneComponent* sceneComp = NewObject<USceneComponent>(this, *FString::Printf(TEXT("Generated ArticulatedPart%d SceneComponent"), id.Value()));
  sceneComp->RegisterComponent();
  sceneComp->AttachToComponent(originComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
  ArticulatedParts.Add(id.Value(), sceneComp);
  UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": AddArticulatedPart: Created new AP with id=%d, origin=\"%s\", sceneComp=\"%s\""), *GetName(), id.Value(), *origin.ToString(), *(sceneComp->GetName()));

  // Try to read path as a StaticMesh asset
  UStaticMesh* staticMesh = LoadObject<UStaticMesh>(nullptr, path);
  if (staticMesh != nullptr)
  {
    UStaticMeshComponent* staticMeshComp = NewObject<UStaticMeshComponent>(this, *FString::Printf(TEXT("Generated ArticulatedPart%d StaticMeshComp"), id.Value()));
    staticMeshComp->RegisterComponent();
    staticMeshComp->AttachToComponent(sceneComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
    staticMeshComp->SetSimulatePhysics(false);
    staticMeshComp->SetStaticMesh(staticMesh);
    staticMeshComp->SetRelativeTransform(transform);
    UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": AddArticulatedPart: id %d, Added Static Mesh \"%s\" from path \"%s\" with transform \"%s\" and component \"%s\""), *GetName(), id.Value(), *(staticMesh->GetName()), path, *transform.ToString(), *ObjName(staticMeshComp));
    return;
  }
  // Try to read path as a SkeletalMesh asset
  USkeletalMesh* skeletalMesh = LoadObject<USkeletalMesh>(nullptr, path);
  if (skeletalMesh != nullptr)
  {
    USkeletalMeshComponent* skeletalMeshComp = NewObject<USkeletalMeshComponent>(this, *FString::Printf(TEXT("Generated ArticulatedPart%d SkeletalMeshComp"), id.Value()));
    skeletalMeshComp->RegisterComponent();
    skeletalMeshComp->AttachToComponent(sceneComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
    skeletalMeshComp->SetSimulatePhysics(false);
    skeletalMeshComp->SetSkeletalMesh(skeletalMesh);
    skeletalMeshComp->SetRelativeTransform(transform);
    UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": AddArticulatedPart: id %d, Added Skeletal Mesh \"%s\" from path \"%s\" with transform \"%s\" and component \"%s\""), *GetName(), id.Value(), *(skeletalMesh->GetName()), path, *transform.ToString(), *ObjName(skeletalMeshComp));
    return;
  }

  // FAILED: Asset doesn't exist or isn't a valid type
  UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": AddArticulatedPart: FAILED: Asset \"%s\" doesn't exist or is not a valid type (StaticMesh, SkeletalMesh)"), *GetName(), path);
}

void ACigiEntity::AddArticulatedBones(TMap<FString, int32> articulatedBoneNames)
{
  if (!IsValid(ArticulatedMesh))
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": AddArticulatedBones: FAILED: There is no Articulated Mesh!"), *GetName());
    return;
  }
  if (articulatedBoneNames.Num() <= 0)
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": AddArticulatedBones: FAILED: No articulated bone names were given!"), *GetName());
    return;
  }

  TArray<FName> boneNames = TArray<FName>();
  ArticulatedMesh->GetBoneNames(boneNames);
  for (FName name : boneNames)
  {
    int32* ptr_id = articulatedBoneNames.Find(name.ToString());
    if (ptr_id == nullptr)
    {
      UE_LOG(LogCigiEntity, Verbose, TEXT("\"%s\": FindArticulatedBones: bone \"%s\" does not match an Articulated Bone Name"), *GetName(), *name.ToString());
      continue;
    }

    if (ArticulatedParts.Contains(*ptr_id) || ArticulatedBones.Contains(*ptr_id))
    {
      // If the bone ID is already registered, do not register it again
      UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": FindArticulatedBones: Could not register bone \"%s\" because its id %d is a duplicate!"), *GetName(), *name.ToString(), *ptr_id);
    }
    else
    {
      FTransform boneTransWorld = ArticulatedMesh->GetBoneTransformByName(name, EBoneSpaces::WorldSpace);
      // Convert transform from world space to local space
      FTransform boneTransRelative = FTransform(GetActorTransform().InverseTransformRotation(boneTransWorld.GetRotation()), GetActorTransform().InverseTransformPosition(boneTransWorld.GetLocation()), FVector(1, 1, 1));
      FArticulatedBoneData boneData = FArticulatedBoneData();
      boneData.boneName = name;
      boneData.originTransform = boneTransRelative;
      ArticulatedBones.Add(*ptr_id, boneData);
      UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": FindArticulatedBones: Registered bone with id=%d, name=\"%s\", and origin=\"%s\""), *GetName(), *ptr_id, *boneData.boneName.ToString(), *boneData.originTransform.ToString());
    }
  }
}

void ACigiEntity::FindArticulatedParts()
{
  bool findParts = ArticulatedPartNames.Num() > 0;
  bool findBones = !ArticulatedMeshName.IsEmpty() && ArticulatedBoneNames.Num() > 0;

  // If there are no parts or bones to find, don't waste time scanning through components
  if (!findParts && !findBones)
  {
    return;
  }

  // Scan all of this actor's components for the articulated mesh and/or any articulated parts
  TSet<UActorComponent*> comps = GetComponents();
  for (UActorComponent* comp : comps)
  {
    if (comp == nullptr)
    {
      continue;
    }

    USceneComponent* sceneComp = Cast<USceneComponent>(comp);
    if (!IsValid(sceneComp))
    {
      continue;
    }

    // Check if this component is the Articulated Mesh
    if (findBones && ArticulatedMesh == nullptr)
    {
      if (sceneComp->GetName().Equals(ArticulatedMeshName))
      {
        UPoseableMeshComponent* poseMesh = Cast<UPoseableMeshComponent>(sceneComp);
        if (!IsValid(poseMesh))
        {
          UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": FindArticulatedParts: Failed to register Articulated Mesh because component \"%s\" is not a Poseable Mesh!"), *GetName(), *ObjName(sceneComp));
        }
        else
        {
          ArticulatedMesh = poseMesh;

          UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": FindArticulatedParts: Registered Articulated Mesh component \"%s\""), *GetName(), *ObjName(sceneComp));
        }
      }
    }

    // Check if this component is an Articulated Part
    if (findParts)
    {
      int32* ptr_id = ArticulatedPartNames.Find(sceneComp->GetName());
      if (ptr_id == nullptr)
      {
        UE_LOG(LogCigiEntity, Verbose, TEXT("\"%s\": FindArticulatedParts: scene component \"%s\" does not match an Articulated Part Name"), *GetName(), *ObjName(sceneComp));
      }
      else if (ArticulatedParts.Contains(*ptr_id) || ArticulatedBones.Contains(*ptr_id))
      {
        UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": FindArticulatedParts: Could not register scene component \"%s\" because its id %d is a duplicate!"), *GetName(), *ObjName(sceneComp), *ptr_id);
      }
      else
      {
        ArticulatedParts.Add(*ptr_id, sceneComp);
        UE_LOG(LogCigiEntity, ENTITY_LOG, TEXT("\"%s\": FindArticulatedParts: Registered scene component \"%s\" with id %d"), *GetName(), *ObjName(sceneComp), *ptr_id);
      }
    }
  }

  if (findBones && IsValid(ArticulatedMesh))
  {
    // Create a duplicate, invisible Skeletal Mesh for collision purposes
    SkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this, "Generated Collision for ArticulatedMesh (BP)");
    SkeletalMeshComponent->RegisterComponent();
    SkeletalMeshComponent->AttachToComponent(ArticulatedMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetCollisionEnabled(COLLISION_ENABLE);
    SkeletalMeshComponent->SetCollisionObjectType(COLLISION_CHANNEL);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(COLLISION_RESPONSE);
    SkeletalMeshComponent->SetHiddenInGame(true);

#if ENGINE_MAJOR_VERSION < 5
    // UE4: UPoseableMeshComponent has a SkeletalMesh, just use that
    SkeletalMeshComponent->SetSkeletalMesh(ArticulatedMesh->SkeletalMesh);
#else
    // UE5: UPoseableMeshComponent might or might not have a SkeletalMesh, try to get one if possible
    USkinnedAsset* sa = ArticulatedMesh->GetSkinnedAsset();
    USkeletalMesh* sm = Cast<USkeletalMesh>(sa);

    if (IsValid(sm))
    {
      SkeletalMeshComponent->SetSkeletalMeshAsset(sm);
    }
    else
    {
      SkeletalMeshComponent->SetSkinnedAsset(sa);
    }
#endif

    SkeletalMeshComponent->SetLeaderPoseComponent(ArticulatedMesh, true, false);

    // Register Articulated Bones
    AddArticulatedBones(ArticulatedBoneNames);
  }
}

void ACigiEntity::SetEnabled(bool enabled)
{
  // Somehow there was a nullptr crash here. This if block will hopefully prevent future crashes
  if (this != nullptr && IsValid(this))
  {
    SetActorTickEnabled(enabled);
    SetActorHiddenInGame(!enabled);
    SetActorEnableCollision(enabled);
  }
}

void ACigiEntity::SetEngineLocation(const sbio::unrealcigi::FUEWorldCoordinates& location)
{
  SetActorLocation(location.ToFVector());
}

FUEWorldCoordinates ACigiEntity::GetEngineLocation() const
{
  return FUEWorldCoordinates::From(GetActorLocation());
}

void ACigiEntity::SetEngineRotation(const sbio::unrealcigi::FUEWorldRotation& rotation)
{
  SetActorRotation(rotation.ToFQuat());
}

FUEWorldRotation ACigiEntity::GetEngineRotation() const
{
  return FUEWorldRotation::From(GetActorRotation());
}

void ACigiEntity::SetArticulatedPartEnabled(ArticulatedPartID apID, bool enabled)
{
  USceneComponent** ptr_ap = ArticulatedParts.Find(apID.Value());
  if (ptr_ap != nullptr)
  {
    if (!IsValid(*ptr_ap))
    {
      UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": SetAPEnabled: Articulated Part with ID=%d is null or otherwise invalid!"), *GetName(), apID.Value());
    }
    else
    {
      // Enable/Disable the SceneComponent at the root of the AP
      if (enabled)
      {
        (*ptr_ap)->Activate();
      }
      else
      {
        (*ptr_ap)->Deactivate();
      }

      (*ptr_ap)->SetComponentTickEnabled(enabled);
      (*ptr_ap)->SetHiddenInGame(!enabled);

      // Enable/Disable all meshes, particles, etc. that are inside of this AP
      TArray<USceneComponent*> children = TArray<USceneComponent*>();
      (*ptr_ap)->GetChildrenComponents(false, children);
      for (USceneComponent* child : children)
      {
        if (!IsValid(child))
        {
          continue;
        }

        if (enabled)
        {
          child->Activate();
        }
        else
        {
          child->Deactivate();
        }

        child->SetComponentTickEnabled(enabled);
        child->SetHiddenInGame(!enabled);
      }
    }
  }
  else
  {
    auto ptr_ab = ArticulatedBones.Find(apID.Value());
    if (ptr_ab == nullptr || !(*ptr_ab).HasName() || !IsValid(ArticulatedMesh))
    {
      UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": SetAPEnabled: Could not find Articulated Part or Bone with ID=%d"), *GetName(), apID.Value());
    }
    else
    {
      // Enable/Disable the articulated bone
      if (enabled)
      {
        ArticulatedMesh->UnHideBoneByName((*ptr_ab).boneName);
      }
      else
      {
        ArticulatedMesh->HideBoneByName((*ptr_ab).boneName, EPhysBodyOp::PBO_None);
      }
    }
  }
}

void ACigiEntity::UpdateArticulatedPartTransform(ArticulatedPartID apID, FTransform transform)
{
  USceneComponent** ptr_ap = ArticulatedParts.Find(apID.Value());
  if (ptr_ap != nullptr)
  {
    if (!IsValid(*ptr_ap))
    {
      UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": UpdateAPTransform: Articulated Part with ID=%d is null or otherwise invalid!"), *GetName(), apID.Value());
    }
    else
    {
      (*ptr_ap)->SetRelativeTransform(transform);
      UE_LOG(LogCigiEntity, CIGI_VELOCITY, TEXT("\"%s\": UpdateAPTransform: id=%d, ArtPart=\"%s\", new relative transform=\"%s\""), *GetName(), apID.Value(), *(*ptr_ap)->GetName(), *(*ptr_ap)->GetRelativeTransform().ToString());
    }
  }
  else
  {
    auto ptr_ab = ArticulatedBones.Find(apID.Value());
    if (ptr_ab == nullptr || !(*ptr_ab).HasName() || !IsValid(ArticulatedMesh))
    {
      UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": UpdateAPTransform: Could not find Articulated Part or Bone with ID=%d"), *GetName(), apID.Value());
    }
    else
    {
      // Apply the input transform to the origin transform, in local space
      FTransform relative = FTransform(transform.GetRotation() * (*ptr_ab).originTransform.GetRotation(), (*ptr_ab).originTransform.GetLocation() + transform.GetLocation(), FVector(1, 1, 1));
      // Convert the transform from local space to world space
      FTransform worldTrans = FTransform(GetActorTransform().TransformRotation(relative.GetRotation()), GetActorTransform().TransformPosition(relative.GetLocation()), FVector(1, 1, 1));
      // Apply the transform to the component in world space
      ArticulatedMesh->SetBoneTransformByName((*ptr_ab).boneName, worldTrans, EBoneSpaces::WorldSpace);

      UE_LOG(LogCigiEntity, Verbose, TEXT("\"%s\": UpdateAPTransform: id=%d, ArtBone=\"%s\", RELATIVE=\"%s\", BONE=\"%s\""), *GetName(), apID.Value(), *(*ptr_ab).boneName.ToString(), *relative.GetRotation().Euler().ToString(),
             *ArticulatedMesh->GetBoneTransformByName((*ptr_ab).boneName, EBoneSpaces::WorldSpace).GetRotation().Euler().ToString());
    }
  }
}

bool ACigiEntity::CreateCollisionSegment(SegmentID segID)
{
  if (CollisionSegments.Contains(segID.Value()))
  {
    return false;
  }

  CollisionSegments.Add(segID.Value(), FCollisionSegment());
  return true;
}

bool ACigiEntity::UpdateCollisionSegment(SegmentID segID, FVector start, FVector end)
{
  FCollisionSegment* p_seg = CollisionSegments.Find(segID.Value());
  if (!p_seg)
  {
    return false;
  }

  p_seg->start = start;
  p_seg->end = end;
  return true;
}

bool ACigiEntity::UpdateCollisionSegment(SegmentID segID, bool enabled)
{
  FCollisionSegment* p_seg = CollisionSegments.Find(segID.Value());
  if (!p_seg)
  {
    return false;
  }

  p_seg->enable = enabled;
  return true;
}

bool ACigiEntity::CreateCollisionVolume(VolumeID volID, bool sphere)
{
  // Check if the volume already exists in the map
  UShapeComponent** p_shape = CollisionVolumes.Find(volID.Value());
  if (p_shape != nullptr)
  {
    return false;
  }

  // Create a new UShapeComponent (either a USphereComponent or UBoxComponent) based on the 'sphere' parameter
  UShapeComponent* shape = nullptr;
  if (sphere)
  {
    shape = NewObject<USphereComponent>(this, *FString::Printf(TEXT("Generated Collision Sphere id=%d"), volID.Value()));
  }
  else
  {
    shape = NewObject<UBoxComponent>(this, *FString::Printf(TEXT("Generated Collision Box id=%d"), volID.Value()));
  }

  // Register the shape component and attach it to the root component
  shape->RegisterComponent();
  shape->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
  shape->SetRelativeTransform(FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector::One()));
  shape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  shape->SetGenerateOverlapEvents(true);
  shape->SetCollisionObjectType(COLLISION_CHANNEL);
  shape->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  shape->SetCollisionResponseToChannel(COLLISION_CHANNEL, ECollisionResponse::ECR_Overlap);
  CollisionVolumes.Add(volID.Value(), shape);
  return true;
}

bool ACigiEntity::SetCollisionVolumeEnabled(VolumeID volID, bool enabled)
{
  // Check if the volume exists in the map
  UShapeComponent** p_shape = CollisionVolumes.Find(volID.Value());
  if (p_shape == nullptr)
  {
    return false;
  }

  // Get the UShapeComponent for this volume ID
  UShapeComponent* shape = *p_shape;
  if (shape == nullptr)
  {
    return false;
  }

  // Set the collision enabled state of the shape component based on the provided boolean
  shape->SetCollisionEnabled(enabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
  return true;
}

bool ACigiEntity::SetCollisionVolumeOffset(VolumeID volID, FVector offset)
{
  // Check if the volume exists in the map
  UShapeComponent** p_shape = CollisionVolumes.Find(volID.Value());
  if (p_shape == nullptr)
  {
    return false;
  }

  // Get the UShapeComponent for this volume ID
  UShapeComponent* shape = *p_shape;
  if (shape == nullptr)
  {
    return false;
  }

  // Set the relative location of the shape component using the provided offset
  shape->SetRelativeLocation(offset);
  return true;
}

bool ACigiEntity::SetCollisionVolumeRotation(VolumeID volID, FQuat rot)
{
  // Check if the volume exists in the map
  UShapeComponent** p_shape = CollisionVolumes.Find(volID.Value());
  if (p_shape == nullptr)
  {
    return false;
  }

  // Get the UShapeComponent for this volume ID
  UShapeComponent* shape = *p_shape;
  if (shape == nullptr)
  {
    return false;
  }

  // Set the relative rotation of the shape component using the provided quaternion
  shape->SetRelativeRotation(rot);
  return true;
}

bool ACigiEntity::SetCollisionVolumeSize(VolumeID volID, FVector size)
{
  // Check if the volume exists in the map
  UShapeComponent** p_shape = CollisionVolumes.Find(volID.Value());
  if (p_shape == nullptr)
  {
    return false;
  }

  // Get the UShapeComponent for this volume ID
  UShapeComponent* shape = *p_shape;
  if (shape == nullptr)
  {
    return false;
  }

  // For sphere components, the size is specified as the full extents, but Unreal uses radius, so we take the max of the three axes
  if (USphereComponent* sphere = Cast<USphereComponent>(shape))
  {
    sphere->SetSphereRadius(size.GetAbsMax());
    return true;
  }

  // For box components, the size is specified as the full extents, but Unreal uses half extents, so we divide by 2
  if (UBoxComponent* box = Cast<UBoxComponent>(shape))
  {
    box->SetBoxExtent(size * 0.5f);
    return true;
  }

  return true;
}

bool ACigiEntity::DestroyCollisionVolume(VolumeID volID)
{
  // Check if the volume exists in the map
  UShapeComponent** p_shape = CollisionVolumes.Find(volID.Value());
  if (p_shape == nullptr)
  {
    return false;
  }

  // Destroy the UShapeComponent
  UShapeComponent* shape = *p_shape;
  if (shape != nullptr)
  {
    shape->DestroyComponent();
  }

  // Remove the volume from the map
  CollisionVolumes.Remove(volID.Value());
  return true;
}

int32 ACigiEntity::FindCollisionVolume(UPrimitiveComponent* volume)
{
  // If the given UPrimitiveComponent is null, return -1
  if (volume == nullptr)
  {
    return -1;
  }

  // Search through the CollisionVolumes map to find the volume ID corresponding to the given UPrimitiveComponent
  for (TPair<int32, UShapeComponent*> pair : CollisionVolumes)
  {
    if (volume == pair.Value)
    {
      return pair.Key;
    }
  }

  // If the volume was not found, return -1
  return -1;
}

void ACigiEntity::UpdateWidgetComponent(SymbolSurfaceID surfaceID)
{
  // Make sure the given surfaceID is valid and has a valid Widget and WidgetType
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler == nullptr)
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": UpdateWidget: FAILED: EventHandler does not exist!"), *GetName());
    return;
  }

  // Make sure the UnrealSymbolManager exists
  FUnrealSymbolSurface* surface = nullptr;
  if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
  {
    surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSurface(surfaceID);
  }

  // If the surface does not exist, log a warning and return
  if (!surface)
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": UpdateWidget: FAILED: surface %d does not exist!"), *GetName(), surfaceID.Value());
    return;
  }

  // Make sure the surface has a valid Widget
  if (!surface->Widget)
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": UpdateWidget: FAILED: surface %d does not have a widget!"), *GetName(), surfaceID.Value());
    return;
  }

  // Make sure the surface has a valid WidgetType
  if (surface->WidgetType == SymbolSurfaceType::UNKNOWN)
  {
    UE_LOG(LogCigiEntity, ENTITY_WARNING, TEXT("\"%s\": UpdateWidget: FAILED: surface %d has an invalid WidgetType!"), *GetName(), surfaceID.Value());
    return;
  }

  // Make a new WidgetComponent for this ID if one does not already exist OR if the existing one is invalid
  UWidgetComponent** p_widgetComp = Widgets.Find(surfaceID.Value());
  UWidgetComponent* widgetComp = NULL;
  if (p_widgetComp)
  {
    widgetComp = *p_widgetComp;
  }

  // If the widget component is invalid, remove it from the map and create a new one
  if (!IsValid(widgetComp))
  {
    if (p_widgetComp)
    {
      Widgets.FindAndRemoveChecked(surfaceID.Value());
    }

    // Create a new WidgetComponent for this surface
    widgetComp = NewObject<UWidgetComponent>(this, *FString::Printf(TEXT("Widget - SymbolSurface %d"), surfaceID.Value()));
    widgetComp->RegisterComponent();
    widgetComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
    widgetComp->SetWidget(surface->Widget);

    Widgets.Add(surfaceID.Value(), widgetComp);
  }

  // If the widget has changed, update the WidgetComponent to use the new widget
  if (widgetComp->GetWidget() != surface->Widget)
  {
    widgetComp->SetWidget(surface->Widget);
  }

  // Transform logic for world surfaces:
  if (surface->WidgetType == SymbolSurfaceType::WORLD || surface->WidgetType == SymbolSurfaceType::BILLBOARD)
  {
    // Set the width and height, converting to unreal units (meters to centimeters)
    // HostInput(1,2) -> SimulationSDK(1,2) -> Unreal(100,200)=CIGI(1,2)
    widgetComp->SetDrawSize((surface->Size * 100.0).ClampAxes(1, 10000));

    // Billboard surfaces are always parallel to the view plane (this is handled in Tick)
    bool newBillboardValue = surface->WidgetType == SymbolSurfaceType::BILLBOARD;
    bool* p_billboard = IsBillboard.Find(surfaceID.Value());

    // If the billboard value has changed, update it
    if (p_billboard == nullptr)
    {
      IsBillboard.Add(surfaceID.Value(), newBillboardValue);
    }
    else
    {
      (*p_billboard) = newBillboardValue;
    }

    // Set the location and rotation, converting CIGI axes into Unreal axes
    // (Billboard and World surfaces have different CIGI axes)
    FVector oldOffset = surface->Offset;
    if (newBillboardValue)
    {
      // From entity's POV: CIGI-Billboard (X+ is left, Y+ is up, Z+ is forwards) VS Unreal (X+ is forwards, Y+ is right, Z+ is up)
      // HostInput(1,2,3) -> SimulationSDK(-100,-200,300) -> Unreal(300, -100, 200)=CIGI(1,2,3)
      // The first '->' conversion is done in simulation sdk, the second '->' is being done here
      FVector billboardOffset = FVector(oldOffset.Z, -oldOffset.X, oldOffset.Y);

      // If there is a valid view, rotate the billboard offset vector so that it is parallel to the view plane (according to CIGI)
      ACigiView* view = nullptr;
      if (FUnrealCigi_PluginModule::globals.pUnrealViewManager != nullptr)
      {
        view = FUnrealCigi_PluginModule::globals.pUnrealViewManager->First();
      }

      // If there is a valid view, rotate the billboard offset vector so that it is parallel to the view plane (according to CIGI)
      if (view != nullptr)
      {
        const FVector oldRot = FUEWorldRotation::From(view->GetActorRotation()).ToFRotator().Euler();
        FVector newRot = FVector(oldRot.X, oldRot.Y, oldRot.Z + 180.0f);
        billboardOffset = FQuat::MakeFromEuler(newRot).RotateVector(billboardOffset);
      }

      // Set the location of the widget to the rotated offset
      widgetComp->SetRelativeLocation(billboardOffset);
    }
    else
    {
      // From entity's POV: CIGI-NonBillboard (X+ is forwards, Y+ is right, Z+ is down) VS Unreal (X+ is forwards, Y+ is right, Z+ is up)
      // HostInput(1,2,3) -> SimulationSDK(100,200,-300) -> Unreal(100, 200, -300)=CIGI(1,2,3)
      // The first '->' conversion is done in simulation sdk, and there is no need for a second conversion
      FVector nonBillboardOffset = FVector(oldOffset.X, oldOffset.Y, oldOffset.Z);
      widgetComp->SetRelativeLocation(nonBillboardOffset);
      
      // Set the rotation of the widget to match the surface's rotation, converting CIGI axes into Unreal axes
      FVector oldRotation = surface->Rotation.Euler();
      FVector newRotation = FVector(oldRotation.X, oldRotation.Y, oldRotation.Z);
      widgetComp->SetRelativeRotation(FQuat::MakeFromEuler(newRotation));
    }
  }
}

void ACigiEntity::RemoveWidgetComponent(SymbolSurfaceID surfaceID)
{
  // Find the widget component for this surface. If one cannot be found, skip it
  UWidgetComponent** p_widgetComp = Widgets.Find(surfaceID.Value());
  if (!p_widgetComp)
  {
    IsBillboard.Remove(surfaceID.Value());
    return;
  }

  // Remove the widget component from the entity
  UWidgetComponent* widgetComp = *p_widgetComp;
  Widgets.FindAndRemoveChecked(surfaceID.Value());
  IsBillboard.Remove(surfaceID.Value());

  if (!IsValid(widgetComp))
  {
    return;
  }

  // Destroy the widget component, but don't destroy the underlying widget (the widget is owned by the symbol surface)
  widgetComp->DestroyComponent(false);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026