#include "ActiveEffectHandler.h"
#include "AVManager.h"
#include "HitEventHandler.h"
#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

bool IsValidEFfect(RE::ActiveEffect* a_activeEffect)
{
	return a_activeEffect->effect && a_activeEffect->spell && a_activeEffect->effect->baseEffect->data.castingType != RE::MagicSystem::CastingType::kConstantEffect && a_activeEffect->conditionStatus.get() != RE::ActiveEffect::ConditionStatus::kFalse;
}

float ActiveEffectHandler::CalculateAVEffectPoiseDamage([[maybe_unused]] RE::ActiveEffect* a_activeEffect, RE::ActorValue a_actorValue)
{
	auto settings = Settings::GetSingleton();

	float poiseDamage;

	std::string sEffectType = a_activeEffect->effect->baseEffect->IsDetrimental() ? "Damage" : "Recovery";
	std::string baseAVString = std::string(magic_enum::enum_name(a_actorValue).substr(1)).c_str();
	auto        actorValue = settings->JSONSettings["Magic Effects"]["Actor Values"][sEffectType][baseAVString];
	if (actorValue != nullptr)
		poiseDamage = static_cast<float>(actorValue);
	else
		return 0.0f;

	std::string resistAVString = std::string(magic_enum::enum_name(a_activeEffect->effect->baseEffect->data.resistVariable).substr(1)).c_str();
	auto        resistValue = settings->JSONSettings["Magic Effects"]["Resist Values"][resistAVString];
	if (resistValue != nullptr) {
		poiseDamage *= static_cast<float>(resistValue);
	}

	return poiseDamage;
}

void ActiveEffectHandler::ActiveEffectUpdate(RE::ActiveEffect* a_activeEffect, float a_delta)
{
	auto aggressor = a_activeEffect->GetCasterActor() ? a_activeEffect->GetCasterActor().get() : nullptr;
	auto target = a_activeEffect->GetTargetActor();

	auto poiseAV = PoiseAV::GetSingleton();

	if (target && aggressor && poiseAV->CanDamageActor(target) && IsValidEFfect(a_activeEffect)) {
		float poiseDamage = CalculateAVEffectPoiseDamage(a_activeEffect, a_activeEffect->effect->baseEffect->data.primaryAV);
		poiseDamage *= -1 * a_activeEffect->magnitude;

		float baseMult = 1.0f;
		HitEventHandler::ApplyPerkEntryPoint(34, aggressor->As<RE::Character>(), target->As<RE::Character>(), &baseMult);
		HitEventHandler::ApplyPerkEntryPoint(33, target->As<RE::Character>(), aggressor->As<RE::Character>(), &baseMult);

		poiseAV->DamageAndCheckPoise(target, aggressor, poiseDamage * baseMult * (a_activeEffect->duration > 0.0f ? a_delta : 1.0f));
	}
}

void ActiveEffectHandler::DualActiveEffectUpdate(RE::ActiveEffect* a_activeEffect, float a_delta)
{
	auto aggressor = a_activeEffect->GetCasterActor() ? a_activeEffect->GetCasterActor().get() : nullptr;
	auto target = a_activeEffect->GetTargetActor();

	auto poiseAV = PoiseAV::GetSingleton();

	if (target && aggressor && poiseAV->CanDamageActor(target) && IsValidEFfect(a_activeEffect)) {
		float poiseDamage = CalculateAVEffectPoiseDamage(a_activeEffect, a_activeEffect->effect->baseEffect->data.primaryAV);
		poiseDamage += CalculateAVEffectPoiseDamage(a_activeEffect, a_activeEffect->effect->baseEffect->data.secondaryAV) * (a_activeEffect->effect->baseEffect->data.secondAVWeight);
		poiseDamage *= -1 * a_activeEffect->magnitude;

		float baseMult = 1.0f;
		HitEventHandler::ApplyPerkEntryPoint(34, aggressor->As<RE::Character>(), target->As<RE::Character>(), &baseMult);
		HitEventHandler::ApplyPerkEntryPoint(33, target->As<RE::Character>(), aggressor->As<RE::Character>(), &baseMult);

		poiseAV->DamageAndCheckPoise(target, aggressor, poiseDamage * baseMult * (a_activeEffect->duration > 0.0f ? a_delta : 1.0f));
	}
}
