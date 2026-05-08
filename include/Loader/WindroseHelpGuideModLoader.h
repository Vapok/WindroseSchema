#pragma once

#include "Loader/WindroseModLoaderBase.h"
#include "SDK/Classes/WindroseNoteDataAsset.h"

namespace RC::Unreal {
    class UDataTable;
}

namespace Windrose {
	class WindroseHelpGuideModLoader : public WindroseModLoaderBase {
	public:
		WindroseHelpGuideModLoader();

		~WindroseHelpGuideModLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual void OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& engineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
	private:
		UWindroseNoteDataAsset* m_helpGuideDataAsset{};
		RC::Unreal::UDataTable* m_helpGuideDescTextTable{};

        void LoadHelpGuides(const nlohmann::json& data);

		void Add(const RC::Unreal::FName& NoteId, const nlohmann::json& Data);

		void Edit(const RC::Unreal::FName& NoteId, UWindroseNoteData* NoteData, const nlohmann::json& Data);

		void AddOrEditDescText(const RC::Unreal::FName& NoteId, const nlohmann::json& Data);

		void DeleteRelatedData(const RC::Unreal::FName& NoteId);
	};
}
