#pragma once

#include "ActorValues/AVInterface.h"

//static float* g_deltaTime = (float*)RELOCATION_ID(523660, 410199).address();          // 2F6B948, 30064C8
//static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address();  // 2F6B94C, 30064CC

static float& g_deltaTime = (*(float*)RELOCATION_ID(523660, 410199).address());

class PoiseAV : public AVInterface
{
public:
	static void InstallHooks()
	{
		Hooks::Install();
	}

	static PoiseAV* GetSingleton()
	{
		static PoiseAV avInterface;
		return &avInterface;
	}

	inline static const char* g_avName = "Poise";

	bool  CanDamageActor(RE::Actor* a_actor);
	float GetBaseActorValue(RE::Actor* a_actor);
	float GetActorValueMax(RE::Actor* a_actor);
	void  DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage);
	void  Update(RE::Actor* a_actor, float a_delta);
	void  GarbageCollection();

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

	bool            appliedStagger = false;
	std::thread::id staggerThread;

	// POISE animations. Didn't like it
	//static void TryStaggerCustom(RE::Actor* a_target, float a_staggerMult, RE::Actor* a_aggressor)
	//{
	//	float stagDir;
	//	if (a_target == a_aggressor || a_aggressor == nullptr) {
	//		stagDir = 0.0f;
	//	} else {
	//		auto hitPos = a_target->GetPosition();
	//		auto heading = a_aggressor->GetHeadingAngle(hitPos, false);
	//		stagDir = (heading >= 0.0f) ? heading / 360.0f : (360.0f + heading) / 360.0f;
	//	}
	//	a_target->SetGraphVariableFloat("staggerDirection", stagDir);  // set direction
	//	a_target->SetGraphVariableFloat("staggerMagnitude", a_staggerMult);
	//	static RE::BSFixedString str = NULL;
	//	if (a_staggerMult <= 0.25) {
	//		if (stagDir > 0.25f && stagDir < 0.75f) {
	//			str = "poise_small_start_fwd";
	//		} else {
	//			str = "poise_small_start";
	//		}
	//	} else if (a_staggerMult <= 0.75) {
	//		if (stagDir > 0.25f && stagDir < 0.75f) {
	//			str = "poise_med_start";
	//		} else {
	//			str = "poise_med_start_fwd";
	//		}
	//	} else if (a_staggerMult <= 1) {
	//		if (stagDir > 0.25f && stagDir < 0.75f) {
	//			str = "poise_large_start";
	//		} else {
	//			str = "poise_large_start_fwd";
	//		}
	//	} else if (a_staggerMult > 1.0) {
	//		if (stagDir > 0.25f && stagDir < 0.75f) {
	//			str = "poise_largest_start";
	//		} else {
	//			str = "poise_large_start_fwd";
	//		}
	//	}
	//	a_target->NotifyAnimationGraph(str);  // play animation
	//	a_target->actorState2.staggered = true;
	//}

	static void TryStagger(RE::Actor* a_target, float a_staggerMult, RE::Actor* a_aggressor)
	{
		GetSingleton()->appliedStagger = true;
		GetSingleton()->staggerThread = std::this_thread::get_id();
		using func_t = decltype(&TryStagger);
		REL::Relocation<func_t> func{ REL::RelocationID(36700, 37710) };
		func(a_target, a_staggerMult, a_aggressor);
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

	static void ApplyPerkEntryPoint(INT32 entry, RE::Actor* actor_a, RE::Actor* actor_b, float* out)
	{
		using func_t = decltype(&ApplyPerkEntryPoint);
		REL::Relocation<func_t> func{ REL::RelocationID(23073, 23526) };  // 1.5.97 14032ECE0
		return func(entry, actor_a, actor_b, out);
	}

protected:
	struct Hooks
	{
		struct PlayerCharacter_Update
		{
			static void thunk(RE::PlayerCharacter* a_player, float a_delta)
			{
				func(a_player, a_delta);
				GetSingleton()->Update(a_player, g_deltaTime);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Actor_Update
		{
			static void thunk(RE::Actor* a_actor, float a_delta)
			{
				func(a_actor, a_delta);
				GetSingleton()->Update(a_actor, g_deltaTime);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Filter_ApplyPerkEntryPoint_Aggressor
		{
			static void thunk(INT32 entry, RE::Actor* target, RE::Actor* aggressor, float& staggerMult)
			{
				auto poiseAV = GetSingleton();
				auto currentThread = std::this_thread::get_id();
				if (poiseAV->appliedStagger && poiseAV->staggerThread == currentThread)
					return;
				if (poiseAV->staggerThread != currentThread)
					logger::debug("Stagger attempted on another thread");
				func(entry, target, aggressor, staggerMult);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Filter_ApplyPerkEntryPoint_Target
		{
			static void thunk(INT32 entry, RE::Actor* target, RE::Actor* aggressor, float& staggerMult)
			{
				auto poiseAV = GetSingleton();
				auto currentThread = std::this_thread::get_id();
				if (poiseAV->appliedStagger && poiseAV->staggerThread == currentThread)
					poiseAV->appliedStagger = false;
				else
					func(entry, target, aggressor, staggerMult);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();
			stl::write_vfunc<RE::Character, 0xAD, Actor_Update>();
			stl::write_thunk_call<Filter_ApplyPerkEntryPoint_Aggressor>(REL::RelocationID(36700, 37710).address() + REL::Relocate(0x9A, 0xA1, 0x9A));  // 1.5.97 1405FA1B0
			stl::write_thunk_call<Filter_ApplyPerkEntryPoint_Target>(REL::RelocationID(36700, 37710).address() + REL::Relocate(0xAE, 0xB9, 0xAE));     // 1.5.97 1405FA1B0
		}
	};
};
