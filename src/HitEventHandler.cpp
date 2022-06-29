#include "HitEventHandler.h"
#include "AVManager.h"
#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float HitEventHandler::RecalculateStagger([[maybe_unused]] RE::Actor* target, [[maybe_unused]] RE::Actor* aggressor, [[maybe_unused]] RE::HitData& hitData)
{
	auto  settings = Settings::GetSingleton();
	float stagger = 0.0;

	auto sourceRef = hitData.sourceRef.get().get();
	if (sourceRef) {
		if (sourceRef->AsProjectile() && sourceRef->AsProjectile()->ammoSource && sourceRef->AsProjectile()->weaponSource) {
			stagger = sourceRef->AsProjectile()->weaponSource->GetWeight() * settings->GameSetting.fPoiseDamageBowMult;
			stagger += sourceRef->AsProjectile()->ammoSource->data.damage * settings->GameSetting.fPoiseDamageArrowMult;
			stagger *= 1.0f + aggressor->GetActorValue(RE::ActorValue::kBowStaggerBonus);
			logger::debug(FMT_STRING("Bow stagger bonus {}"), aggressor->GetActorValue(RE::ActorValue::kBowStaggerBonus));
		} else
			logger::debug("Missed attack with sourceRef");
	} else if (hitData.skill == RE::ActorValue::kUnarmedDamage) {
		stagger = aggressor->GetActorValue(RE::ActorValue::kUnarmedDamage) * settings->GameSetting.fPoiseDamageUnarmedMult;
	} else if (hitData.skill == RE::ActorValue::kNone) {
		stagger = hitData.physicalDamage * settings->GameSetting.fPoiseDamageCreatureMult;
	} else if (hitData.weapon) {
		stagger = hitData.weapon->GetWeight() * settings->GameSetting.fPoiseDamageMeleeMult;
		logger::debug(FMT_STRING("Weapon Weight {}"), hitData.weapon->GetWeight());
	} else if (hitData.skill == RE::ActorValue::kBlock && aggressor->GetEquippedObject(true)) {
		stagger = aggressor->GetEquippedObject(true)->As<RE::TESObject>()->GetWeight() * settings->GameSetting.fPoiseDamageBashMult;
		logger::debug(FMT_STRING("Object Weight {}"), stagger);
	} else
		logger::debug("Unknown attack");

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

void HitEventHandler::PreProcessHit(RE::Actor* target, [[maybe_unused]] RE::HitData& hitData)
{
	auto settings = Settings::GetSingleton();
	if (target->currentProcess && (!target->actorState2.staggered || settings->GameSetting.bPoiseAllowStaggerLock)) {
		auto poiseDamage = RecalculateStagger(target, hitData.aggressor.get().get(), hitData);
		PoiseAV::GetSingleton()->DamageAndCheckPoise(target, hitData.aggressor.get().get(), poiseDamage);
	}
	hitData.stagger = static_cast<uint32_t>(0.00);
}
