//Copyright SimBlocks LLC 2016-2026
#include "unrealcigiCelestialEventHandler.h"
#include "unrealcigiUtil.h"
#include "unrealcigiEventHandler.h"
#include "CelestialVaultDaySequenceActor.h"
#include "EngineUtils.h"

using namespace sbio;
using namespace sbio::unrealcigi;

ACelestialVaultDaySequenceActor* GetCelestialVaultActor(UWorld* WorldRef)
{
  ACelestialVaultDaySequenceActor* pCelestialVaultActor = nullptr;

  // Iterate through all actors of type ACelestialVaultDaySequenceActor in the world
  for (TActorIterator<ACelestialVaultDaySequenceActor> it(WorldRef); it; ++it)
  {
    pCelestialVaultActor = *it;
    break;
  }

  // If no CelestialVaultDaySequenceActor was found, log a warning and return nullptr
  if (pCelestialVaultActor == nullptr || !IsValid(pCelestialVaultActor))
  {
    UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateDateTimeMessage: No CelestialVaultDaySequenceActor was found, failed to update date/time"));
    return nullptr;
  }

  return pCelestialVaultActor;
}

void CUnrealCigiCelestialEventHandler::UpdateCelestialSphereMessage(const sbio::ig::celestial::SUpdateCelestialSphereMessage& data, UWorld* WorldRef)
{
  // get CelestialVault actor
  ACelestialVaultDaySequenceActor* pCelestialVaultActor = GetCelestialVaultActor(WorldRef);
  if (pCelestialVaultActor == nullptr)
  {
    return;
  }

  if (data.EphemerisEnabled)
  {
    // Set the duration back to its desired continuous value.
    // For example, 24 hours for a real-time day/night cycle.
    pCelestialVaultActor->SetTimePerCycle(24.0f);
  }
  else
  {
    // Set the duration of a day cycle in real time to 0 hours.
    // This effectively pauses the progression of the time of day.
    pCelestialVaultActor->SetTimePerCycle(0.0f);
  }
}

void CUnrealCigiCelestialEventHandler::UpdateDateTimeMessage(const sbio::ig::celestial::SUpdateDateTimeMessage& data, UWorld* WorldRef)
{
  // get CelestialVault actor
  ACelestialVaultDaySequenceActor* pCelestialVaultActor = GetCelestialVaultActor(WorldRef);
  if (pCelestialVaultActor == nullptr)
  {
    return;
  }

  pCelestialVaultActor->SetTimeOfDay(data.Time.hour.Value() + (data.Time.minute.Value() / 60.0) + (data.Time.seconds.Value() / 3600.0));
}

void CUnrealCigiCelestialEventHandler::UpdateLatLon(sbio::math::Latitude latitude, sbio::math::Longitude Longitude, UWorld* WorldRef)
{
  // get CelestialVault actor
  ACelestialVaultDaySequenceActor* pCelestialVaultActor = GetCelestialVaultActor(WorldRef);
  if (pCelestialVaultActor == nullptr)
  {
    return;
  }

  pCelestialVaultActor->Latitude = latitude.Value();
  pCelestialVaultActor->Longitude = Longitude.Value();
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026