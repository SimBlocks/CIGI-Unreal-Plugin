//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include <vector>

class ACigiController;
enum class ComponentClass : uint8;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiComponentDispatcher
     * @brief Dispatches received CIGI component messages to the corresponding event handlers.
     */
    class CUnrealCigiComponentDispatcher
    {
    public:
      std::vector<TWeakObjectPtr<ACigiController>> Controllers;

      /**
       * @brief Registers a controller to receive component messages.
       *
       * Null controllers and controllers that are already registered are ignored.
       * The controller is stored as a weak reference so its lifetime is not extended.
       *
       * @param controller Controller to register.
       */
      void AddController(ACigiController* controller);

      /**
       * @brief Dispatches a component message to all registered controllers.
       *
       * Removes expired controller references, constructs an FComponentMessage from
       * the supplied values, and invokes ACigiController::OnComponentMessage on each
       * valid controller.
       *
       * @param componentClass Class of component affected by the message.
       * @param componentID Identifier of the component.
       * @param componentState State value to apply to the component.
       * @param instanceID Identifier of the component instance.
       * @param data Additional component-specific message data.
       */
      void Process(ComponentClass componentClass, int componentID, int componentState, int instanceID, sbio::ig::SComponentData data);
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026