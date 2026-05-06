#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

namespace Palworld {
    class SignatureManager {
    public:
        static void Initialize();
        
        // Expected parameter format: [CLASS]::[FUNCTION] or [FUNCTION], for example AGameModeBase::InitGameState or AsyncTask
        static void* GetSignature(const std::string& ClassAndFunction);
    private:
        static inline std::unordered_map<std::string, void*> SignatureMap;

        static inline std::unordered_map<std::string, std::string> Signatures {
            // Blueprint Loader apply logic
            { "UBlueprintGeneratedClass::PostLoadDefaultObject", "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8D 99 38 03 00 00" },
            // Raw Table apply logic
            { "FPakPlatformFile::GetPakFolders", "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 74 24 20 55 48 8B EC 48 83 EC 40 48 8D 4D F0 48 8B DA" },
            // Important so we can easily run things on the Game Thread
            { "AsyncTask", "48 8B C4 56 48 83 EC 60 48 89 58 08 48 89 68 18 8B" },
            // UE4SS has StaticFindObject, but this lets us use it earlier.
            { "UObjectGlobals::StaticFindObject", "48 89 5C 24 18 55 56 57 41 54 41 55 41 56 41 57 48 81 EC 80 04 00 00" },
            // I had issues using the IsA provided by UE4SS due to early init, so I switched to using Unreal's own.
            { "FField::IsA", "48 8B 41 08 48 8B 4A 08 48 85 C9 74 08 48 85 48 10 0F 95 C0 C3" },
            // Important, we need this early.
            { "FName::Constructor", "48 89 5C 24 08 57 48 83 EC 30 48 8B D9 41 8B F8 33 C9 4C 8B DA 44 8B D1 4C 8B CA 48 85 D2" },
            // FMemory::Free for GMalloc
            { "FMemory::Free", "48 85 C9 74 2E 53 48 83 EC 20 48 8B D9 48 8B ?? ?? ?? ?? ?? 48 85 C9 75 0C E8 ?? ?? ?? ?? 48 8B" },
            // Raw Tables
            { "UDataTable::Serialize", "48 89 5C 24 18 57 48 81 EC E0 01 00 00 48 8B ?? ?? ?? ?? ?? 48 33 C4 48 89 84 24 D8 01 00 00" },
        };
        static inline std::unordered_map<std::string, std::string> SignaturesCallResolve {
            { "FFieldClass::GetNameToFieldClassMap", "E8 ?? ?? ?? ?? 48 8B 5C 24 50 48 8B E8 8B 4C 24 50 48 8B FB 48 C1 EF 20" },
            // Important, we need this early.
            { "FName::ToString_Wchar", "E8 ?? ?? ?? ?? BE 01 00 00 00 39 75 48 0F 8E ?? ?? ?? ?? 4C 89 B4 24 80 00 00 00" },
            // More efficient object lookup for certain cases.
            { "GetObjectsOfClass", "E8 ?? ?? ?? ?? 4C 89 36 48 8D 4D D7 48 8B D3 4C 89 76 08" },
        };
    };
}