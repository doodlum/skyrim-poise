#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float PoiseAV::GetBaseActorValue([[maybe_unused]] RE::Actor* a_actor)
{
	auto poise = 0.0f;
	poise += (a_actor->GetBaseActorValue(Settings::GetSingleton()->EffectSetting.PoiseHealthBaseAV) + a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, Settings::GetSingleton()->EffectSetting.PoiseHealthBaseAV)) * Settings::GetSingleton()->GameSetting.fPoiseHealthAVMult;
	poise += a_actor->equippedWeight * Settings::GetSingleton()->GameSetting.fPoiseHealthArmorMult;

	return poise;
}

float PoiseAV::GetActorValueMax([[maybe_unused]] RE::Actor* a_actor)
{
	return GetBaseActorValue(a_actor);
}

void PoiseAV::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
{
	auto avManager = AVManager::GetSingleton();
	auto g_trueHUD = PoiseAVHUD::GetSingleton()->g_trueHUD;

	if (g_trueHUD && !Settings::GetSingleton()->GameSetting.bPoiseAllowStaggerLock) {
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
		}  else {
			TryStagger(a_actor, 0.5f, nullptr);
			if (g_trueHUD)
				g_trueHUD->OverrideSpecialBarColor(a_actor->GetHandle(), TRUEHUD_API::BarColorType::BarColor, 0x808080);
		}
	} else {
		avManager->RestoreActorValue(g_avName, a_actor, avManager->GetActorValueMax(g_avName, a_actor) * Settings::GetSingleton()->GameSetting.fPoiseRegenRate * a_delta);
	}
	avManager->mtx.unlock();
}
