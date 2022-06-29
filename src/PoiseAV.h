#pragma once

#include "AVInterface.h"
#include "AVManager.h"
#include "PCH.h"

// variables
//static float* g_deltaTime = (float*)RELOCATION_ID(523660, 410199).address();          // 2F6B948, 30064C8
//static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address();  // 2F6B94C, 30064CC

class PoiseAV : public AVInterface
{
public:
	static void InstallHooks()
	{
		Hooks::Install();
	}

	inline static const char* g_avName = "Poise";

	float GetBaseActorValue(RE::Actor* a_actor);

	float GetActorValueMax(RE::Actor* a_actor);

	void DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage);

	void Update(RE::Actor* a_actor, float a_delta);

	void            GarbageCollection();

	static PoiseAV* GetSingleton()
	{
		static PoiseAV avInterface;
		return &avInterface;
	}

	//static void TryPushActorAway(RE::Actor* target, [[maybe_unused]] float staggerMult, RE::Actor* aggressor)
	//{
	//	if (auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>()) {
	//		if (auto script = scriptFactory->Create()) {
	//			script->SetCommand(fmt::format(FMT_STRING("PushActorAway {:X} 0"), target->GetFormID()));
	//			script->CompileAndRun(aggressor);
	//		}
	//	}
	//}

	//static void TryApplyHavokImpulse(RE::Actor* target, float afX, float afY, float afZ, float afMagnitude)
	//{
	//	if (auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>()) {
	//		if (auto script = scriptFactory->Create()) {
	//			script->SetCommand(fmt::format(FMT_STRING("ApplyHavokImpulse {} {} {} {}"), afX, afY, afZ, afMagnitude));
	//			script->CompileAndRun(target);
	//		}
	//	}
	//}

	RE::TESFaction* ForceFullBodyStagger;

	void RetrieveFullBodyStaggerFaction()
	{
		ForceFullBodyStagger = RE::TESForm::LookupByID(0x10CED7)->As<RE::TESFaction>();
	}

	static void TryStagger(RE::Actor* a_target, float a_staggerMult, RE::Actor* a_aggressor)
	{
		using func_t = decltype(&TryStagger);
		REL::Relocation<func_t> func{ REL::RelocationID(36700, 37710) };
		func(a_target, -a_staggerMult, a_aggressor);
	}

	static void RemoveFromFaction(RE::Actor* a_actor, RE::TESFaction* a_faction)
	{
		using func_t = decltype(&RemoveFromFaction);
		REL::Relocation<func_t> func{ REL::RelocationID(36680, 37688) };
		func(a_actor, a_faction);
	}

	static RE::ActorValue GetActorValueIdFromName(char* a_name)
	{
		using func_t = decltype(&GetActorValueIdFromName);
		REL::Relocation<func_t> func{ REL::RelocationID(26570, 27203) };
		func(a_name);
	}

protected:

	struct Hooks
	{
		struct PlayerCharacter_Update
		{
			static void thunk(RE::PlayerCharacter* a_player, float a_delta)
			{
				func(a_player, a_delta);
				GetSingleton()->Update(a_player, a_delta);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Actor_Update
		{
			static void thunk(RE::Actor* a_actor, float a_delta)
			{
				func(a_actor, a_delta);
				GetSingleton()->Update(a_actor, a_delta);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();
			stl::write_vfunc<RE::Character, 0xAD, Actor_Update>();
		}
	};
};
