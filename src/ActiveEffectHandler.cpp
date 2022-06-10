#include "ActiveEffectHandler.h"
#include "AVManager.h"
#include "HitEventHandler.h"
#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float ActiveEffectHandler::CalculateAVEffectPoiseDamage([[maybe_unused]] RE::ActiveEffect* a_activeEffect, RE::ActorValue a_actorValue)
{
	if (a_actorValue == RE::ActorValue::kNone)
		return 0.0f;

	float poiseDamage = 0.0f;

	auto settings = Settings::GetSingleton();

	std::string avString = std::string(magic_enum::enum_name(a_actorValue).substr(1)).c_str();

	std::string sType = a_activeEffect->effect->baseEffect->IsDetrimental() ? "Damage" : "Recovery";

	if (settings->EffectSetting.root["Multipliers"][sType][avString] != nullptr)
		poiseDamage = static_cast<float>(settings->EffectSetting.root["Multipliers"][sType][avString]);
	else
		logger::debug(FMT_STRING("Could not find {} av"), avString);

	return poiseDamage;
}

void ActiveEffectHandler::ActiveEffectUpdate(RE::ActiveEffect* a_activeEffect, float a_delta)
{
	auto aggressor = a_activeEffect->GetCasterActor() ? a_activeEffect->GetCasterActor().get() : nullptr;
	auto target = a_activeEffect->GetTargetActor();

	if (aggressor && target && a_activeEffect->conditionStatus.get() != RE::ActiveEffect::ConditionStatus::kFalse) {
		float poiseDamage = CalculateAVEffectPoiseDamage(a_activeEffect, a_activeEffect->effect->baseEffect->data.primaryAV);
		poiseDamage *= -1 * a_activeEffect->magnitude;

		float baseMult = 1.0f;
		HitEventHandler::ApplyPerkEntryPoint(34, aggressor->As<RE::Character>(), target->As<RE::Character>(), &baseMult);
		HitEventHandler::ApplyPerkEntryPoint(33, target->As<RE::Character>(), aggressor->As<RE::Character>(), &baseMult);

		HitEventHandler::DamageAndCheckPoise(target, aggressor, poiseDamage * baseMult * (a_activeEffect->duration > 0.0f ? a_delta : 1.0f));
	}
}

void ActiveEffectHandler::DualActiveEffectUpdate(RE::ActiveEffect* a_activeEffect, float a_delta)
{
	auto aggressor = a_activeEffect->GetCasterActor() ? a_activeEffect->GetCasterActor().get() : nullptr;
	auto target = a_activeEffect->GetTargetActor();

	if (aggressor && target && a_activeEffect->conditionStatus.get() != RE::ActiveEffect::ConditionStatus::kFalse) {
		float poiseDamage = CalculateAVEffectPoiseDamage(a_activeEffect, a_activeEffect->effect->baseEffect->data.primaryAV);
		poiseDamage += CalculateAVEffectPoiseDamage(a_activeEffect, a_activeEffect->effect->baseEffect->data.secondaryAV) * (a_activeEffect->effect->baseEffect->data.secondAVWeight);
		poiseDamage *= -1 * a_activeEffect->magnitude;

		float baseMult = 1.0f;
		HitEventHandler::ApplyPerkEntryPoint(34, aggressor->As<RE::Character>(), target->As<RE::Character>(), &baseMult);
		HitEventHandler::ApplyPerkEntryPoint(33, target->As<RE::Character>(), aggressor->As<RE::Character>(), &baseMult);

		HitEventHandler::DamageAndCheckPoise(target, aggressor, poiseDamage * baseMult * (a_activeEffect->duration > 0.0f ? a_delta : 1.0f));
	}
}
