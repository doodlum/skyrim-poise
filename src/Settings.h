#pragma once

#include <SimpleIni.h>

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
		constexpr auto path = L"Data/SKSE/Plugins/PoiseAV.ini";

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(path);

		GameSetting.Load(ini);
	}

	struct
	{
		void Load(CSimpleIniA& a_ini)
		{
			static const char* section = "Multipliers";

			fPoiseBase = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseBase", 100.0f));
			fPoiseMassFactor = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseMassFactor", 0.5f));
			fPoisePlayerMassMult = static_cast<float>(a_ini.GetDoubleValue(section, "fPoisePlayerMassMult", 2.5f));

			fPoiseArmorFactor = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseArmorFactor", 1.0f));
			
			fPoiseRegenDelayMax = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseRegenDelayMax", 3.0f));
			fPoiseRegenRate = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseRegenRate", 0.1f));

			fPoiseDamageAmmoBase = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseDamageAmmoBase", 10.0f));
			fPoiseDamageBashBase = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseDamageBashBase", 1.5f));

			fPoiseDamageHitMult = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseDamageHitMult", 1.0f));
			fPoiseDamageSpellMult = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseDamageSpellMult", 50.0f));
			fPoiseDamageStaggerMult = static_cast<float>(a_ini.GetDoubleValue(section, "fPoiseDamageStaggerMult", 10.0f));


		}

		float fPoiseBase;
		float fPoiseMassFactor;
		float fPoisePlayerMassMult;

		float fPoiseArmorFactor;

		float fPoiseRegenDelayMax;
		float fPoiseRegenRate;

		float fPoiseDamageAmmoBase;
		float fPoiseDamageBashBase;

		float fPoiseDamageHitMult;
		float fPoiseDamageSpellMult;
		float fPoiseDamageStaggerMult;

	} GameSetting;


private:
	Settings() = default;
	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;

	~Settings() = default;

	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;
};
