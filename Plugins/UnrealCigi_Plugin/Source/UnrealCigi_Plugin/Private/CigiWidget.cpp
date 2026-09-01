//Copyright SimBlocks LLC 2016-2026

#include "CigiWidget.h"
#include "unrealcigiEventHandler.h"

using namespace sbio;
using namespace sbio::symbol;

void UCigiWidget::ReleaseSlateResources(bool bReleaseChildren)
{
  Super::ReleaseSlateResources(bReleaseChildren);
  MyCigiWidget.Reset();
}

TSharedRef<SWidget> UCigiWidget::RebuildWidget()
{
  UE_LOG(LogTemp, Display, TEXT("Rebuilding widget with surfaceID %d"), m_SymbolSurfaceID.Value());
  // This widget is just a wrapper for SSlateCigiWidget.
  // We just pass in the surface ID and let the SSlateCigiWidget class do all of the display logic
  MyCigiWidget = SNew(SSlateCigiWidget).SurfaceID(m_SymbolSurfaceID.Value());
  return MyCigiWidget.ToSharedRef();
}

void UCigiWidget::SetSurfaceID(SymbolSurfaceID symbolSurfaceID)
{
  m_SymbolSurfaceID = symbolSurfaceID;
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026