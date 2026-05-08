#include "Loader/WindroseResourceLoader.h"
#include "Unreal/FString.hpp"
#include "Unreal/NameTypes.hpp"
#include "SDK/Classes/KismetRenderingLibrary.h"
#include "SDK/Classes/KismetSystemLibrary.h"
#include "Utility/Logging.h"
#include "Helpers/String.hpp"
#include <set>

using namespace RC;
using namespace RC::Unreal;
namespace fs = std::filesystem;

namespace Windrose {
    WindroseResourceLoader::WindroseResourceLoader() : WindroseModLoaderBase("resources")
    {
    }
    WindroseResourceLoader::~WindroseResourceLoader()
    {
    }

    void WindroseResourceLoader::OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase != EEngineLifecyclePhase::GameInstanceInit)
        {
            return;
        }

        UnregisterResourceAssets(modName);

        LoadImages(modName, loaderPath);
    }

    void WindroseResourceLoader::OnAutoReload(const std::filesystem::path::string_type& modName, const std::filesystem::path& modFilePath)
    {
        UnregisterResourceAssetByFilePath(modName, modFilePath);
        LoadImage(modName, modFilePath);
    }

    bool WindroseResourceLoader::CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        if (engineLifecyclePhase == EEngineLifecyclePhase::GameInstanceInit)
        {
            return true;
        }

        return false;
    }

    bool WindroseResourceLoader::OnInitialize()
    {
        return true;
    }

    void WindroseResourceLoader::RegisterResourceAsset(const std::filesystem::path::string_type& modName, RC::Unreal::UObject* resource)
    {
        auto modResourcesIt = m_loadedResourcesMap.find(modName);
        if (modResourcesIt != m_loadedResourcesMap.end())
        {
            auto& modResources = modResourcesIt->second;
            modResources.push_back(resource);
        }
        else
        {
            std::vector<UObject*> newResourcesContainer;
            newResourcesContainer.push_back(resource);
            m_loadedResourcesMap.emplace(modName, newResourcesContainer);
        }
    }

    void WindroseResourceLoader::UnregisterResourceAsset(RC::Unreal::UObject* resource)
    {
        // We have to rename here, because CollectGarbage happens at the end of a frame and from testing, it happens after we add a new resource -
        // which isn't quick enough. You're not allowed to rename an asset to something that already exists, otherwise UE will crash.
        auto tempName = std::format(TEXT("{}-Temp"), resource->GetFullName());
        resource->Rename(tempName.c_str());
        resource->ClearRootSet();
    }

    void WindroseResourceLoader::UnregisterResourceAssetByFilePath(const std::filesystem::path::string_type& modName, const std::filesystem::path& modFilePath)
    {
        auto loadedResourcesIt = m_loadedResourcesMap.find(modName);
        if (loadedResourcesIt == m_loadedResourcesMap.end())
        {
            return;
        }

        auto fileName = modFilePath.stem().native();

        auto& loadedResources = loadedResourcesIt->second;
        std::erase_if(loadedResources, [&](RC::Unreal::UObject* loadedResource) {
            auto loadedResourceName = loadedResource->GetName();
            if (loadedResourceName.ends_with(fileName))
            {
                UnregisterResourceAsset(loadedResource);
                return true;
            }

            return false;
        });

        // Manually calling GC here. It's fine because UnregisterResourceAssets only happens when auto-reloading or reloading WindroseSchema mods.
        UECustom::UKismetSystemLibrary::CollectGarbage();
    }

    void WindroseResourceLoader::UnregisterResourceAssets(const std::filesystem::path::string_type& modName)
    {
        auto loadedResourcesIt = m_loadedResourcesMap.find(modName);
        if (loadedResourcesIt != m_loadedResourcesMap.end())
        {
            auto& loadedResources = loadedResourcesIt->second;
            for (auto& loadedResource : loadedResources)
            {
                UnregisterResourceAsset(loadedResource);
            }
            loadedResources.clear();

            // Manually calling GC here. It's fine because UnregisterResourceAssets only happens when auto-reloading or reloading WindroseSchema mods.
            UECustom::UKismetSystemLibrary::CollectGarbage();
        }
    }

    void WindroseResourceLoader::UnregisterResourceAssets()
    {
        for (auto& [modName, loadedResources] : m_loadedResourcesMap)
        {
            for (auto& loadedResource : loadedResources)
            {
                UnregisterResourceAsset(loadedResource);
            }
            loadedResources.clear();
        }

        // Manually calling GC here. It's fine because UnregisterResourceAssets only happens when auto-reloading or reloading WindroseSchema mods.
        UECustom::UKismetSystemLibrary::CollectGarbage();
    }

    void WindroseResourceLoader::LoadImages(const std::filesystem::path::string_type& modName, const std::filesystem::path& resourcesPath)
    {
        auto imagesPath = resourcesPath / "images";
        if (!fs::is_directory(imagesPath))
        {
            return;
        }

        for (const auto& file : fs::directory_iterator(imagesPath)) {
            try
            {
                auto& filePath = file.path();
                LoadImage(modName, filePath);
            }
            catch (const std::exception&)
            {
                throw;
            }
        }
    }

    void WindroseResourceLoader::LoadImage(const std::filesystem::path::string_type& modName, const std::filesystem::path& imagePath)
    {
        // More formats are supported by UE, but we should stick to the commonly used ones.
        const std::set<std::string> supportedExtensions = { ".png", ".jpg", ".jpeg", ".bmp", ".tga" };

        if (!imagePath.has_extension())
        {
            return;
        }

        auto extensionName = imagePath.extension().string();
        if (!supportedExtensions.count(extensionName))
        {
            return;
        }

        auto imageName = imagePath.stem().native();

        auto newTexture = UECustom::UKismetRenderingLibrary::ImportFileAsTexture2D(nullptr, FString(imagePath.c_str()));
        newTexture->SetRootSet();

        auto packagePath = std::format(TEXT("WindroseSchema/Resources/{}/{}"), modName, imageName);
        newTexture->Rename(packagePath.c_str()); // becomes "/Engine/Transient.WindroseSchema/Resources/modname/resourcename"

        RegisterResourceAsset(modName, newTexture);

        PS::Log<LogLevel::Normal>(STR("Registered Image Resource '{}'\n"), packagePath);
    }
}
