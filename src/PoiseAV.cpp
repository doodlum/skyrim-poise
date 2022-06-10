#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float PoiseAV::GetBaseActorValue([[maybe_unused]] RE::Actor* a_actor)
{
	auto settings = Settings::GetSingleton();
	auto poise = a_actor->GetBaseActorValue(RE::ActorValue::kMass) * settings->GameSetting.fPoiseHealthAVMult;
	poise += a_actor->equippedWeight * Settings::GetSingleton()->GameSetting.fPoiseHealthArmorMult;

	if (auto raceMult = settings->EffectSetting.root["Races"][a_actor->GetFormEditorID()]["Multiplier"] != nullptr) {
		poise *= raceMult;
	}

	return poise;
}

float PoiseAV::GetActorValueMax([[maybe_unused]] RE::Actor* a_actor)
{
	return GetBaseActorValue(a_actor);
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
			RemoveFromFactionCC(a_actor, ForceFullBodyStagger);
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
