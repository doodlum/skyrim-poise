#pragma once

#include <SimpleIni.h>

namespace RE
{
	enum class DIFFICULTY : std::int32_t
	{
		kNovice = 0,
		kApprentice = 1,
		kAdept = 2,
		kExpert = 3,
		kMaster = 4,
		kLegendary = 5
	};
}

class Settings
{
public:
	[[nodiscard]] static Settings* GetSingleton()
	{
		static Settings singleton;
		return &singleton;
	}

	void LoadSettings()
	{
		GameSetting.Load(L"Data/SKSE/Plugins/ChocolatePoise.ini");
		EffectSetting.Load(L"Data/SKSE/Plugins/ChocolatePoise.json");
	}

	struct
	{
		float fDiffMultHPByPCVE;
		float fDiffMultHPByPCE;
		float fDiffMultHPByPCN;
		float fDiffMultHPByPCH;
		float fDiffMultHPByPCVH;
		float fDiffMultHPByPCL;
		float fDiffMultHPToPCVE;
		float fDiffMultHPToPCE;
		float fDiffMultHPToPCN;
		float fDiffMultHPToPCH;
		float fDiffMultHPToPCVH;
		float fDiffMultHPToPCL;

		float fPoiseHealthAVMult;
		float fPoiseHealthArmorMult;

		float fPoiseRegenRate;

		float fPoiseDamageBowMult;
		float fPoiseDamageArrowMult;
		float fPoiseDamageMeleeMult;
		float fPoiseDamageBashMult;
		float fPoiseDamageStaggerMult;
		float fPoiseDamageUnarmedMult;

		bool bPoiseAllowStaggerLock;
		bool bPoiseUseSpecialBar;

		void Load(const wchar_t* a_path)
		{
			auto gameSettingCollection = RE::GameSettingCollection::GetSingleton();

			fDiffMultHPByPCVE = gameSettingCollection->GetSetting("fDiffMultHPByPCVE")->GetFloat();
			fDiffMultHPByPCE = gameSettingCollection->GetSetting("fDiffMultHPByPCE")->GetFloat();
			fDiffMultHPByPCN = gameSettingCollection->GetSetting("fDiffMultHPByPCN")->GetFloat();
			fDiffMultHPByPCH = gameSettingCollection->GetSetting("fDiffMultHPByPCH")->GetFloat();
			fDiffMultHPByPCVH = gameSettingCollection->GetSetting("fDiffMultHPByPCVH")->GetFloat();
			fDiffMultHPByPCL = gameSettingCollection->GetSetting("fDiffMultHPByPCL")->GetFloat();
			fDiffMultHPToPCVE = gameSettingCollection->GetSetting("fDiffMultHPToPCVE")->GetFloat();
			fDiffMultHPToPCE = gameSettingCollection->GetSetting("fDiffMultHPToPCE")->GetFloat();
			fDiffMultHPToPCN = gameSettingCollection->GetSetting("fDiffMultHPToPCN")->GetFloat();
			fDiffMultHPToPCH = gameSettingCollection->GetSetting("fDiffMultHPToPCH")->GetFloat();
			fDiffMultHPToPCVH = gameSettingCollection->GetSetting("fDiffMultHPToPCVH")->GetFloat();
			fDiffMultHPToPCL = gameSettingCollection->GetSetting("fDiffMultHPToPCL")->GetFloat();

			CSimpleIniA ini;
			ini.SetUnicode();
			ini.LoadFile(a_path);

			static const char* section = "Multipliers";

			fPoiseHealthAVMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseHealthAVMult", 0));
			fPoiseHealthArmorMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseHealthArmorMult", 0));

			fPoiseRegenRate = static_cast<float>(ini.GetDoubleValue(section, "fPoiseRegenRate", 0));

			fPoiseDamageBowMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseDamageBowMult", 0));
			fPoiseDamageArrowMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseDamageArrowMult", 0));
			fPoiseDamageMeleeMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseDamageMeleeMult", 0));
			fPoiseDamageBashMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseDamageBashMult", 0));
			fPoiseDamageUnarmedMult = static_cast<float>(ini.GetDoubleValue(section, "fPoiseDamageUnarmedMult", 0));

			bPoiseAllowStaggerLock = ini.GetBoolValue(section, "bPoiseAllowStaggerLock", false);
			bPoiseUseSpecialBar = ini.GetBoolValue(section, "bPoiseUseSpecialBar", false);
		}

		float GetDamageMultiplier(RE::Actor* a_aggressor, RE::Actor* a_target)
		{
			if (a_aggressor && (a_aggressor->IsPlayerRef() || a_aggressor->IsPlayerTeammate())) {
				switch (static_cast<RE::DIFFICULTY>(RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty)) {
				case RE::DIFFICULTY::kNovice:
					return fDiffMultHPByPCVE;
				case RE::DIFFICULTY::kApprentice:
					return fDiffMultHPByPCE;
				case RE::DIFFICULTY::kAdept:
					return fDiffMultHPByPCN;
				case RE::DIFFICULTY::kExpert:
					return fDiffMultHPByPCH;
				case RE::DIFFICULTY::kMaster:
					return fDiffMultHPByPCVH;
				case RE::DIFFICULTY::kLegendary:
					return fDiffMultHPByPCL;
				}
			} else if (a_target && (a_target->IsPlayerRef() || a_target->IsPlayerTeammate())) {
				switch (static_cast<RE::DIFFICULTY>(RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty)) {
				case RE::DIFFICULTY::kNovice:
					return fDiffMultHPToPCVE;
				case RE::DIFFICULTY::kApprentice:
					return fDiffMultHPToPCE;
				case RE::DIFFICULTY::kAdept:
					return fDiffMultHPToPCN;
				case RE::DIFFICULTY::kExpert:
					return fDiffMultHPToPCH;
				case RE::DIFFICULTY::kMaster:
					return fDiffMultHPToPCVH;
				case RE::DIFFICULTY::kLegendary:
					return fDiffMultHPToPCL;
				}
			}
			return 1.0f;
		}

	} GameSetting;

	struct
	{
		void Load(const wchar_t* a_path)
		{
			std::ifstream i(a_path);
			i >> root;

			auto av = magic_enum::enum_cast<RE::ActorValue>('k' + (std::string)root["PoiseHealthBaseAV"]);
			if (av.has_value())
				PoiseHealthBaseAV = av.value();
			else
				PoiseHealthBaseAV = RE::ActorValue::kNone;
		}

		RE::ActorValue PoiseHealthBaseAV;
		json root;

	} EffectSetting;

private:
	Settings() = default;
	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;

	~Settings() = default;

	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;
};
