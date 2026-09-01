//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiComponentDispatcher.h"
#include "CigiController.h"
#include "CigiBPLib.h"
#include "UnrealCigiEventHandler.h"

namespace sbio
{
  namespace unrealcigi
  {
    void CUnrealCigiComponentDispatcher::AddController(ACigiController* controller)
    {
      if (controller == nullptr)
      {
        return;
      }

      for (const TWeakObjectPtr<ACigiController>& reference : Controllers)
      {
        if (reference.Get() == controller)
        {
          return;
        }
      }
      Controllers.push_back(controller);
    }

    void CUnrealCigiComponentDispatcher::Process(ComponentClass componentClass, int componentID, int componentState, int instanceID, sbio::ig::SComponentData data)
    {
      FComponentMessage componentMessage(componentID, componentState, instanceID, data);
      for (int32 index = static_cast<int32>(Controllers.size()) - 1; index >= 0; --index)
      {
        if (!Controllers[index].IsValid())
        {
          Controllers.erase(Controllers.begin() + index);
        }
      }

      int messagesSent = 0;
      for (const TWeakObjectPtr<ACigiController>& reference : Controllers)
      {
        ACigiController* controller = reference.Get();
        if (IsValid(controller))
        {
          controller->OnComponentMessage(componentClass, componentMessage);
          ++messagesSent;
        }
      }

      const FString componentClassName = UCigiBPLib::Conv_ComponentClassToString(componentClass);
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("%s: Notified %d CigiController instances with message: %s"), *componentClassName, messagesSent, *componentMessage.ToString());
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026