//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiSymbolEventHandler.h"

#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiSymbolManager.h"
#include "UnrealCigiComponentDispatcher.h"
#include "CigiController.h"
#include "SymbolConfig.h"
#include "CigiSymbol.h"
#include "SymbolLib/SymbolCircle.h"
#include "SymbolLib/SymbolPolygon.h"
#include "SymbolLib/SymbolText.h"
#include "SymbolLib/SymbolTexturedCircle.h"
#include "SymbolLib/SymbolTexturedPolygon.h"
#include "CoreMinimal.h"

using namespace sbio;
using namespace sbio::symbol;
using namespace sbio::unrealcigi;

CUnrealCigiSymbolEventHandler::CUnrealCigiSymbolEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
{
}

sbio::symbol::CSymbol* CUnrealCigiSymbolEventHandler::FindSymbol(SymbolID symbolID) const
{
  if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager == nullptr)
  {
    return nullptr;
  }

  return FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbol(symbolID);
}

void CUnrealCigiSymbolEventHandler::UpdateSymbolSurfaceWidget(SymbolSurfaceID surfaceID, int32 newAttachID)
{
  if (FUnrealCigi_PluginModule::globals.pSymbolSurfacePresenter == nullptr)
  {
    return;
  }

  FUnrealCigi_PluginModule::globals.pSymbolSurfacePresenter->UpdateSurfaceWidget(surfaceID, newAttachID);
}

void CUnrealCigiSymbolEventHandler::RemoveSymbolSurfaceWidget(SymbolSurfaceID surfaceID)
{
  if (FUnrealCigi_PluginModule::globals.pSymbolSurfacePresenter == nullptr)
  { 
    return;
  }
  
  FUnrealCigi_PluginModule::globals.pSymbolSurfacePresenter->RemoveSurfaceWidget(surfaceID);
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolTextMessage(const sbio::ig::symbol::SCreateSymbolTextMessage& data)
{
  // SymbolLib has created the symbol before dispatching this callback. Unreal only verifies the created geometry.
  sbio::symbol::CSymbolText* text = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolText>(data.SymbolID, ESymbolType::TEXT);
  if (text == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolTextMessage: SymbolLib has no text geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolTextMessage: Finished for id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolTextMessage(const sbio::ig::symbol::SUpdateSymbolTextMessage& data)
{
  // SymbolLib applies the text data before dispatching this callback. Unreal invalidates its cached text geometry.
  // Find the text geometry in the SymbolLib using the provided SymbolID.
  sbio::symbol::CSymbolText* text = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolText>(data.SymbolID, ESymbolType::TEXT);
  
  // Verify that the number of text symbols in the SymbolLib matches the expected number from the CIGI message.
  if (text == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolTextMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  // Invalidate the Unreal render cache so the next render uses the updated text geometry.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateText(data.SymbolID);
  const sbio::symbol::SSymbolTextDefinition& properties = text->GetProperties();
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolTextMessage: SymbolLib updated SymbolText id=%d with font(id=%d,size=%f)"), data.SymbolID.Value(), properties.fontID.Value(), properties.fFontSize);
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolCircleMessage(const sbio::ig::symbol::SCreateSymbolCircleMessage& data)
{
  // SymbolLib has created the symbol before dispatching this callback. Unreal only verifies the created geometry.
  sbio::symbol::CSymbolCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolCircle>(data.SymbolID, ESymbolType::CIRCLE);
  
  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolCircleMessage: SymbolLib has no circle geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolCircleMessage: Finished for SymbolID=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolCircleMessage(const sbio::ig::symbol::SUpdateSymbolCircleMessage& data)
{
  // SymbolLib applies the circle data before dispatching this callback. Unreal invalidates its cached circle geometry.
  sbio::symbol::CSymbolCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolCircle>(data.SymbolID, ESymbolType::CIRCLE);
  
  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolCircleMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  // Invalidate the Unreal render cache so the next render uses the updated arc geometry.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateCircleGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolCircleMessage: SymbolLib updated arc circle id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolCircleElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleElementMessage& data)
{
  // SymbolLib applies the element data before dispatching this callback. Unreal only needs to invalidate the cached geometry.
  sbio::symbol::CSymbolCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolCircle>(data.SymbolID, ESymbolType::CIRCLE);
  
  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolCircleElementMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  // Invalidate the Unreal render cache so the next render uses the updated arc geometry.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateCircleGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolCircleElementMessage: SymbolLib updated arc element for circle id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolCircleFilledMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledMessage& data)
{
  // SymbolLib applies the filled-circle data before dispatching this callback. Unreal invalidates its cached circle geometry.
  sbio::symbol::CSymbolCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolCircle>(data.SymbolID, ESymbolType::CIRCLE);
  
  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolCircleFilledMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  // Invalidate the Unreal render cache so the next render uses the updated filled-circle geometry.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateCircleGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolCircleFilledMessage: SymbolLib updated filled circle id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolCircleFilledElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledElementMessage& data)
{
  // SymbolLib applies the element data before dispatching this callback. Unreal only needs to invalidate the cached geometry.
  sbio::symbol::CSymbolCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolCircle>(data.SymbolID, ESymbolType::CIRCLE);
  
  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolCircleFilledElementMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  // Invalidate the Unreal render cache so the next render uses the updated filled-circle geometry.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateCircleGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolCircleFilledElementMessage: SymbolLib updated filled element for circle id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolTexturedCircleMessage(const sbio::ig::symbol::SCreateSymbolTexturedCircleMessage& data)
{
  // SymbolLib has created the symbol before dispatching this callback. Unreal only verifies the created geometry.
  sbio::symbol::CSymbolTexturedCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolTexturedCircle>(data.SymbolID, ESymbolType::TEXTURED_CIRCLE);
  
  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolTexturedCircleMessage: SymbolLib has no textured circle geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolTexturedCircleMessage: Finished for SymbolID=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolTexturedCircleMessage(const sbio::ig::symbol::SUpdateSymbolTexturedCircleMessage& data)
{
  // SymbolLib applies the textured-circle data before dispatching this callback. Unreal invalidates its cached geometry.
  sbio::symbol::CSymbolTexturedCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolTexturedCircle>(data.SymbolID, ESymbolType::TEXTURED_CIRCLE);
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolTexturedCircleMessage: SymbolLib has no textured circle geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  // Invalidate the Unreal render cache so the next render uses the updated textured-circle geometry.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateTexturedCircleGeometry(data.SymbolID);
  const sbio::symbol::SSymbolTexturedCircle& properties = circle->GetProperties();

  // Verify that the number of circles in the SymbolLib matches the expected number from the CIGI message.
  if (properties.circles.size() != data.NumCircles)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolTexturedCircleMessage: SymbolID=%d expected %u circles but SymbolLib contains %d"), data.SymbolID.Value(), data.NumCircles, static_cast<int32>(properties.circles.size()));
  }
}

void CUnrealCigiSymbolEventHandler::OnUpdateTexturedCircleMessage(const sbio::ig::symbol::SUpdateTexturedCircleMessage& data)
{
  // SymbolLib applies the circle data before dispatching this callback. No Unreal data is copied here; the textured-circle cache is invalidated instead.
  sbio::symbol::CSymbolTexturedCircle* circle = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolTexturedCircle>(data.SymbolID, ESymbolType::TEXTURED_CIRCLE);
  if (circle == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateTexturedCircleMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  const sbio::symbol::SSymbolTexturedCircle& properties = circle->GetProperties();
  if (data.CircleIndex >= properties.circles.size())
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateTexturedCircleMessage: Invalid CircleIndex=%u for SymbolID=%d"), data.CircleIndex, data.SymbolID.Value());
    return;
  }

  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateTexturedCircleGeometry(data.SymbolID);
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolPolygonMessage(const sbio::ig::symbol::SCreateSymbolPolygonMessage& data)
{
  // SymbolLib has created the symbol before dispatching this callback. Unreal only verifies the created geometry.
  sbio::symbol::CSymbolPolygon* polygon = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolPolygon>(data.SymbolID, ESymbolType::POLYGON);
  if (polygon == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolPolygonMessage: SymbolLib has no polygon geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolPolygonMessage: SymbolLib created polygon id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolPolygonMessage(const sbio::ig::symbol::SUpdateSymbolPolygonMessage& data)
{
  // SymbolLib applies the polygon data before dispatching this callback. Unreal invalidates its cached polygon geometry.
  sbio::symbol::CSymbolPolygon* polygon = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolPolygon>(data.SymbolID, ESymbolType::POLYGON);
  if (polygon == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolPolygonMessage: SymbolLib has no polygon geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidatePolygonGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolPolygonMessage: SymbolLib updated Symbol %d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolPolygonVertexMessage& data)
{
  // SymbolLib applies the vertex data before dispatching this callback. Unreal invalidates its cached polygon geometry.
  sbio::symbol::CSymbolPolygon* polygon = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolPolygon>(data.SymbolID, ESymbolType::POLYGON);
  if (polygon == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolPolygonVertexMessage: FAILED: SymbolID=%d is not a polygon"), data.SymbolID.Value());
    return;
  }

  const sbio::symbol::SSymbolPolygon& properties = polygon->GetProperties();
  if (data.VertexIndex >= properties.vertices.size())
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolPolygonVertexMessage: FAILED: VertexIndex=%u is out of range for SymbolID=%d"), data.VertexIndex, data.SymbolID.Value());
    return;
  }

  // SymbolLib applies the vertex message before notifying this handler. Invalidate the Unreal render cache so the next render uses the updated vertex.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidatePolygonGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolPolygonVertexMessage: SymbolLib updated vertex %u for polygon id=%d"), data.VertexIndex, data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SCreateSymbolTexturedPolygonMessage& data)
{
  // SymbolLib has created the symbol before dispatching this callback. Unreal only verifies the created geometry.
  sbio::symbol::CSymbolTexturedPolygon* polygon = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolTexturedPolygon>(data.SymbolID, ESymbolType::TEXTURED_POLYGON);
  if (polygon == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolTexturedPolygonMessage: SymbolLib has no textured polygon geometry for SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolTexturedPolygonMessage: Finished for SymbolID=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SUpdateSymbolTexturedPolygonMessage& data)
{
  // SymbolLib applies the textured-polygon data before dispatching this callback. Unreal invalidates its cached geometry.
  sbio::symbol::CSymbolTexturedPolygon* polygon = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolTexturedPolygon>(data.SymbolID, ESymbolType::TEXTURED_POLYGON);
  if (polygon == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolTexturedPolygonMessage: FAILED: SymbolID=%d is not a textured polygon"), data.SymbolID.Value());
    return;
  }

  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateTexturedPolygonGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolTexturedPolygonMessage: SymbolLib updated textured polygon id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolTexturedPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolTexturedPolygonVertexMessage& data)
{
  // SymbolLib applies the vertex data before dispatching this callback. Unreal invalidates its cached textured-polygon geometry.
  sbio::symbol::CSymbolTexturedPolygon* polygon = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindGeometry<sbio::symbol::CSymbolTexturedPolygon>(data.SymbolID, ESymbolType::TEXTURED_POLYGON);
  if (polygon == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolTexturedPolygonVertexMessage: FAILED: SymbolID=%d is not a textured polygon"), data.SymbolID.Value());
    return;
  }

  const sbio::symbol::SSymbolTexturedPolygon& properties = polygon->GetProperties();
  if (data.VertexIndex >= properties.vertices.size())
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolTexturedPolygonVertexMessage: FAILED: VertexIndex=%u is out of range for SymbolID=%d"), data.VertexIndex, data.SymbolID.Value());
    return;
  }

  // SymbolLib applies the vertex message before notifying this handler. Invalidate the Unreal render cache so the next render uses the updated vertex.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->InvalidateTexturedPolygonGeometry(data.SymbolID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolTexturedPolygonVertexMessage: SymbolLib updated vertex %u for textured polygon id=%d"), data.VertexIndex, data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateEntityBillboardSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateEntityBillboardSymbolSurfaceMessage& data)
{
  FUnrealSymbolSurface* surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->UpdateBillboardSurface(data);
  if (!surface)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateEntityBillboardSymbolSurfaceMessage: Could not find or create a symbol surface with id=%d"), data.SurfaceID.Value());
    return;
  }

  UpdateSymbolSurfaceWidget(surface->SurfaceID, data.EntityID.Value());
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateEntityBillboardSymbolSurfaceMessage: Updated surface %d to have params: %s"), data.SurfaceID.Value(), *surface->ToString());
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolSurfaceMessage(const sbio::ig::symbol::SCreateSymbolSurfaceMessage& data)
{
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->CreateSurface(data.SurfaceID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolSurfaceMessage: Surface with id=%d now exists"), data.SurfaceID.Value());
}

void CUnrealCigiSymbolEventHandler::OnDestroySymbolSurfaceMessage(const sbio::ig::symbol::SDestroySymbolSurfaceMessage& data)
{
  FUnrealSymbolSurface* surface = nullptr;
  if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
  {
    surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSurface(data.SurfaceID);
  }
  if (!surface)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnDestroySymbolSurfaceMessage: Could not find a symbol surface with id=%d"), data.SurfaceID.Value());
    return;
  }

  RemoveSymbolSurfaceWidget(data.SurfaceID);
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RemoveSurface(data.SurfaceID);
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnDestroySymbolSurfaceMessage: Deleted the symbol surface with id=%d"), data.SurfaceID.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateSymbolSurfaceMessage& data)
{
  FUnrealSymbolSurface* surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->UpdateWorldSurface(data);
  if (!surface)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolSurfaceMessage: Could not find or create a symbol surface with id=%d"), data.SurfaceID.Value());
    return;
  }

  UpdateSymbolSurfaceWidget(surface->SurfaceID, data.EntityID.Value());
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolSurfaceMessage: Updated surface %d to have params: %s"), data.SurfaceID.Value(), *surface->ToString());
}

void CUnrealCigiSymbolEventHandler::OnUpdateViewSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateViewSymbolSurfaceMessage& data)
{
  FUnrealSymbolSurface* surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->UpdateViewSurface(data);
  if (!surface)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateViewSymbolSurfaceMessage: Could not find or create a symbol surface with id=%d"), data.SurfaceID.Value());
    return;
  }

  UpdateSymbolSurfaceWidget(surface->SurfaceID, data.ViewID.Value());
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateViewSymbolSurfaceMessage: Updated surface %d to have params: %s"), data.SurfaceID.Value(), *surface->ToString());
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolColorMessage(const sbio::ig::symbol::SSetSymbolColorMessage& data)
{
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolColorMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolColorMessage: SymbolLib set color of Symbol %d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnDestroySymbolMessage(const sbio::ig::symbol::SDestroySymbolMessage& data)
{
  // SymbolLib has destroyed the symbol before dispatching this callback. Unreal removes any cached render data.
  FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RemoveRenderCaches(data.SymbolID);
  if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbol(data.SymbolID) == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnDestroySymbolMessage: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnDestroySymbolMessage: SymbolLib destroyed symbol id=%d"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolVisibleMessage(const sbio::ig::symbol::SSetSymbolVisibleMessage& data)
{
  // SymbolLib applies visibility before dispatching this callback. No Unreal setter is required.
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolVisibleMessage: FAILED: There is no SymbolID=%d"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolVisibleMessage: SymbolLib set Symbol %d to be %s"), data.SymbolID.Value(), symbol->IsVisible() ? TEXT("Visible") : TEXT("Invisible"));
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolAttachedMessage(const sbio::ig::symbol::SSetSymbolAttachedMessage& data)
{
  // SymbolLib applies the parent relationship before dispatching this callback. This handler only validates both symbols.
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolAttachedMessage: Invalid SymbolID %d"), data.SymbolID.Value());
    return;
  }

  sbio::symbol::CSymbol* parent = FindSymbol(data.ParentSymbolID);
  if (parent == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolAttachedMessage: Invalid ParentID=%d"), data.ParentSymbolID.Value());
  }
  else
  {
    UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolAttachedMessage: SymbolLib attached SymbolID=%d to ParentID=%d"), data.SymbolID.Value(), data.ParentSymbolID.Value());
  }
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolUnattachedMessage(const sbio::ig::symbol::SSetSymbolUnattachedMessage& data)
{
  // SymbolLib removes the parent relationship before dispatching this callback. This handler only validates the symbol state.
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolUnattachedMessage: Invalid SymbolID=%d"), data.SymbolID.Value());
    return;
  }
  if (symbol->GetParentSymbolID() == UnknownSymbolID)
  {
    UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolUnattachedMessage: SymbolID=%d has no parent"), data.SymbolID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolUnattachedMessage: Detached SymbolID=%d from its parent"), data.SymbolID.Value());
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolSurfaceMessage(const sbio::ig::symbol::SSetSymbolSurfaceMessage& data)
{
  // SymbolLib applies the symbol-to-surface assignment before dispatching this callback. Unreal only validates that the surface exists.
  // Find the symbol in the SymbolLib using the provided SymbolID
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);

  // Check if the symbol exists
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolSurfaceMessage: Invalid SymbolID %d"), data.SymbolID.Value());
    return;
  }

  // Check if the UnrealSymbolManager is valid
  if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolSurfaceMessage: UnrealSymbolManager is null"));
    return;
  }

  // Find the surface in the SymbolLib using the provided SurfaceID
  FUnrealSymbolSurface* surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSurface(data.SurfaceID);
  
  // Check if the surface exists
  if (surface == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetSymbolSurfaceMessage: Invalid SurfaceID %d"), data.SurfaceID.Value());
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetSymbolSurfaceMessage: SymbolLib attached symbol with id=%d to surface with id=%d"), data.SymbolID.Value(), data.SurfaceID.Value());
}

void CUnrealCigiSymbolEventHandler::OnSetTopLevelSymbolTransformMessage(const sbio::ig::symbol::SSetTopLevelSymbolTransformMessage& data)
{
  // CCigiSymbol applies the rotation before dispatching this callback. No setter is needed here.
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetTopLevelSymbolTransformMessage: Invalid SymbolID %d"), data.SymbolID.Value());
    return;
  }

  // CCigiSymbol::SetRotation applies the rotation before dispatching this message.
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetTopLevelSymbolTransformMessage: SymbolLib updated symbol %d with rotation=%.2f"), data.SymbolID.Value(), data.Rotation.Value());
}

void CUnrealCigiSymbolEventHandler::OnSetChildSymbolTransformMessage(const sbio::ig::symbol::SSetChildSymbolTransformMessage& data)
{
  // CCigiSymbol applies the rotation before dispatching this callback. No setter is needed here.
  // Find the symbol in the SymbolLib using the provided SymbolID
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);

  // Verify the symbol exists
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetChildSymbolTransformMessage: Invalid SymbolID %d"), data.SymbolID.Value());
    return;
  }

  // CCigiSymbol::SetRotation applies the rotation before dispatching this message.
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnSetChildSymbolTransformMessage: SymbolLib updated symbol %d with rotation=%.2f"), data.SymbolID.Value(), data.Rotation.Value());
}

void CUnrealCigiSymbolEventHandler::OnUpdateSymbolMessage(const sbio::ig::symbol::SUpdateSymbolMessage& data)
{
  // SymbolLib applies position and scale before dispatching this callback. No Unreal setter is required.
  // Find the symbol in the SymbolLib using the provided SymbolID
  sbio::symbol::CSymbol* symbol = FindSymbol(data.SymbolID);

  // Verify the symbol exists
  if (symbol == nullptr)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSymbolMessage: Invalid SymbolID %d"), data.SymbolID.Value());
    return;
  }

  // Log the updated position and scale of the symbol
  const sbio::math::Vec2f position = symbol->GetPosition();
  const sbio::math::Vec2f scale = symbol->GetScale();
  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUpdateSymbolMessage: SymbolLib updated symbol %d with offset=(%.2f,%.2f) and scale=(%.2f,%.2f)"), data.SymbolID.Value(), position.x(), position.y(), scale.x(), scale.y());
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolComponentStateMessage(const sbio::ig::symbol::SSetSymbolComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::SYMBOL, data.ComponentID.Value(), data.ComponentState, data.SymbolID.Value(), data.ComponentData);
}

void CUnrealCigiSymbolEventHandler::OnSetSymbolSurfaceComponentStateMessage(const sbio::ig::symbol::SSetSymbolSurfaceComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::SYMBOL_SURFACE, data.ComponentID.Value(), data.ComponentState, data.SymbolSurfaceID.Value(), data.ComponentData);
}

void CUnrealCigiSymbolEventHandler::OnCreateSymbolFromTemplateMessage(const sbio::ig::symbol::SCreateSymbolFromTemplateMessage& data)
{
  // SimulationSDK creates a TEMPLATE placeholder before dispatching this callback.
  // Reject an existing concrete symbol, but allow the manager to replace the placeholder.
  sbio::symbol::CSymbol* existingSymbol = FindSymbol(data.SymbolID);
  if (existingSymbol != nullptr && existingSymbol->GetSymbolType() != sbio::symbol::ESymbolType::TEMPLATE)
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolFromTemplateMessage: FAILED: A symbol already exists with id=%d. Choose a unique SymbolID."), data.SymbolID.Value());
    return;
  }

  // Find the symbol template by its ID
  USymbolConfig* symbolTemplate = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbolTemplate(data.TemplateID);

  // Check if the symbol template exists
  if (symbolTemplate == nullptr)
  {
    if (existingSymbol != nullptr && existingSymbol->GetSymbolType() == sbio::symbol::ESymbolType::TEMPLATE)
    {
      FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RemoveSymbol(data.SymbolID);
    }
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolFromTemplateMessage: FAILED: No symbol template with id=%d."), data.TemplateID);
    return;
  }

  // Create the symbol from the template
  if (!FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->CreateSymbolFromTemplate(data.SymbolID, *symbolTemplate))
  {
    sbio::symbol::CSymbol* failedSymbol = FindSymbol(data.SymbolID);
    if (failedSymbol != nullptr && failedSymbol->GetSymbolType() == sbio::symbol::ESymbolType::TEMPLATE)
    {
      FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->RemoveSymbol(data.SymbolID);
    }
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateSymbolFromTemplateMessage: FAILED: Could not create symbol from template id=%d"), data.TemplateID);
    return;
  }

  UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnCreateSymbolFromTemplateMessage: Created symbol %d from template %d"), data.SymbolID.Value(), data.TemplateID);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026