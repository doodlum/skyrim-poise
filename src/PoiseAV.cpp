#include "PoiseAV.h"
#include "PoiseAVHUD.h"
#include "Settings.h"

float PoiseAV::GetBaseActorValue([[maybe_unused]] RE::Actor* a_actor)
{
	auto poise = 0.0f;
	if (!a_actor->GetRace()->GetPlayable())
		poise = std::lerp(50.0f, a_actor->GetActorValue(RE::ActorValue::kMass) * 50.0f, Settings::GetSingleton()->GameSetting.fPoiseMassFactor);

	poise = a_actor->equippedWeight * 0.5f;

	return poise;
}

float PoiseAV::GetActorValueMax([[maybe_unused]] RE::Actor* a_actor)
{
	return GetBaseActorValue(a_actor);
}

void PoiseAV::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
{
	auto avManager = AVManager::GetSingleton();
	avManager->mtx.lock();
	if (avManager->GetActorValue(g_avName, a_actor) <= 0.5f) {
		if (a_actor->actorState2.staggered)
			avManager->RestoreActorValue(g_avName, a_actor, avManager->GetActorValueMax(g_avName, a_actor));
	} else {
		avManager->RestoreActorValue(g_avName, a_actor, avManager->GetActorValueMax(g_avName, a_actor) * (1.0f / 30.0f) * a_delta);
	}
	avManager->mtx.unlock();
}
