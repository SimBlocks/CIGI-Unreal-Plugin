//Copyright SimBlocks LLC 2016-2026
/**
 * @file UnrealCigi_Declarations.h
 * @brief Forward declarations for core SimBlocks UnrealCigi plugin types.
 *
 * This header provides forward declarations for key classes and structs used throughout the plugin,
 * including the event handler, symbol, and symbol surface types.
 *
 * Usage:
 * - Include this file in headers that need to reference types without requiring their full definitions.
 * - Helps reduce compile-time dependencies and circular includes.
 *
 * Types declared:
 * - sbio::unrealcigi::CUnrealCigiEventHandler: Main event handler for CIGI IG messages and simulation events.
 * - sbio::unrealcigi::Symbol: Represents a symbol object for rendering and management.
 * - sbio::unrealcigi::FUnrealSymbolSurface: Structure describing a symbol surface for display.
 */

#pragma once

/**
 * @namespace sbio::unrealcigi
 * @brief Namespace for SimBlocks.io UnrealCigi plugin types.
 */
// Forward declarations
namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;
    class CUnrealCigiCelestialEventHandler;
    class CUnrealCigiSymbolManager;
    class CUnrealCigiSymbolSurfacePresenter;
    class CUnrealCigiEntityManager;
    class CUnrealCigiViewManager;
    class CUnrealCigiPhysicsManager;
    class CUnrealCigiDatabaseManager;
    class CUnrealCigiComponentDispatcher;
    class CUnrealCigiEnvironmentManager;
    struct FUnrealSymbolSurface;
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026