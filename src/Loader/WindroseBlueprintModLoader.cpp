#include <regex>
#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "Unreal/UObject.hpp"
#include "Unreal/AActor.hpp"
#include "Helpers/String.hpp"
#include "SDK/Helper/PropertyHelper.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "Utility/JsonHelpers.h"
#include "Loader/WindroseBlueprintModLoader.h"
#include "SDK/Classes/KismetSystemLibrary.h"
#include "SDK/Helper/BPGeneratedClassHelper.h"
#include "SDK/Helper/Memory.h"
#include "SDK/Classes/Custom/UBlueprintGeneratedClass.h"
#include "SDK/Classes/Custom/UInheritableComponentHandler.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"

using namespace RC;
using namespace RC::Unreal;

namespace Windrose {
    WindroseBlueprintModLoader::WindroseBlueprintModLoader() : WindroseModLoaderBase("blueprints")
    {
        SetDisplayName(TEXT("Blueprint Mod Loader"));
    }

    WindroseBlueprintModLoader::~WindroseBlueprintModLoader()
    {
        auto expected = PostLoadHook.disable();
        PostLoadHook = {};
        PostLoadCallback = nullptr;
        m_modsMap.clear();
    }

    void WindroseBlueprintModLoader::OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase == EEngineLifecyclePhase::PostEngineInit)
        {
            PS::JsonHelpers::ParseJsonFilesInPath(loaderPath, [&](const nlohmann::json& data) {
                LoadSafe(data);
            });
        }
        else if (engineLifecyclePhase == EEngineLifecyclePhase::GameInstanceInit)
        {
            PS::JsonHelpers::ParseJsonFilesInPath(loaderPath, [&](const nlohmann::json& data) {
                LoadUnsafe(data);
            });
        }
    }

    void WindroseBlueprintModLoader::OnAutoReload(const std::filesystem::path::string_type& modName, const std::filesystem::path& modFilePath)
    {
        PS::JsonHelpers::ParseJsonFileInPath(modFilePath, [&](const nlohmann::json& data) {
            LoadUnsafe(data);
        });
    }

    bool WindroseBlueprintModLoader::CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase == EEngineLifecyclePhase::PostEngineInit)
        {
            return true;
        }

        return false;
    }

    bool WindroseBlueprintModLoader::OnInitialize()
    {
        // PostLoad hook handles default objects for classes
        if (!HookPostLoad())
        {
            PS::Log<LogLevel::Error>(TEXT("Cannot hook UBlueprintGeneratedClass::PostLoad which means blueprint mods will not function properly.\n"));
            return false;
        }

        // PostInitComponents hook handles actor instances
        if (!HookPostInitComponents())
        {
            PS::Log<LogLevel::Error>(TEXT("Cannot hook AActor::PostInitComponents which means blueprint mods will not function properly.\n"));
            return false;
        }

        return true;
    }

    bool WindroseBlueprintModLoader::HookPostLoad()
    {
        auto vtable = Windrose::GetVTablePtrByClassPath(TEXT("/Script/Engine.BlueprintGeneratedClass"));
        if (!vtable)
        {
            PS::Log<LogLevel::Error>(STR("Something went wrong with getting VTable pointer for UBlueprintGeneratedClass."));
            return false;
        }

        void* postloadPtr = Windrose::GetVirtualFunctionFromVTable(vtable, 19);
        PS::Log<LogLevel::Verbose>(TEXT("Found UBlueprintGeneratedClass::PostLoad: {}\n"), postloadPtr);

        PostLoadCallback = [&](UClass* actorClass) {
            ModifyObject(actorClass->GetClassDefaultObject());
        };

        PostLoadHook = safetyhook::create_inline(postloadPtr,
            reinterpret_cast<void*>(PostLoad));

        return true;
    }

    bool WindroseBlueprintModLoader::HookPostInitComponents()
    {
        auto vtable = Windrose::GetVTablePtrByClassPath(TEXT("/Script/Engine.Actor"));
        if (!vtable)
        {
            PS::Log<LogLevel::Error>(TEXT("Something went wrong with getting VTable pointer for AActor.\n"));
            return false;
        }

        void* postInitCompsPtr = Windrose::GetVirtualFunctionFromVTable(vtable, 169);
        PS::Log<LogLevel::Verbose>(TEXT("Found AActor::PostInitializeComponents: {}\n"), postInitCompsPtr);

        PostInitComponentsCallback = [&](AActor* self) {
            if (m_hasDeferredDataAssets)
            {
                ApplyDeferredDataAssets();
            }
            ModifyObject(self);
        };

        PostInitComponentsHook = safetyhook::create_inline(postInitCompsPtr,
            reinterpret_cast<void*>(PostInitComponents));

        return true;
    }

    void WindroseBlueprintModLoader::LoadSafe(const nlohmann::json& data)
    {
        for (auto& [assetName, assetData] : data.items())
        {
            auto assetNameWide = RC::to_generic_string(assetName);
            if (!assetNameWide.starts_with(TEXT("/Game/")))
            {
                auto assetFName = FName(assetNameWide, FNAME_Add);
                auto newMod = WindroseBlueprintMod(assetFName, assetData);
                auto it = m_modsMap.find(assetFName);
                if (it != m_modsMap.end())
                {
                    m_modsMap.at(assetFName).push_back(newMod);
                }
                else
                {
                    auto newModContainer = std::vector<WindroseBlueprintMod>{
                        newMod
                    };
                    m_modsMap.emplace(assetFName, newModContainer);
                }

                PS::Log<LogLevel::Normal>(STR("Loaded changes to {}\n"), assetNameWide);
            }
        }
    }

    void WindroseBlueprintModLoader::LoadUnsafe(const nlohmann::json& data)
    {
        for (auto& [assetName, assetData] : data.items())
        {
            auto originalAssetPath = RC::to_generic_string(assetName);
            auto assetNameWide = originalAssetPath;
            if (assetNameWide.starts_with(TEXT("/Game/")))
            {
                // Construct the full object path (Package.ObjectName) so StaticFindObject
                // resolves the DataAsset export, not just the UPackage container.
                // e.g. /Game/.../DA_BI_Foo  ->  /Game/.../DA_BI_Foo.DA_BI_Foo
                static const std::wregex ShortNamePattern(LR"(^(.*/)([^/]+)$)");
                auto objectPath = std::regex_replace(assetNameWide, ShortNamePattern, TEXT("$1$2.$2"));

                // Probe the exact path first to distinguish DataAssets from Blueprint classes.
                // DataAssets (DA_*, etc.) have no _C subobject; appending _C and calling
                // LoadAsset_Blocking on a DataAsset path can crash. If the object is already
                // in memory as a non-class (PrimaryDataAsset, etc.) we patch it directly here
                // and skip the LoadAsset_Blocking path entirely.
                auto existingObject = UECustom::UObjectGlobals::StaticFindObject(nullptr, nullptr, objectPath.c_str(), false);
                if (existingObject && !existingObject->IsA<UClass>())
                {
                    auto objClass = existingObject->GetClassPrivate();
                    auto objClassName = objClass ? objClass->GetNamePrivate().ToString() : TEXT("");

                    if (objClassName.find(TEXT("Blueprint")) != RC::StringType::npos)
                    {
                        PS::Log<LogLevel::Warning>(TEXT("Skipped '{}': resolved to editor Blueprint object (class '{}'). "
                            "Use the generated class path (...Foo.Foo_C) or short class key.\n"),
                            originalAssetPath, objClassName);
                        continue;
                    }

                    PS::Log<LogLevel::Normal>(TEXT("Patching DataAsset '{}' (class '{}')...\n"),
                        existingObject->GetNamePrivate().ToString(), objClassName);
                    existingObject->SetRootSet();
                    ApplyData(assetData, existingObject);
                    PS::Log<RC::LogLevel::Normal>(TEXT("Applied changes to DataAsset {}\n"),
                        existingObject->GetNamePrivate().ToString());
                    continue;
                }

                if (existingObject == nullptr)
                {
                    // DataAsset not in memory yet — force-loading it during R5GameInstance::Init
                    // can crash. Queue it for deferred application once PostInitializeComponents
                    // fires, by which point building DataAssets will be loaded.
                    PS::Log<LogLevel::Normal>(TEXT("'{}' not in memory at GameInstanceInit, queuing for deferred patch.\n"),
                        objectPath);
                    m_deferredDataAssetMods[objectPath] = assetData;
                    m_hasDeferredDataAssets = true;
                    continue;
                }

                // existingObject is non-null and IsA<UClass> — standard Blueprint path.
                static const std::wregex Pattern(LR"(^(.*/)([^/.]+)$)");
                auto blueprintClassPath = std::regex_replace(assetNameWide, Pattern, TEXT("$1$2.$2_C"));

                auto softObjectPtr = UECustom::TSoftObjectPtr<UObject>(UECustom::FSoftObjectPath(blueprintClassPath));
                auto asset = UECustom::UKismetSystemLibrary::LoadAsset_Blocking(softObjectPtr);
                if (!asset)
                {
                    throw std::runtime_error(RC::fmt("Failed to apply blueprint changes, asset '%S' was invalid", originalAssetPath.c_str()));
                }
                if (!asset->IsA<UClass>())
                {
                    auto loadedAssetClass = asset->GetClassPrivate();
                    auto loadedAssetClassName = loadedAssetClass ? loadedAssetClass->GetNamePrivate().ToString() : TEXT("<unknown>");
                    PS::Log<LogLevel::Warning>(TEXT("Path '{}' resolved to non-class '{}' (class '{}'). Skipping.\n"),
                        blueprintClassPath, asset->GetNamePrivate().ToString(), loadedAssetClassName);
                    continue;
                }

                asset->SetRootSet();
                auto loadedClass = static_cast<UClass*>(asset);
                auto& defaultObject = loadedClass->GetClassDefaultObject();
                auto* targetObject = defaultObject.Get();

                ApplyData(assetData, targetObject);
                PS::Log<RC::LogLevel::Normal>(TEXT("Applied changes to {}\n"), targetObject->GetNamePrivate().ToString());
            }
        }
    }

    void WindroseBlueprintModLoader::ApplyDeferredDataAssets()
    {
        auto it = m_deferredDataAssetMods.begin();
        while (it != m_deferredDataAssetMods.end())
        {
            auto& path = it->first;
            auto obj = UECustom::UObjectGlobals::StaticFindObject(nullptr, nullptr, path.c_str(), false);
            if (obj && !obj->IsA<UClass>())
            {
                auto objClass = obj->GetClassPrivate();
                auto objClassName = objClass ? objClass->GetNamePrivate().ToString() : TEXT("");
                PS::Log<LogLevel::Normal>(TEXT("Applying deferred patch to DataAsset '{}' (class '{}')...\n"),
                    obj->GetNamePrivate().ToString(), objClassName);
                obj->SetRootSet();
                try
                {
                    ApplyData(it->second, obj);
                    PS::Log<RC::LogLevel::Normal>(TEXT("Applied deferred changes to DataAsset {}\n"),
                        obj->GetNamePrivate().ToString());
                }
                catch (const std::exception& e)
                {
                    PS::Log<RC::LogLevel::Error>(TEXT("Failed deferred patch for '{}': {}\n"),
                        path, RC::to_generic_string(e.what()));
                }
                it = m_deferredDataAssetMods.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (m_deferredDataAssetMods.empty())
        {
            m_hasDeferredDataAssets = false;
        }
    }

    std::vector<WindroseBlueprintMod>& WindroseBlueprintModLoader::GetModsForBlueprint(const RC::Unreal::FName& name)
    {
        auto it = m_modsMap.find(name);
        if (it != m_modsMap.end())
        {
            return it->second;
        }

        throw std::runtime_error(RC::fmt("Failed to get mods for this blueprint. Affected mod name: %S", name.ToString().c_str()));
    }

    void WindroseBlueprintModLoader::ModifyObject(RC::Unreal::UObject* object)
    {
        if (!object) return;
        auto objectClass = object->GetClassPrivate();
        auto& objectName = objectClass->GetNamePrivate();

        if (!m_modsMap.contains(objectName))
        {
            return;
        }

        auto& mods = GetModsForBlueprint(objectName);
        for (auto& mod : mods)
        {
            try
            {
                ApplyMod(mod, object);
            }
            catch (const std::exception& e)
            {
                PS::Log<RC::LogLevel::Error>(TEXT("Failed modifying blueprint '{}', {}\n"), objectName.ToString(), RC::to_generic_string(e.what()));
            }
        }
    }

    void WindroseBlueprintModLoader::ApplyMod(const WindroseBlueprintMod& mod, UObject* object)
    {
        auto& data = mod.GetData();
        ApplyData(data, object);
    }

    void WindroseBlueprintModLoader::ApplyData(const nlohmann::json& data, RC::Unreal::UObject* object)
    {
        auto objectClass = object->GetClassPrivate();
        if (!objectClass)
        {
            throw std::runtime_error("Cannot apply data, object class was null.");
        }

        UECustom::UBlueprintGeneratedClass* blueprintClass = nullptr;
        if (objectClass->IsA(UECustom::UBlueprintGeneratedClass::StaticClass()))
        {
            blueprintClass = static_cast<UECustom::UBlueprintGeneratedClass*>(objectClass);
        }

        for (auto& [propertyName, propertyValue] : data.items())
        {
            auto propertyNameWide = RC::to_generic_string(propertyName);
            auto property = Windrose::PropertyHelper::GetPropertyByName(objectClass, propertyNameWide);
            
            if (!property)
            {
                PS::Log<RC::LogLevel::Warning>(TEXT("Property '{}' does not exist in {}\n"), propertyNameWide, objectClass->GetNamePrivate().ToString());
                continue;
            }

            if (auto objectProperty = CastField<FObjectProperty>(property))
            {
                auto objectValue = *property->ContainerPtrToValuePtr<UObject*>(object);
                if (!objectValue)
                {
                    // null object can be a component template only on BlueprintGeneratedClass defaults.
                    if (blueprintClass)
                    {
                        HandleInheritableComponent(blueprintClass, propertyNameWide, propertyValue);
                    }
                    else
                    {
                        PS::Log<LogLevel::Warning>(TEXT("Property '{}' in {} was null and couldn't be resolved as a blueprint component template.\n"),
                            propertyNameWide, objectClass->GetNamePrivate().ToString());
                    }
                }
                else
                {
                    // Object has a pointer assigned to it so we let PropertyHelper handle it.
                    PropertyHelper::CopyJsonValueToContainer(object, property, propertyValue);
                }
            }
            else
            {
                // Any other property values get handled here like Numeric, Bool, String, etc.
                PropertyHelper::CopyJsonValueToContainer(object, property, propertyValue);
            }
        }
    }

    void WindroseBlueprintModLoader::HandleInheritableComponent(UECustom::UBlueprintGeneratedClass* bpClass, const RC::StringType& componentName,
                                                         const nlohmann::json& componentData)
    {
        auto& bpClassName = bpClass->GetNamePrivate();

        if (!componentData.is_object())
        {
            PS::Log<LogLevel::Warning>(TEXT("{} failed to apply, provided JSON value wasn't an object\n"), bpClassName.ToString());
            return;
        }

        auto componentFullName = std::format(TEXT("{}_GEN_VARIABLE"), componentName);
        UObject* inheritableComponent = nullptr;

        auto inheritableComponentHandler = bpClass->GetInheritableComponentHandler();
        if (inheritableComponentHandler)
        {
            auto records = inheritableComponentHandler->GetRecords();
            for (auto& record : records)
            {
                if (record.ComponentTemplate.Get() == nullptr) continue;

                if (record.ComponentTemplate.Get()->GetName() == componentFullName)
                {
                    inheritableComponent = record.ComponentTemplate.Get();
                    break;
                }
            }
        }

        if (inheritableComponent)
        {
            ModifyComponent(inheritableComponent, componentData);
            return;
        }

        // Component wasn't inside Inheritable Components list, so check SimpleConstructionScript next.
        HandleNodeComponent(bpClass, componentFullName, componentData);
    }

    void WindroseBlueprintModLoader::HandleNodeComponent(UECustom::UBlueprintGeneratedClass* bpClass, const RC::StringType& componentName, const nlohmann::json& componentData)
    {
        auto simpleConstructionScript = bpClass->GetSimpleConstructionScript();
        if (!simpleConstructionScript)
        {
            return;
        }

        UObject* nodeComponent = nullptr;

        auto& nodes = simpleConstructionScript->GetAllNodes();
        for (auto& nodeElement : nodes)
        {
            auto nodeComponentTemplate = nodeElement->GetComponentTemplate();
            if (!nodeComponentTemplate)
            {
                continue;
            }

            if (nodeComponentTemplate->GetName() == componentName)
            {
                nodeComponent = nodeComponentTemplate;
                break;
            }
        }

        if (!nodeComponent)
        {
            return;
        }

        ModifyComponent(nodeComponent, componentData);
    }

    void WindroseBlueprintModLoader::ModifyComponent(RC::Unreal::UObject* component, const nlohmann::json& componentData)
    {
        for (auto& [innerKey, innerValue] : componentData.items())
        {
            auto componentPropertyName = RC::to_generic_string(innerKey);
            auto componentProperty = PropertyHelper::GetPropertyByName(component->GetClassPrivate(), componentPropertyName.c_str());
            if (!componentProperty)
            {
                PS::Log<LogLevel::Warning>(TEXT("Property {} doesn't exist in {}\n"), componentPropertyName, component->GetName());
                continue;
            }

            PropertyHelper::CopyJsonValueToContainer(component, componentProperty, innerValue);
        }
    }

    void WindroseBlueprintModLoader::PostLoad(RC::Unreal::UClass* self)
    {
        PostLoadHook.call(self);

        if (!PostLoadCallback)
        {
            return;
        }

        PostLoadCallback(self);
    }

    void WindroseBlueprintModLoader::PostInitComponents(RC::Unreal::AActor* self)
    {
        PostInitComponentsHook.call(self);

        if (!PostInitComponentsCallback)
        {
            return;
        }

        PostInitComponentsCallback(self);
    }
}