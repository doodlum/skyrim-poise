#include "Hooks/PoiseAV.h"

#include "ActorValues/AVManager.h"
#include "Storage/ActorCache.h"
#include "Storage/Settings.h"
#include "UI/PoiseAVHUD.h"

bool PoiseAV::CanDamageActor(RE::Actor* a_actor)
{
	if (a_actor && a_actor->currentProcess && !a_actor->IsChild()) {
		switch (Settings::GetSingleton()->Modes.StaggerMode) {
		case 0:
			return true;
		case 1:
			return !a_actor->actorState2.staggered;
		}
	}
	return false;
}

float PoiseAV::GetBaseActorValue(RE::Actor* a_actor)
{
	auto        settings = Settings::GetSingleton();
	std::string editorID = a_actor->GetRace()->GetFormEditorID();

	float health;
	if (!editorID.empty() && (settings->JSONSettings["Races"][editorID] != nullptr))
		health = (float)settings->JSONSettings["Races"][editorID];
	else
		health = a_actor->GetBaseActorValue(RE::ActorValue::kMass);

	health *= settings->Health.BaseMult;
	health += ActorCache::GetSingleton()->GetOrCreateCachedWeight(a_actor) * Settings::GetSingleton()->Health.ArmorMult;

	//if (auto levelledModifier = (RE::ExtraLevCreaModifier*)a_actor->extraList.GetByType(RE::ExtraDataType::kLevCreaModifier)) {
	//	auto modifier = levelledModifier->modifier;
	//	if (modifier.any(RE::LEV_CREA_MODIFIER::kEasy)) {
	//		health *= 0.75;
	//	} else if (modifier.any(RE::LEV_CREA_MODIFIER::kMedium)) {
	//		health *= 1.00;
	//	} else if (modifier.any(RE::LEV_CREA_MODIFIER::kHard)) {
	//		health *= 1.25;
	//	} else if (modifier.any(RE::LEV_CREA_MODIFIER::kVeryHard)) {
	//		health *= 1.50;
	//	}
	//}

	return std::clamp(health, 0.0f, FLT_MAX);
}

float PoiseAV::GetActorValueMax([[maybe_unused]] RE::Actor* a_actor)
{
	return GetBaseActorValue(a_actor);
}

void PoiseAV::DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage)
{
	auto                               settings = Settings::GetSingleton();
	auto                               avManager = AVManager::GetSingleton();
	std::lock_guard<std::shared_mutex> lk(avManager->mtx);

//	if (a_poiseDamage > 0 && a_target != a_aggressor) {
//	a_poiseDamage *= settings->GetDamageMultiplier(a_aggressor, a_target);
		if (a_target != a_aggressor) {
			if (a_target->IsPlayerRef())
				a_poiseDamage *= settings->Damage.ToPCMult;
			else
				a_poiseDamage *= settings->Damage.ToNPCMult;
		}
	}

	avManager->DamageActorValue(g_avName, a_target, a_poiseDamage);
	auto poise = avManager->GetActorValue(g_avName, a_target);
	if (poise == 0.0f) {
		a_target->AddToFaction(ForceFullBodyStagger, 0);
		auto poiseDamagePercent = a_poiseDamage / avManager->GetActorValueMax(g_avName, a_target);
		// Stagger duration is relative to the power of the attacking weapon
		logger::debug(FMT_STRING("Poise Damage Percent {}"), poiseDamagePercent);
		TryStagger(a_target, poiseDamagePercent, a_aggressor);
	}
	logger::debug(FMT_STRING("Target {} Poise Damage {} Poise Health {} / {}"), a_target->GetName(), a_poiseDamage, avManager->GetActorValue(g_avName, a_target), avManager->GetActorValueMax(g_avName, a_target));
}

void PoiseAV::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
{
	if (a_actor->currentProcess && a_actor->currentProcess->InHighProcess() && a_actor->Is3DLoaded()) {
		auto settings = Settings::GetSingleton();

		if (PoiseAVHUD::trueHUDInterface && settings->TrueHUD.SpecialBar) {
			if (!CanDamageActor(a_actor))
				PoiseAVHUD::trueHUDInterface->OverrideSpecialBarColor(a_actor->GetHandle(), TRUEHUD_API::BarColorType::BarColor, 0x808080);
			else
				PoiseAVHUD::trueHUDInterface->RevertSpecialBarColor(a_actor->GetHandle(), TRUEHUD_API::BarColorType::BarColor);
		}

		auto                               avManager = AVManager::GetSingleton();
		std::lock_guard<std::shared_mutex> lk(avManager->mtx);
		if (avManager->GetActorValue(g_avName, a_actor) == 0.0f) {
			if (a_actor->actorState2.staggered) {
				avManager->RestoreActorValue(g_avName, a_actor, FLT_MAX);
				if (PoiseAVHUD::trueHUDInterface && settings->TrueHUD.SpecialBar) {
					PoiseAVHUD::trueHUDInterface->FlashActorSpecialBar(SKSE::GetPluginHandle(), a_actor->GetHandle(), true);
				}
				RemoveFromFaction(a_actor, ForceFullBodyStagger);
			} else {
				TryStagger(a_actor, 0.5f, nullptr);
			}
		} else {
			avManager->RestoreActorValue(g_avName, a_actor, avManager->GetActorValueMax(g_avName, a_actor) * settings->Health.RegenRate * a_delta);
		}
	}
}

void PoiseAV::GarbageCollection()
{
	auto                               avManager = AVManager::GetSingleton();
	std::lock_guard<std::shared_mutex> lk(avManager->mtx);

	json temporaryJson = avManager->avStorage;
	for (auto& el : avManager->avStorage.items()) {
		std::string sformID = el.key();
		try {
			if (auto form = RE::TESForm::LookupByID(static_cast<RE::FormID>(std::stoul(sformID)))) {
				if (auto actor = RE::TESForm::LookupByID(static_cast<RE::FormID>(std::stoul(sformID)))->As<RE::Actor>()) {
					if (actor->currentProcess && actor->currentProcess->InHighProcess() && actor->Is3DLoaded())
						continue;
				}
			}
			temporaryJson.erase(sformID);
		} catch (std::invalid_argument const&) {
			logger::error("Bad input: std::invalid_argument thrown");
		} catch (std::out_of_range const&) {
			logger::error("Integer overflow: std::out_of_range thrown");
		}
	}
	avManager->avStorage = temporaryJson;
}
