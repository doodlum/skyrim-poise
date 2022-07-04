#include "HitEventHandler.h"
#include "AVManager.h"
#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float HitEventHandler::GetWeaponDamage(RE::TESObjectWEAP* a_weapon)
{
	auto settings = Settings::GetSingleton();
	for (auto index = a_weapon->numKeywords - 1; index >= 0; index--) {
		if (a_weapon->keywords[index]) {
			std::string keyword = a_weapon->keywords[index]->formEditorID.c_str();
			auto        pos = keyword.find("WeapType");
			if (pos != 0)
				continue;
			std::string type = keyword.substr(pos + 8, keyword.length());
			if (type == "Bow" && a_weapon->weaponData.animationType == RE::WEAPON_TYPE::kCrossbow)
				type = "Crossbow";
			if (!type.empty()) {
				auto weaponDamage = settings->JSONSettings["Weapons"]["Damage"][type];
				if (weaponDamage != nullptr)
					return std::lerp(static_cast<float>(weaponDamage), a_weapon->weight, settings->Damage.WeightContribution);
			}
		}
	}
	return a_weapon->weight;
}

float HitEventHandler::GetShieldDamage(RE::TESObjectARMO* a_shield)
{
	auto settings = Settings::GetSingleton();
	auto shieldDamage = settings->JSONSettings["Weapons"]["Damage"]["Shield"];
	if (shieldDamage != nullptr)
		return std::lerp(static_cast<float>(shieldDamage), a_shield->weight, settings->Damage.WeightContribution);
	return a_shield->weight;
}

float HitEventHandler::RecalculateStagger(RE::Actor* target, RE::Actor* aggressor, RE::HitData* hitData)
{
	auto  settings = Settings::GetSingleton();
	float stagger = 0.0;

	auto sourceRef = hitData->sourceRef.get().get();
	if (sourceRef) {
		if (sourceRef->AsProjectile() && sourceRef->AsProjectile()->ammoSource && sourceRef->AsProjectile()->weaponSource) {
			stagger = GetWeaponDamage(sourceRef->AsProjectile()->weaponSource) * settings->Damage.BowMult;
			stagger *= 1.0f + aggressor->GetActorValue(RE::ActorValue::kBowStaggerBonus);
		} else
			logger::debug("Missed attack with sourceRef");
	} else if (hitData->skill == RE::ActorValue::kUnarmedDamage) {
		stagger = aggressor->GetActorValue(RE::ActorValue::kUnarmedDamage) * settings->Damage.UnarmedMult;
	} else if (hitData->skill == RE::ActorValue::kNone) {
		stagger = hitData->physicalDamage * settings->Damage.CreatureMult;
	} else if (hitData->weapon) {
		stagger = GetWeaponDamage(hitData->weapon) * settings->Damage.MeleeMult;
	} else if (hitData->skill == RE::ActorValue::kBlock && aggressor->GetEquippedObject(true)) {
		stagger = GetShieldDamage(aggressor->GetEquippedObject(true)->As<RE::TESObjectARMO>()) * settings->Damage.BashMult;
	} else
		logger::debug("Unknown attack");

	auto attackData = hitData->attackData ? hitData->attackData.get() : nullptr;
	if (attackData) {
		stagger *= attackData->data.damageMult;
		logger::debug(FMT_STRING("Damage Mult {}"), attackData->data.damageMult);
	}

	float baseMult = 1.0f - hitData->percentBlocked;
	logger::debug(FMT_STRING("Percent Blocked {}"), hitData->percentBlocked);

	ApplyPerkEntryPoint(34, aggressor, target, &baseMult);
	ApplyPerkEntryPoint(33, target, aggressor, &baseMult);

	return stagger * baseMult;
}

void HitEventHandler::PreProcessHit(RE::Actor* target, RE::HitData* hitData)
{
	auto poiseAV = PoiseAV::GetSingleton();
	auto aggressor = hitData->aggressor ? hitData->aggressor.get().get() : nullptr;
	if (aggressor && poiseAV->CanDamageActor(target)) {
		auto poiseDamage = RecalculateStagger(target, aggressor, hitData);
		poiseAV->DamageAndCheckPoise(target, aggressor, poiseDamage);
	}
	hitData->stagger = static_cast<uint32_t>(0.00);
}
