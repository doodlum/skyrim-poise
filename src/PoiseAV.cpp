#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float PoiseAV::GetBaseActorValue([[maybe_unused]] RE::Actor* a_actor)
{
	auto settings = Settings::GetSingleton();
	float poise;
	std::string editorID = a_actor->GetRace()->GetFormEditorID();

	if (!editorID.empty() && (settings->EffectSetting.root["Races"][editorID] != nullptr))
		poise = (float)settings->EffectSetting.root["Races"][editorID];
	else
		poise = a_actor->GetBaseActorValue(RE::ActorValue::kMass);

	poise *= settings->GameSetting.fPoiseHealthAVMult;
	poise += a_actor->equippedWeight * Settings::GetSingleton()->GameSetting.fPoiseHealthArmorMult;

	return poise;
}

float PoiseAV::GetActorValueMax([[maybe_unused]] RE::Actor* a_actor)
{
	return GetBaseActorValue(a_actor);
}

void PoiseAV::DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage)
{
	auto settings = Settings::GetSingleton();
	auto avManager = AVManager::GetSingleton();
	avManager->mtx.lock();

	if (a_poiseDamage > 0 && a_target != a_aggressor) {
		a_poiseDamage *= settings->GameSetting.GetDamageMultiplier(a_aggressor, a_target);
		if (a_target->IsPlayerRef()) {
			a_poiseDamage *= settings->GameSetting.fPoiseDamageToPCMult;
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

	avManager->mtx.unlock();
}

void PoiseAV::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
{
	auto settings = Settings::GetSingleton();
	auto avManager = AVManager::GetSingleton();
	auto g_trueHUD = PoiseAVHUD::GetSingleton()->g_trueHUD;

	if (g_trueHUD && !settings->GameSetting.bPoiseAllowStaggerLock) {
		if (a_actor->actorState2.staggered)
			g_trueHUD->OverrideSpecialBarColor(a_actor->GetHandle(), TRUEHUD_API::BarColorType::BarColor, 0x808080);
		else
			g_trueHUD->RevertSpecialBarColor(a_actor->GetHandle(), TRUEHUD_API::BarColorType::BarColor);
	}

	avManager->mtx.lock();
	if (avManager->GetActorValue(g_avName, a_actor) == 0.0f) {
		if (a_actor->actorState2.staggered) {
			avManager->RestoreActorValue(g_avName, a_actor, FLT_MAX);
			if (g_trueHUD) {
				g_trueHUD->FlashActorSpecialBar(SKSE::GetPluginHandle(), a_actor->GetHandle(), true);
			}
			RemoveFromFaction(a_actor, ForceFullBodyStagger);
		} else {
			TryStagger(a_actor, 0.5f, nullptr);
			if (g_trueHUD && !settings->GameSetting.bPoiseAllowStaggerLock)
				g_trueHUD->OverrideSpecialBarColor(a_actor->GetHandle(), TRUEHUD_API::BarColorType::BarColor, 0x808080);
		}
	} else {
		avManager->RestoreActorValue(g_avName, a_actor, avManager->GetActorValueMax(g_avName, a_actor) * settings->GameSetting.fPoiseRegenRate * a_delta);
	}
	avManager->mtx.unlock();
}

void PoiseAV::GarbageCollection() {
	auto avManager = AVManager::GetSingleton();
	avManager->mtx.lock();

	json temporaryJson = avManager->avStorage;
	for (auto& el : avManager->avStorage.items()) {
		std::string sformID = el.key();
		try {
			if (auto form = RE::TESForm::LookupByID(static_cast<RE::FormID>(std::stoul(sformID)))) {
				if (auto actor = RE::TESForm::LookupByID(static_cast<RE::FormID>(std::stoul(sformID)))->As<RE::Actor>()) {
					if (actor->currentProcess && actor->currentProcess->InHighProcess())
						continue;
				}
			}
			avManager->avStorage.erase(sformID);
		} catch (std::invalid_argument const&) {
			logger::error("Bad input: std::invalid_argument thrown");
		} catch (std::out_of_range const&) {
			logger::error("Integer overflow: std::out_of_range thrown");
		}
	}
	avManager->avStorage = temporaryJson;

	avManager->mtx.unlock();
}
