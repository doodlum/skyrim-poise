#pragma once

#include <SimpleIni.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

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

	struct
	{
		int StaggerMode{ 1 };
	} Modes;

	struct
	{
		float BaseMult{ 20 };
		float ArmorMult{ 0.5 };
		float ResistSlope{ 100.0f };
		float RegenRate{ 0.333f };
	} Health;

	struct
	{
		float BashMult{ 2 };
		float BowMult{ 1 };
		float CreatureMult{ 1 };
		float MeleeMult{ 1 };
		float UnarmedMult{ 1 };

		float ToPCMult{ 1.5 };
		float ToNPCMult{ 1 };

		float WeightContribution{ 0.5 };
		float GauntletWeightContribution{ 0.5 };
		float UnarmedSkillContribution{ 0.5 };
	} Damage;

	struct
	{
		bool SpecialBar{ true };
	} TrueHUD;

	json JSONSettings;

	float GetDamageMultiplier(RE::Actor* a_aggressor, RE::Actor* a_target);
	void  LoadGameSettings();
	void  LoadINI(const wchar_t* a_path);
	void  LoadJSON(const wchar_t* a_path);
	void  LoadSettings();

private:
	Settings() = default;
	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;

	~Settings() = default;

	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;
};
