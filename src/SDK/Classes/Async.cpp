#include "SDK/Classes/Async.h"
#include "SDK/WindroseSignatures.h"

namespace UECustom {
    void AsyncTask(ENamedThreads Thread, const RC::Unreal::TUniqueFunction<void()>& Function)
    {
        using AsyncTaskSignature = void(*)(ENamedThreads, const RC::Unreal::TUniqueFunction<void()>&);
        static void* FunctionPtr = nullptr;

        if (!FunctionPtr)
        {
            FunctionPtr = Windrose::SignatureManager::GetSignature("AsyncTask");
        }

        if (!FunctionPtr)
        {
            return;
        }

        reinterpret_cast<AsyncTaskSignature>(FunctionPtr)(Thread, Function);
    }
}
