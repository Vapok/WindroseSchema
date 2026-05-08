#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Loader/WindroseMainLoader.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/WindroseSignatures.h"
#include "SDK/Classes/Async.h"
#include "SDK/UnrealOffsets.h"
#include "../version.h"

using namespace RC;
using namespace RC::Unreal;

class WindroseSchema : public RC::CppUserModBase
{
public:
    WindroseSchema() : CppUserModBase()
    {
        auto Version = std::format(STR("{}.{}.{}"), VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);

        ModName = STR("WindroseSchema");
        ModVersion = Version;
        ModDescription = STR("Allows modifying of Windrose's assets dynamically.");
        ModAuthors = STR("Okaetsu");

        auto config = PS::PSConfig::Get();
        config->Load();

        PS::Log<LogLevel::Verbose>(STR("Initializing SignatureManager...\n"));
        Windrose::SignatureManager::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Initializing UnrealOffsets...\n"));
        Windrose::UnrealOffsets::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Preparing to pre-initialize WindroseSchema...\n"));
        MainLoader.PreInitialize();

        PS::Log<RC::LogLevel::Normal>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~WindroseSchema() override
    {
    }

    auto on_ui_init() -> void override
    {
        if (!UE4SSProgram::settings_manager.Debug.DebugConsoleEnabled)
        {
            return;
        }

        PS::Log<LogLevel::Verbose>(STR("GUI Console is enabled, enabling ImGui for WindroseSchema...\n"));

        UE4SS_ENABLE_IMGUI()

        PS::Log<LogLevel::Verbose>(STR("Registering Windrose Schema tab in GUI Console...\n"));
        register_tab(STR("Windrose Schema"), [](CppUserModBase* instance) {
            auto mod = dynamic_cast<WindroseSchema*>(instance);
            if (!mod)
            {
                return;
            }
        });

        PS::Log<LogLevel::Verbose>(STR("Finished registering Windrose Schema tab for GUI Console.\n"));
    }

    auto on_update() -> void override
    {
    }

    auto on_program_start() -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        MainLoader.Initialize();
    }
private:
    Windrose::WindroseMainLoader MainLoader;
};


#define WINDROSESCHEMA_API __declspec(dllexport)
extern "C"
{
    WINDROSESCHEMA_API RC::CppUserModBase* start_mod()
    {
        return new WindroseSchema();
    }

    WINDROSESCHEMA_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
