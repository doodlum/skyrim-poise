#include "Hooks/HitEventHandler.h"

#include "Hooks/PoiseAV.h"
#include "Storage/Settings.h"


float HitEventHandler::GetWeaponDamage(RE::TESObjectWEAP* a_weapon)
{
	auto settings = Settings::GetSingleton();
	for (int index = a_weapon->numKeywords - 1; index >= 0; index--) {
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

float HitEventHandler::GetUnarmedDamage(RE::Actor* a_actor)
{
	auto settings = Settings::GetSingleton();

	auto unarmedDamage = std::lerp(static_cast<float>(settings->JSONSettings["Weapons"]["Damage"]["HandToHandMelee"]), a_actor->GetActorValue(RE::ActorValue::kUnarmedDamage), settings->Damage.UnarmedSkillContribution);
	auto gauntlet = a_actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHands);

	return gauntlet ? std::lerp(unarmedDamage, unarmedDamage + gauntlet->weight, settings->Damage.GauntletWeightContribution) : unarmedDamage;
}

float HitEventHandler::GetShieldDamage(RE::TESObjectARMO* a_shield)
{
	auto settings = Settings::GetSingleton();
	auto shieldDamage = settings->JSONSettings["Weapons"]["Damage"]["Shield"];
	if (shieldDamage != nullptr)
		return std::lerp(static_cast<float>(shieldDamage), a_shield->weight, settings->Damage.WeightContribution);
	return a_shield->weight;
}

float HitEventHandler::GetMiscDamage()
{
	auto settings = Settings::GetSingleton();
	auto miscDamage = settings->JSONSettings["Weapons"]["Damage"]["Misc"];
	if (miscDamage != nullptr)
		return static_cast<float>(miscDamage);
	return 5.0f;
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
	} else if (hitData->weapon) {
		if (hitData->weapon->As<RE::TESObjectWEAP>()->IsHandToHandMelee()) {
			stagger = GetUnarmedDamage(aggressor) * settings->Damage.UnarmedMult;
		} else {
			stagger = GetWeaponDamage(hitData->weapon) * settings->Damage.MeleeMult;
		}
	} else if (hitData->skill == RE::ActorValue::kNone) {
		stagger = hitData->physicalDamage * settings->Damage.CreatureMult;
	} else if (hitData->skill == RE::ActorValue::kBlock) {
		auto leftHand = aggressor->GetEquippedObject(true);
		auto rightHand = aggressor->GetEquippedObject(false);
		if (leftHand && leftHand->formType == RE::FormType::Armor) {
			stagger = GetShieldDamage(leftHand->As<RE::TESObjectARMO>()) * settings->Damage.BashMult;
		} else if (rightHand && rightHand->formType == RE::FormType::Weapon) {
			stagger = GetWeaponDamage(rightHand->As<RE::TESObjectWEAP>()) * settings->Damage.BashMult;
		} else {
			stagger = GetMiscDamage() * settings->Damage.BashMult;
		}
	} else {
		logger::debug("Unknown attack");
		return 0.0f;
	}

	auto attackData = hitData->attackData ? hitData->attackData.get() : nullptr;
	if (attackData) {
		stagger *= 1 + attackData->data.staggerOffset;
		logger::debug(FMT_STRING("Stagger Mult {}"), 1 * attackData->data.staggerOffset);
	}

	float baseMult = 1.0f - hitData->percentBlocked;
	logger::debug(FMT_STRING("Percent Blocked {}"), hitData->percentBlocked);

	PoiseAV::ApplyPerkEntryPoint(34, aggressor, target, &baseMult);
	PoiseAV::ApplyPerkEntryPoint(33, target, aggressor, &baseMult);

	stagger *= baseMult;
	if (hitData->totalDamage && hitData->physicalDamage)
	stagger *= hitData->totalDamage / hitData->physicalDamage;
	stagger = stagger * min(1 - (target->armorRating * 0.12f + target->armorBaseFactorSum) / 100.0f, 0.8f); 

	return stagger;
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
