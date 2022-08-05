#include "Hooks/ActiveEffectHandler.h"

#include "Hooks/PoiseAV.h"
#include "Storage/Settings.h"

float ActiveEffectHandler::CalculateEffectMultiplier(RE::ActorValue a_actorValue, bool a_detrimental)
{
	auto        settings = Settings::GetSingleton();
	std::string sEffectType = a_detrimental ? "Damage" : "Recovery";
	std::string baseAVString = std::string(magic_enum::enum_name(a_actorValue).substr(1)).c_str();
	auto        actorValue = settings->JSONSettings["Magic Effects"]["Actor Values"][sEffectType][baseAVString];
	return actorValue != nullptr ? static_cast<float>(actorValue) : 0.0f;
}

void ActiveEffectHandler::ProcessValueModifier(RE::Actor* a_target, RE::ActorValue a_actorValue, float a_magnitudeDelta, RE::Actor* a_aggressor)
{
	auto poiseAV = PoiseAV::GetSingleton();
	auto settings = Settings::GetSingleton();

	if (a_target != a_aggressor && poiseAV->CanDamageActor(a_target) && a_magnitudeDelta != 0) {
		float poiseDamage = CalculateEffectMultiplier(a_actorValue, a_magnitudeDelta > 0) * a_magnitudeDelta;

		if (a_aggressor) {
			float baseMult = 1.0f;
			PoiseAV::ApplyPerkEntryPoint(34, a_aggressor->As<RE::Character>(), a_target->As<RE::Character>(), &baseMult);
			PoiseAV::ApplyPerkEntryPoint(33, a_target->As<RE::Character>(), a_aggressor->As<RE::Character>(), &baseMult);
			poiseDamage *= baseMult;
			if (poiseDamage > 0)
			{
				poiseDamage *= settings->GetDamageMultiplier(a_aggressor, a_target);
				if (a_target != a_aggressor) {
					if (a_target->IsPlayerRef())
						poiseDamage *= settings->Damage.ToPCMult;
					else
						poiseDamage *= settings->Damage.ToNPCMult;
				}
			}
		}

		poiseAV->DamageAndCheckPoise(a_target, a_aggressor, poiseDamage);
	}
}


