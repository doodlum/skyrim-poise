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

	float CalculateAVEffectPoiseDamage(RE::ActiveEffect* a_activeEffect, RE::ActorValue a_actorValue);
	void  ActiveEffectUpdate(RE::ActiveEffect* a_activeEffect, float a_delta);
	void  DualActiveEffectUpdate(RE::ActiveEffect* a_activeEffect, float a_delta);

	float CalculateStaggerEffectPoiseDamage(RE::ActiveEffect* a_activeEffect);
	void  StaggerUpdate(RE::ActiveEffect* a_activeEffect, float a_delta);

protected:
	struct Hooks
	{
		struct ActiveEffect_Update
		{
			static void thunk(RE::ActiveEffect* a_activeEffect, float a_delta)
			{
				func(a_activeEffect, a_delta);
				GetSingleton()->ActiveEffectUpdate(a_activeEffect, a_delta);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct DualActiveEffect_Update
		{
			static void thunk(RE::ActiveEffect* a_activeEffect, float a_delta)
			{
				func(a_activeEffect, a_delta);
				GetSingleton()->DualActiveEffectUpdate(a_activeEffect, a_delta);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct StaggerEffect_Update
		{
			static void thunk(RE::ActiveEffect* a_activeEffect, float a_delta)
			{
				//func(a_activeEffect, a_delta);
				GetSingleton()->StaggerUpdate(a_activeEffect, a_delta);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_vfunc<0x4, ActiveEffect_Update>(RE::VTABLE_ValueModifierEffect[0]);
			stl::write_vfunc<0x4, DualActiveEffect_Update>(RE::VTABLE_DualValueModifierEffect[0]);
			stl::write_vfunc<0x4, StaggerEffect_Update>(RE::VTABLE_StaggerEffect[0]);
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
