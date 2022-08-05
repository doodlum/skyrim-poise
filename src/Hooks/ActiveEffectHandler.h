#pragma once

class ActiveEffectHandler
{
public:
	[[nodiscard]] static ActiveEffectHandler* GetSingleton()
	{
		static ActiveEffectHandler singleton;
		return std::addressof(singleton);
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	float CalculateEffectMultiplier(RE::ActorValue a_actorValue, bool a_detrimental);
	void  ProcessValueModifier(RE::Actor* a_target, RE::ActorValue a_actorValue, float a_magnitudeDelta, RE::Actor* a_caster);

protected:
	struct Hooks
	{
		struct ValueModifier_ModifyActorValue
		{
			static void thunk(RE::Actor* a_target, RE::ACTOR_VALUE_MODIFIER a_actorValueModifier, RE::ActorValue a_actorValue, float a_magnitudeDelta, RE::Actor* a_caster)
			{
				func(a_target, a_actorValueModifier, a_actorValue, a_magnitudeDelta, a_caster);
				GetSingleton()->ProcessValueModifier(a_target, a_actorValue,  -a_magnitudeDelta, a_caster);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct ValueModifier_DamageHealth
		{
			static void thunk(RE::Actor* a_target, float a_damage, RE::Actor* a_attacker, bool a_useDifficultyModifier)
			{
				func(a_target, a_damage, a_attacker, a_useDifficultyModifier);
				GetSingleton()->ProcessValueModifier(a_target, RE::ActorValue::kHealth, a_damage, a_attacker);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		//struct ValueModifier_RestoreActorValue
		//{
		//	static void thunk(RE::Actor* a_target, RE::ActorValue a_actorValue, float a_magnitudeDelta)
		//	{
		//		func(a_target, a_actorValue, a_magnitudeDelta);
		//		GetSingleton()->ProcessValueModifier(a_target, a_actorValue, -a_magnitudeDelta, nullptr);
		//	}
		//	static inline REL::Relocation<decltype(thunk)> func;
		//};

		static void Install()
		{
			stl::write_thunk_call<ValueModifier_ModifyActorValue>(REL::RelocationID(34286, 35086).address() + REL::Relocate(0x2D1, 0x2CC));   // 1.5.97 140567a80
			stl::write_thunk_call<ValueModifier_DamageHealth>(REL::RelocationID(34286, 35086).address() + REL::Relocate(0x237, 0x232));       // 1.5.97 140567a80
			//stl::write_thunk_call<ValueModifier_RestoreActorValue>(REL::RelocationID(34286, 35086).address() + REL::Relocate(0x13E, 0x138));  // 1.5.97 140567a80
		}
	};

private:
	constexpr ActiveEffectHandler() noexcept = default;
	ActiveEffectHandler(const ActiveEffectHandler&) = delete;
	ActiveEffectHandler(ActiveEffectHandler&&) = delete;

	~ActiveEffectHandler() = default;

	ActiveEffectHandler& operator=(const ActiveEffectHandler&) = delete;
	ActiveEffectHandler& operator=(ActiveEffectHandler&&) = delete;
};
