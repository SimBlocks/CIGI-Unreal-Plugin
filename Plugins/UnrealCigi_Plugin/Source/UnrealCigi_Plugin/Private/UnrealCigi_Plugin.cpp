//Copyright SimBlocks LLC 2016-2026

#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"

#include "Misc/CoreDelegates.h"

#include "unrealcigiEventHandler.h"
#include "unrealcigiUtil.h"
#include "UnrealCigiConfigLoader.h"
#include "UnrealCigiDatabaseManager.h"
#include "UnrealCigiEnvironmentManager.h"
#include "SymbolLib/SymbolSurfaceManager.h"
#include "ViewLib/ViewManager.h"
#include "EntityLib/EntityLib.h"
#include "EntityLib/EntityManager.h"
#include "ViewLib/ViewLib.h"
#include "IGCigiLib/CigiView.h"
#include "UtilitiesLib/EventDispatcher.h"
#include "UtilitiesLib/Logger.h"
#include "UtilitiesLib/UtilitiesLib.h"
#include "ViewLib/IViewCreator.h"
#include "EngineLib/EngineLib.h"
#include "EngineLib/ImageGeneratorEventMessenger.h"
#include "SymbolLib/Symbol.h"
#include "SymbolLib/SymbolLib.h"
#include "IGCigiLib/IGCigiLib.h"
#include "IGCigiLib/CigiSymbolGeometryFactory.h"
#include "IGCigiLib/IGResponseEventDispatcher.h"
#include "ViewLib/ViewGroup.h"

using namespace std;
using namespace sbio;
using namespace sbio::utils;
using namespace sbio::entity;
using namespace sbio::symbol;
using namespace sbio::engine;
using namespace sbio::view;
using namespace sbio::cigi;
using namespace sbio::cigi::ig;
using namespace sbio::ig;
using namespace sbio::unrealcigi;

IMPLEMENT_MODULE(FUnrealCigi_PluginModule, UnrealCigi_Plugin);

// store all of the opened dlls so they can be shut down later
void* DLLPocoFoundation64;
void* DLLPocoJSON64;
void* DLLPocoNet64;

SUnrealCigiGlobals FUnrealCigi_PluginModule::globals = SUnrealCigiGlobals();
bool FUnrealCigi_PluginModule::globalsInitialized = false;

const FString DEFAULT_HOST_IP = "127.0.0.1";
const int DEFAULT_HOST_TO_IG_PORT = 5001;
const int DEFAULT_IG_TO_HOST_PORT = 5000;
bool TestingWithProjectLauncher = false;

void SUnrealCigiGlobals::Reset()
{
  pImageGenerator.reset();
  pExportedFunctionsEventDispatcher.reset();
  pEventMessenger.reset();
  pEventHandler.reset();
  pDatabaseManager.reset();
  pEnvironmentManager.reset();
  pSymbolSurfacePresenter.reset();
  pUnrealSymbolManager.reset();
  pPhysicsManager.reset();
  pComponentDispatcher.reset();
  pUnrealViewManager.reset();
  pUnrealEntityManager.reset();
  pSymbolSurfaceManager.reset();
  pViewManager.reset();
  pEntityManager.reset();
  pLogger.reset();
  pEventDispatcher.reset();
  applicationsDataPath.clear();
  librariesDataPath.clear();
}

SUnrealCigiGlobals::~SUnrealCigiGlobals() = default;

static bool LoadJsonHost(sbio::cigi::ig::SIGSetupOptions& igSetupOptions, TSharedPtr<FJsonObject> jsonObject);

/**
 * @class CigiViewCreator
 * @brief Creates Unreal view actors for the CIGI view subsystem.
 */
class CigiViewCreator : public sbio::view::IViewCreator
{
  virtual std::unique_ptr<CView> CreateView(ViewID viewID) override
  {
    return std::make_unique<CCigiView>(viewID);
  }
};

static void LoadIGSetupOptions(sbio::cigi::ig::SIGSetupOptions& igSetupOptions)
{
  igSetupOptions.eCigiVersion = ECigiVersion::VERSION_4_0;
  igSetupOptions.imageGeneratorID = ImageGeneratorID(0);
  igSetupOptions.eSynchronizationMode = ECigiSynchronizationMode::ASYNCHRONOUS;

  FString filePath = FString();
  TSharedPtr<FJsonObject> ConfigObject = CUnrealCigiConfigLoader::LoadJsonConfig(filePath);
  if (!ConfigObject.IsValid())
  {
    return;
  }

  // Attempt to read "TestingWithProjectLauncher" parameter
  if (ConfigObject->TryGetBoolField(TEXT("TestingWithProjectLauncher"), TestingWithProjectLauncher))
  {
    if (TestingWithProjectLauncher)
    {
#if WITH_EDITORONLY_DATA
      return;
#endif
    }
  }

  // Attempt to read "cigiVersion" parameter
  FString sCigiVersion;
  if (ConfigObject->TryGetStringField(TEXT("cigiVersion"), sCigiVersion))
  {
    if (sCigiVersion == "3.3")
    {
      igSetupOptions.eCigiVersion = ECigiVersion::VERSION_3_3;
    }
    else if (sCigiVersion == "4.0")
    {
      igSetupOptions.eCigiVersion = ECigiVersion::VERSION_4_0;
    }
  }

  // Attempt to read "imageGeneratorID" parameter
  int nImageGeneratorID = 0;
  if (ConfigObject->TryGetNumberField(TEXT("imageGeneratorID"), nImageGeneratorID))
  {
    igSetupOptions.imageGeneratorID = ImageGeneratorID(nImageGeneratorID);
  }

  // Either the old single-host parameters the new multi-host parameters, or both can be used to specify hosts
  // This variable tracks how many hosts were found. There should be at least one host specified in the JSON config file.
  int numHosts = 0;

  // Attempt to read single-host parameters from the JSON file's base object (this is the old method of specifying hosts)
  bool singleHostIsValid = LoadJsonHost(igSetupOptions, ConfigObject);
  if (singleHostIsValid)
  {
    numHosts++;
  }

  // Attempt to read multi-host parameters from the "hosts" array (this is the new method of specifying hosts)
  const TArray<TSharedPtr<FJsonValue>>* p_hostsValArray;
  if (ConfigObject->TryGetArrayField(TEXT("hosts"), p_hostsValArray))
  {
    for (TSharedPtr<FJsonValue> hostVal : (*p_hostsValArray))
    {
      // Get the current hostObject (curr item in "hosts" list)
      const TSharedPtr<FJsonObject>* p_hostObject;
      if (!hostVal->TryGetObject(p_hostObject))
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped host group b/c hostVal->TryGetObject failed!"));
        continue;
      }
      TSharedPtr<FJsonObject> hostObject = *p_hostObject;
      if (!hostObject.IsValid())
      {
        UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Skipped host group b/c hostObject is not valid!"));
        continue;
      }

      // Attempt to read the host parameters
      bool currHostIsValid = LoadJsonHost(igSetupOptions, hostObject);
      if (currHostIsValid)
      {
        numHosts++;
      }
    }
  }

  // If no valid hosts were found in the JSON file, then log a warning and use the default host values.
  if (numHosts == 0)
  {
    SHostSettings defaultHostSettings;
    defaultHostSettings.hostIPAddress = std::string(TCHAR_TO_UTF8(*DEFAULT_HOST_IP));
    defaultHostSettings.hostToIGPort = DEFAULT_HOST_TO_IG_PORT;
    defaultHostSettings.igToHostPort = DEFAULT_IG_TO_HOST_PORT;
    igSetupOptions.hostSettings.push_back(defaultHostSettings);

    UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: No valid CIGI Hosts were found! Added a default host: ip=%s, hostToIGPort=%d, igToHostPort=%d"), *DEFAULT_HOST_IP, DEFAULT_HOST_TO_IG_PORT, DEFAULT_IG_TO_HOST_PORT);
  }

  FString sDatabaseControl;
  if (ConfigObject->TryGetStringField(TEXT("databaseControl"), sDatabaseControl))
  {
    if (sDatabaseControl == "Host")
    {
      igSetupOptions.bDatabaseControlledByIG = false;
    }
    else if (sDatabaseControl == "IG")
    {
      igSetupOptions.bDatabaseControlledByIG = true;
    }
  }

  int defaultIGControlledDatabaseID = 0;
  if (ConfigObject->TryGetNumberField(TEXT("defaultIGControlledDatabaseID"), defaultIGControlledDatabaseID))
  {
    igSetupOptions.defaultIGControlledDatabaseID = CigiDatabaseNumber(defaultIGControlledDatabaseID);
  }

  FString sSynchronizationMode;
  if (ConfigObject->TryGetStringField(TEXT("synchronizationMode"), sSynchronizationMode))
  {
    sSynchronizationMode.TrimStartAndEndInline();

    if (sSynchronizationMode.Equals(TEXT("Asynchronous"), ESearchCase::IgnoreCase) || sSynchronizationMode.Equals(TEXT("Async"), ESearchCase::IgnoreCase))
    {
      igSetupOptions.eSynchronizationMode = ECigiSynchronizationMode::ASYNCHRONOUS;
    }
    else if (sSynchronizationMode.Equals(TEXT("Synchronous"), ESearchCase::IgnoreCase) || sSynchronizationMode.Equals(TEXT("Sync"), ESearchCase::IgnoreCase))
    {
      igSetupOptions.eSynchronizationMode = ECigiSynchronizationMode::SYNCHRONOUS;
    }
    else
    {
      UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Invalid synchronizationMode '%s'. Using default Asynchronous."), *sSynchronizationMode);
    }
  }

  FString sPacketLogger;
  if (ConfigObject->TryGetStringField(TEXT("PacketLogger"), sPacketLogger))
  {
    if (sPacketLogger == "None")
    {
      igSetupOptions.ePacketLoggerState = EPacketLoggerState::NONE;
    }
    else if (sPacketLogger == "Read")
    {
      igSetupOptions.ePacketLoggerState = EPacketLoggerState::READ;
    }
    else if (sPacketLogger == "Write")
    {
      igSetupOptions.ePacketLoggerState = EPacketLoggerState::WRITE;
    }
  }
}

// Supporting function for LoadIGSetupOptions
static bool LoadJsonHost(sbio::cigi::ig::SIGSetupOptions& igSetupOptions, TSharedPtr<FJsonObject> jsonObject)
{
  SHostSettings hostSettings;
  hostSettings.hostIPAddress = "127.0.0.1";
  hostSettings.igToHostPort = DEFAULT_IG_TO_HOST_PORT;
  hostSettings.hostToIGPort = DEFAULT_HOST_TO_IG_PORT;

  // Read "hostIP" (new name) OR "hostIPAddress" (old name)
  FString sHostIP;
  if (jsonObject->TryGetStringField(TEXT("hostIP"), sHostIP))
  {
    hostSettings.hostIPAddress = std::string(TCHAR_TO_UTF8(*sHostIP));
  }
  else if (jsonObject->TryGetStringField(TEXT("hostIPAddress"), sHostIP))
  {
    hostSettings.hostIPAddress = std::string(TCHAR_TO_UTF8(*sHostIP));
  }
  // If no valid IP address is given for the host, then this host is invalid
  else
  {
    return false;
  }

  // Read "igToHostPort" (new name) OR "sendPort" (old name)
  int nIgToHostPort = 0;
  if (jsonObject->TryGetNumberField(TEXT("igToHostPort"), nIgToHostPort))
  {
    hostSettings.igToHostPort = nIgToHostPort;
  }
  else if (jsonObject->TryGetNumberField(TEXT("sendPort"), nIgToHostPort))
  {
    hostSettings.igToHostPort = nIgToHostPort;
  }

  // Read "hostToIgPort" (new name) OR "receivePort" (old name)
  int nHostToIgPort = 0;
  if (jsonObject->TryGetNumberField(TEXT("hostToIgPort"), nHostToIgPort))
  {
    hostSettings.hostToIGPort = nHostToIgPort;
  }
  else if (jsonObject->TryGetNumberField(TEXT("receivePort"), nHostToIgPort))
  {
    hostSettings.hostToIGPort = nHostToIgPort;
  }

  if (hostSettings.igToHostPort <= 0 || hostSettings.igToHostPort > 65535)
  {
    UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Host '%s' has invalid igToHostPort=%d. Skipping host."), *sHostIP, hostSettings.igToHostPort);
    return false;
  }

  // Validate that the hostToIGPort is within the valid port range (1-65535). If not, log a warning and skip this host.
  if (hostSettings.hostToIGPort <= 0 || hostSettings.hostToIGPort > 65535)
  {
    UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Host '%s' has invalid hostToIgPort=%d. Skipping host."), *sHostIP, hostSettings.hostToIGPort);
    return false;
  }

  if (sbio::unrealcigi::utils::IsValidIPv4Address(sHostIP) == false && !sHostIP.Contains(TEXT("localhost")) && !sHostIP.Contains(TEXT(".")) && !sHostIP.Contains(TEXT("-")))
  {
    UE_LOG(LogCigiEventHandler, JSON_WARNING, TEXT("JSON: Host '%s' does not look like a valid IP address or host name. Skipping host."), *sHostIP);
    return false;
  }

  // Add this new host to the list of hosts
  igSetupOptions.hostSettings.push_back(hostSettings);

  // Log whenever a valid host is added
  UE_LOG(LogCigiEventHandler, JSON_LOG, TEXT("JSON: Added CIGI Host with ip=%s, hostToIgPort=%d, igToHostPort=%d"), *sHostIP, hostSettings.hostToIGPort, hostSettings.igToHostPort);

  // The host is valid as long as a valid ip address was given. Specifying the ports is optional.
  return true;
}

// Override function to load the dlls
void FUnrealCigi_PluginModule::StartupModule()
{
  // Register a callback for engine pre-exit to release rooted UObjects before Unreal begins shutting down UObjects.
  EnginePreExitHandle = FCoreDelegates::OnEnginePreExit.AddRaw(this, &FUnrealCigi_PluginModule::HandleEnginePreExit);

  // Get the DLL path from the SBIO_SIMULATION_SDK environment variable.
  std::filesystem::path simulationsdkPath = sbio::utils::GetEnvVariablePath("SBIO_SIMULATION_SDK");

  // Determine the runtime configuration (Release vs Debug) to load the correct set of DLLs
  // The runtime configuration is selected by UnrealBuildTool.
#if SBIO_RUNTIME_DEBUG
  const std::string runtimeFolder = "vs2022_Debug";
  const std::string runtimeSuffix = "d";
#else
  const std::string runtimeFolder = "vs2022_Release";
  const std::string runtimeSuffix;
#endif

  // Load DLLs and store their handles
  const FString pocoFoundationPath = UTF8_TO_TCHAR((simulationsdkPath / ("Code/Bin/vc143/x64/" + runtimeFolder + "/PocoFoundation64" + runtimeSuffix + ".dll")).string().c_str());
  const FString pocoJsonPath = UTF8_TO_TCHAR((simulationsdkPath / ("Code/Bin/vc143/x64/" + runtimeFolder + "/PocoJSON64" + runtimeSuffix + ".dll")).string().c_str());
  const FString pocoNetPath = UTF8_TO_TCHAR((simulationsdkPath / ("Code/Bin/vc143/x64/" + runtimeFolder + "/PocoNet64" + runtimeSuffix + ".dll")).string().c_str());

  DLLPocoFoundation64 = FPlatformProcess::GetDllHandle(*pocoFoundationPath);
  DLLPocoJSON64 = FPlatformProcess::GetDllHandle(*pocoJsonPath);
  DLLPocoNet64 = FPlatformProcess::GetDllHandle(*pocoNetPath);

  // Check if any DLLs failed to load
  if (DLLPocoFoundation64 == nullptr || DLLPocoJSON64 == nullptr || DLLPocoNet64 == nullptr)
  {
    UE_LOG(LogCigiEventHandler, Error, TEXT("Failed to load SimulationSDK dependencies. Foundation='%s', JSON='%s', Net='%s'."), DLLPocoFoundation64 != nullptr ? TEXT("loaded") : *pocoFoundationPath, DLLPocoJSON64 != nullptr ? TEXT("loaded") : *pocoJsonPath,
           DLLPocoNet64 != nullptr ? TEXT("loaded") : *pocoNetPath);

    // Free any DLLs that were successfully loaded before the failure
    FPlatformProcess::FreeDllHandle(DLLPocoFoundation64);
    FPlatformProcess::FreeDllHandle(DLLPocoJSON64);
    FPlatformProcess::FreeDllHandle(DLLPocoNet64);
    DLLPocoFoundation64 = nullptr;
    DLLPocoJSON64 = nullptr;
    DLLPocoNet64 = nullptr;
  }
}

void FUnrealCigi_PluginModule::HandleEnginePreExit()
{
  // Release rooted config objects before Unreal begins shutting down UObjects.
  // This ensures that any UObject references in those config objects are released while UObjects are still valid,
  // preventing potential crashes during shutdown.
  if (globals.pEventHandler != nullptr)
  {
    CUnrealCigiConfigLoader::ReleaseRootedConfigObjects();
  }
}

// Override function to shut down the dlls
void FUnrealCigi_PluginModule::ShutdownModule()
{
  // Unregister the engine pre-exit callback
  if (EnginePreExitHandle.IsValid())
  {
    FCoreDelegates::OnEnginePreExit.Remove(EnginePreExitHandle);
    EnginePreExitHandle.Reset();
  }

  // Ensure rooted config objects are released before destroying the handler,
  // even if shutdown occurs outside the normal engine pre-exit path.
  HandleEnginePreExit();

  // Reset globals to release resources and avoid crashes if StartupModule is called again
  globals.Reset();
  globalsInitialized = false;

  // Free the loaded DLLs
  FPlatformProcess::FreeDllHandle(DLLPocoFoundation64);
  FPlatformProcess::FreeDllHandle(DLLPocoJSON64);
  FPlatformProcess::FreeDllHandle(DLLPocoNet64);

  DLLPocoFoundation64 = nullptr;
  DLLPocoJSON64 = nullptr;
  DLLPocoNet64 = nullptr;
}

void FUnrealCigi_PluginModule::StartIG()
{
  // Fist-time setup: Doing this more than once per UE Editor Window will cause crashes
  if (!globalsInitialized)
  {
    InitializeGlobals();
    globalsInitialized = (globals.pImageGenerator != nullptr);
  }
}

void FUnrealCigi_PluginModule::StopIG()
{
  // The current method of stopping the IG doesn't require any cleanup in this function
}

// StartIG() first call: Loads the globals, create supporting objects, and initialize the Image Generator
void FUnrealCigi_PluginModule::InitializeGlobals()
{
  std::filesystem::path sDir = GetCurrentWorkingDirectory();

  // Reset globals so that consecutive StartupModule() calls don't crash
  globals.Reset();

  // Init Image Generator
  SIGSetupOptions igSetupOptions;
  LoadIGSetupOptions(igSetupOptions);

  UE_LOG(LogCigiEventHandler, Log, TEXT("InitializeGlobals: Preparing IG startup with %d configured host(s), CIGI version=%d, imageGeneratorID=%d, databaseControlledByIG=%d, defaultIGControlledDatabaseID=%d"), static_cast<int32>(igSetupOptions.hostSettings.size()),
         static_cast<int32>(igSetupOptions.eCigiVersion), igSetupOptions.imageGeneratorID.Value(), igSetupOptions.bDatabaseControlledByIG ? 1 : 0, igSetupOptions.defaultIGControlledDatabaseID.Value());

  int32 hostIndex = 0;
  for (const SHostSettings& hostSettings : igSetupOptions.hostSettings)
  {
    UE_LOG(LogCigiEventHandler, Log, TEXT("InitializeGlobals: Host[%d] ip=%s hostToIGPort=%d igToHostPort=%d"), hostIndex, UTF8_TO_TCHAR(hostSettings.hostIPAddress.c_str()), hostSettings.hostToIGPort, hostSettings.igToHostPort);

    ++hostIndex;
  }

  // If testing through the project launcher, skip initializing any of the CIGI stuff in the editor
  // otherwise two image generators are running, and the host fails to connect to the project launcher one
  if (TestingWithProjectLauncher)
  {
#if WITH_EDITORONLY_DATA
    return;
#endif
  }

  // Set up the global paths and shared managers
  globals.applicationsDataPath = GetSdkPath() / "Data" / "Applications";
  globals.librariesDataPath = GetSdkPath() / "Data" / "Libraries";
  globals.pEventDispatcher = make_shared<CEventDispatcher>();
  globals.pLogger = make_shared<CLogger>("Unreal ImageGenerator", true, true);

  // Create the shared managers and handlers
  globals.pEntityManager = make_shared<CEntityManager>();
  globals.pViewManager = make_unique<CViewManager>();
  globals.pSymbolSurfaceManager = make_shared<CSymbolSurfaceManager>();
  globals.pDatabaseManager = std::make_unique<CUnrealCigiDatabaseManager>();
  globals.pEnvironmentManager = std::make_unique<CUnrealCigiEnvironmentManager>();
  globals.pUnrealEntityManager = std::make_unique<CUnrealCigiEntityManager>();
  globals.pUnrealViewManager = std::make_unique<CUnrealCigiViewManager>();
  globals.pPhysicsManager = std::make_unique<CUnrealCigiPhysicsManager>(*globals.pUnrealEntityManager);
  globals.pComponentDispatcher = std::make_unique<CUnrealCigiComponentDispatcher>();
  globals.pEventHandler = std::make_unique<CUnrealCigiEventHandler>();
  globals.pUnrealSymbolManager = std::make_unique<CUnrealCigiSymbolManager>(globals.pSymbolSurfaceManager);
  globals.pSymbolSurfacePresenter = std::make_unique<CUnrealCigiSymbolSurfacePresenter>(*globals.pEventHandler);
  globals.pEventMessenger = std::make_unique<CImageGeneratorEventMessenger>(globals.pEventHandler.get());
  globals.pExportedFunctionsEventDispatcher = std::make_unique<CIGResponseEventDispatcher>();

  // Initialize the entity library with the shared entity manager
  SEntityLibParams entityLibParams;
  entityLibParams.pEntityManager = globals.pEntityManager;
  InitEntityLib(globals, entityLibParams);
  InitUtilitiesLib(globals);

  // Initialize the CIGI library with the shared managers and event messenger
  SIGCigiLibParams cigiParams;
  cigiParams.pEventMessenger = globals.pEventMessenger.get();
  cigiParams.pEntityManager = globals.pEntityManager;
  cigiParams.pSymbolSurfaceManager = globals.pSymbolSurfaceManager;
  cigiParams.pViewManager = globals.pViewManager;
  InitIGCigiLib(globals, cigiParams);

  // Initialize the view library with the shared view manager
  SViewLibParams viewLibParams;
  viewLibParams.pViewManager = globals.pViewManager;
  InitViewLib(globals, viewLibParams);

  // Initialize the view creator for CIGI views
  SEngineLibParams engineLibParams;
  engineLibParams.pEventMessenger = globals.pEventMessenger.get();
  InitEngineLib(globals, engineLibParams);

  std::unique_ptr<CSymbolGeometryFactory> pSymbolGeometryFactory = std::make_unique<CCigiSymbolGeometryFactory>();
  InitSymbolLib(globals, std::move(pSymbolGeometryFactory));

  igSetupOptions.pathToCigiSisoConversionsFile = GetSdkPath() / "Data" / "Applications" / "CIGI ImageGenerator" / "CigiSiso.csv";

  // Create image generator object
  globals.pImageGenerator = make_unique<CCigiImageGenerator>(igSetupOptions);
  globals.pImageGenerator->Initialize();
  SetIGCigiLibImageGenerator(globals.pImageGenerator.get());

  // Note: ViewIDs must be handled by CUnrealCigiEventHandler::LoadJsonConfig and CUnrealCigiEventHandler::SetupViewActor.
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026