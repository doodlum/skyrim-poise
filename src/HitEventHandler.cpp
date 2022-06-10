#include "HitEventHandler.h"
#include "AVManager.h"
#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

void HitEventHandler::DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage)
{
	auto avManager = AVManager::GetSingleton();
	avManager->mtx.lock();

	if (a_poiseDamage > 0 && a_target != a_aggressor)
		a_poiseDamage *= Settings::GetSingleton()->GameSetting.GetDamageMultiplier(a_aggressor, a_target);

	avManager->DamageActorValue(PoiseAV::g_avName, a_target, a_poiseDamage);
	auto poise = avManager->GetActorValue(PoiseAV::g_avName, a_target);
	if (poise == 0.0f) {
		auto poiseDamagePercent = a_poiseDamage / avManager->GetActorValueMax(PoiseAV::g_avName, a_target);
		// Stagger duration is relative to the power of the attacking weapon
		logger::debug(FMT_STRING("Poise Damage Percent {}"), poiseDamagePercent);
		PoiseAV::TryStagger(a_target, poiseDamagePercent, a_aggressor);
	}
	logger::debug(FMT_STRING("Target {} Poise Damage {} Poise Health {} / {}"), a_target->GetName(), a_poiseDamage, avManager->GetActorValue(PoiseAV::g_avName, a_target), avManager->GetActorValueMax(PoiseAV::g_avName, a_target));

	avManager->mtx.unlock();
}

float HitEventHandler::RecalculateStagger([[maybe_unused]] RE::Actor* target, [[maybe_unused]] RE::Actor* aggressor, [[maybe_unused]] RE::HitData& hitData)
{
	float stagger = 0.0;

	auto sourceRef = hitData.sourceRef.get().get();
	if (sourceRef) {
		if (sourceRef->AsProjectile() && sourceRef->AsProjectile()->ammoSource && sourceRef->AsProjectile()->weaponSource) {
			stagger += sourceRef->AsProjectile()->weaponSource->GetWeight() * Settings::GetSingleton()->GameSetting.fPoiseDamageBowMult;
			stagger += sourceRef->AsProjectile()->ammoSource->data.damage * Settings::GetSingleton()->GameSetting.fPoiseDamageArrowMult;
			stagger *= 1.0f + aggressor->GetActorValue(RE::ActorValue::kBowStaggerBonus);
			logger::debug(FMT_STRING("Bow stagger bonus {}"), aggressor->GetActorValue(RE::ActorValue::kBowStaggerBonus));
		} else
			logger::debug("Missed attack with sourceRef");
	} else if (hitData.skill == RE::ActorValue::kUnarmedDamage) {
		stagger += aggressor->GetActorValue(RE::ActorValue::kUnarmedDamage) * Settings::GetSingleton()->GameSetting.fPoiseDamageUnarmedMult;
	} else if (hitData.weapon) {
		stagger += hitData.weapon->GetWeight() * Settings::GetSingleton()->GameSetting.fPoiseDamageMeleeMult;
		logger::debug(FMT_STRING("Weapon Weight {}"), hitData.weapon->GetWeight());
	} else if (hitData.skill == RE::ActorValue::kBlock && aggressor->GetEquippedObject(true)) {
		stagger += aggressor->GetEquippedObject(true)->As<RE::TESObject>()->GetWeight() * Settings::GetSingleton()->GameSetting.fPoiseDamageBashMult;
		logger::debug(FMT_STRING("Object Weight {}"), stagger);
	} else
		logger::debug("Missed attack");

	if (hitData.attackData) {
		stagger *= hitData.attackData.get()->data.damageMult;
		logger::debug(FMT_STRING("Damage Mult {}"), hitData.attackData.get()->data.damageMult);
	}

	float baseMult = 1.0f - hitData.percentBlocked;
	logger::debug(FMT_STRING("Percent Blocked {}"), hitData.percentBlocked);

	ApplyPerkEntryPoint(34, aggressor->As<RE::Character>(), target->As<RE::Character>(), &baseMult);
	ApplyPerkEntryPoint(33, target->As<RE::Character>(), aggressor->As<RE::Character>(), &baseMult);

	return stagger * baseMult;
}

void HitEventHandler::PreProcessVanillaStaggerAttempt([[maybe_unused]] RE::Actor* target, [[maybe_unused]] RE::Actor* aggressor, float& stagger)
{
	if (!target->actorState2.staggered || Settings::GetSingleton()->GameSetting.bPoiseAllowStaggerLock)
		DamageAndCheckPoise(target, aggressor, stagger * Settings::GetSingleton()->GameSetting.fPoiseDamageStaggerMult);
	stagger = 0.0f;
}

void HitEventHandler::PreProcessHitEvent(RE::Actor* target, [[maybe_unused]] RE::HitData& hitData)
{
	if (!target->actorState2.staggered || Settings::GetSingleton()->GameSetting.bPoiseAllowStaggerLock) {
		auto poiseDamage = RecalculateStagger(target, hitData.aggressor.get().get(), hitData);
		DamageAndCheckPoise(target, hitData.aggressor.get().get(), poiseDamage);
	}

	hitData.stagger = static_cast<uint32_t>(0.00);
}
