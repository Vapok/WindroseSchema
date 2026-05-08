#include "SDK/Helper/BPGeneratedClassHelper.h"
#include "SDK/WindroseSignatures.h"

using namespace RC;
using namespace RC::Unreal;

UObject* UECustom::BPGeneratedClassHelper::FindComponentTemplateByName(UObject* BPGeneratedClass, FName TemplateName)
{
    using FindComponentTemplateByNameSignature = UObject*(*)(UObject*, const FName&);
    static void* FunctionPtr = nullptr;

    if (!FunctionPtr)
    {
        FunctionPtr = Windrose::SignatureManager::GetSignature("UBlueprintGeneratedClass::FindComponentTemplateByName");
    }

    if (!FunctionPtr)
    {
        return nullptr;
    }

    return reinterpret_cast<FindComponentTemplateByNameSignature>(FunctionPtr)(BPGeneratedClass, TemplateName);
}

bool UECustom::BPGeneratedClassHelper::GetGeneratedClassesHierarchy(UClass* InClass, TArray<UObject*>& OutBPGClasses)
{
    using GetGeneratedClassesHierarchySignature = bool(*)(UClass*, TArray<UObject*>&);
    static void* FunctionPtr = nullptr;

    if (!FunctionPtr)
    {
        FunctionPtr = Windrose::SignatureManager::GetSignature("UBlueprintGeneratedClass::FindComponentTemplateByName");
    }

    if (!FunctionPtr)
    {
        return false;
    }

    return reinterpret_cast<GetGeneratedClassesHierarchySignature>(FunctionPtr)(InClass, OutBPGClasses);
}
