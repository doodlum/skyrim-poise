#include "Storage/Settings.h"

#include "UI/PoiseAVHUD.h"


float Settings::GetDamageMultiplier(RE::Actor* a_aggressor, RE::Actor* a_target)
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

void Settings::LoadGameSettings()
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
}

void Settings::LoadINI(const wchar_t* a_path)
{
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(a_path);

	Modes.StaggerMode = ini.GetLongValue("Modes", "StaggerMode", Modes.StaggerMode);

	Health.BaseMult = static_cast<float>(ini.GetDoubleValue("Health", "BaseMult", Health.BaseMult));
	Health.ArmorMult = static_cast<float>(ini.GetDoubleValue("Health", "ArmorMult", Health.ArmorMult));
	Health.ResistSlope = static_cast<float>(ini.GetDoubleValue("Health", "ResistSlope", Health.ResistSlope));
	Health.RegenRate = static_cast<float>(ini.GetDoubleValue("Health", "RegenRate", Health.RegenRate));

	Damage.BashMult = static_cast<float>(ini.GetDoubleValue("Damage", "BashMult", Damage.BashMult));
	Damage.BowMult = static_cast<float>(ini.GetDoubleValue("Damage", "BowMult", Damage.BowMult));
	Damage.CreatureMult = static_cast<float>(ini.GetDoubleValue("Damage", "CreatureMult", Damage.CreatureMult));
	Damage.MeleeMult = static_cast<float>(ini.GetDoubleValue("Damage", "MeleeMult", Damage.MeleeMult));

	Damage.UnarmedMult = static_cast<float>(ini.GetDoubleValue("Damage", "UnarmedMult", Damage.UnarmedMult));

	Damage.ToPCMult = static_cast<float>(ini.GetDoubleValue("Damage", "ToPCMult", Damage.ToPCMult));
	Damage.ToNPCMult = static_cast<float>(ini.GetDoubleValue("Damage", "ToNPCMult", Damage.ToNPCMult));

	Damage.WeightContribution = static_cast<float>(ini.GetDoubleValue("Damage", "WeightContribution", Damage.WeightContribution));
	Damage.GauntletWeightContribution = static_cast<float>(ini.GetDoubleValue("Damage", "GauntletWeightContribution", Damage.GauntletWeightContribution));
	Damage.UnarmedSkillContribution = static_cast<float>(ini.GetDoubleValue("Damage", "UnarmedSkillContribution", Damage.UnarmedSkillContribution));

	TrueHUD.SpecialBar = ini.GetBoolValue("TrueHUD", "SpecialBar", TrueHUD.SpecialBar) && (!GetModuleHandleA("valhallaCombat.dll") || ini.GetBoolValue("TrueHUD", "IgnoreValhallaCombat", false));
}

void Settings::LoadJSON(const wchar_t* a_path)
{
	std::ifstream i(a_path);
	i >> JSONSettings;
}

void Settings::LoadSettings()
{
	LoadGameSettings();
	LoadINI(L"Data/SKSE/Plugins/ChocolatePoise.ini");
	LoadJSON(L"Data/SKSE/Plugins/ChocolatePoise.json");

	if (PoiseAVHUD::trueHUDInterface) {
		if (PoiseAVHUD::trueHUDInterface->RequestSpecialResourceBarsControl(SKSE::GetPluginHandle()) == TRUEHUD_API::APIResult::OK) {
			if (TrueHUD.SpecialBar)
				PoiseAVHUD::trueHUDInterface->RegisterSpecialResourceFunctions(SKSE::GetPluginHandle(), PoiseAVHUD::GetCurrentSpecial, PoiseAVHUD::GetMaxSpecial, true);
			else
				PoiseAVHUD::trueHUDInterface->ReleaseSpecialResourceBarControl(SKSE::GetPluginHandle());
		}
	}
}
