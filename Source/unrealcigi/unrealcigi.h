//Copyright SimBlocks LLC 2016-2026

#pragma once

/**
 * @class FCustomGameModuleImpl
 * @brief Implements the Unreal game module startup and shutdown hooks.
 */
class FCustomGameModuleImpl : public FDefaultGameModuleImpl
{
public:
  virtual void StartupModule() {};
  virtual void ShutdownModule() {};
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026