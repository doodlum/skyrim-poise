#include "HitEventHandler.h"
#include "AVManager.h"
#include "Settings.h"
#include "PoiseAV.h"
#include "PoiseAVHUD.h"

float HitEventHandler::RecalculateStagger([[maybe_unused]] RE::Actor* target, [[maybe_unused]] RE::Actor* aggressor, [[maybe_unused]] RE::HitData& hitData)
{
	float stagger = 0.0;

	auto sourceRef = hitData.sourceRef.get().get();
	if (sourceRef) {
		if (sourceRef->AsProjectile() && sourceRef->AsProjectile()->ammoSource && sourceRef->AsProjectile()->weaponSource) {
			stagger += sourceRef->AsProjectile()->weaponSource->GetWeight();
			stagger *= (1 + aggressor->GetActorValue(RE::ActorValue::kBowStaggerBonus));
		} else
			logger::info("idk");
	} else if (hitData.weapon) {
		stagger += hitData.weapon->GetWeight();
	} else if (hitData.skill == RE::ActorValue::kBlock) {
		stagger += aggressor->GetEquippedObject(true)->As<RE::TESObject>()->GetWeight();
	} else
		logger::info("idk");

	if (hitData.attackData) 
		stagger *= hitData.attackData.get()->data.damageMult;

	float multiplier = 1.0f - hitData.percentBlocked;

	ApplyPerkEntryPoint(34, aggressor->As<RE::Character>(), target->As<RE::Character>(), &multiplier);
	ApplyPerkEntryPoint(33, target->As<RE::Character>(), aggressor->As<RE::Character>(), &multiplier);

	return stagger * multiplier;
}

//static bool IsCasting(RE::Actor* a_actor)
//{
//	return std::any_of(
//		a_actor->magicCasters,
//		a_actor->magicCasters + RE::Actor::SlotTypes::kTotal,
//		[a_actor](auto cast) {
//			return cast && a_actor->IsCasting(cast->currentSpell);
//		});
//}

//static bool ActorDoingSomething(RE::Actor* a_actor) 
//{
//	return a_actor->GetAttackingWeapon() || a_actor->IsBlocking() || a_actor->IsRunning() || IsCasting(a_actor) || a_actor;
//}

static void DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage)
{
	if (a_poiseDamage <= 0.0f)
		return;
	auto avManager = AVManager::GetSingleton();
	avManager->mtx.lock();
	avManager->DamageActorValue(PoiseAV::g_avName, a_target, a_poiseDamage);
	auto poise = avManager->GetActorValue(PoiseAV::g_avName, a_target);
	if (poise <= 0.50f) {
		// Stagger duration is relative to the power of the attacking weapon
		auto poiseDamagePercent = (a_poiseDamage / avManager->GetActorValueMax(PoiseAV::g_avName, a_target)) / 100;
		PoiseAV::TryStagger(a_target, 0.5f + poiseDamagePercent, a_aggressor);
	} /*else if (a_target->IsPlayerRef() || !ActorDoingSomething(a_target)) {
		auto poiseDamagePercent = (a_poiseDamage / avManager->GetActorValueMax(PoiseAV::g_avName, a_target)) / 100;
		PoiseAV::TryStagger(a_target, min(poiseDamagePercent / 2, 0.5f), a_aggressor);
	}*/
	avManager->mtx.unlock();
}

void HitEventHandler::PreProcessVanillaStaggerAttempt(RE::Actor* target, [[maybe_unused]] RE::Actor* aggressor, float& stagger)
{
	DamageAndCheckPoise(target, aggressor, stagger * Settings::GetSingleton()->GameSetting.fPoiseDamageStaggerMult);
	stagger = 0.0f;
}

void HitEventHandler::PreProcessHitEvent(RE::Actor* target, [[maybe_unused]] RE::HitData& hitData)
{
	if (target == hitData.aggressor.get().get())
		return;

	auto poiseDamage = RecalculateStagger(target, hitData.aggressor.get().get(), hitData);

	DamageAndCheckPoise(target, hitData.aggressor.get().get(), poiseDamage * Settings::GetSingleton()->GameSetting.fPoiseDamageHitMult);

	hitData.stagger = static_cast<uint32_t>(0.00);
}
