#pragma once

class HitEventHandler
{
public:
	[[nodiscard]] static HitEventHandler* GetSingleton()
	{
		static HitEventHandler singleton;
		return std::addressof(singleton);
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	float GetWeaponDamage(RE::TESObjectWEAP* a_weapon);
	float GetShieldDamage(RE::TESObjectARMO* a_shield);

	float RecalculateStagger(RE::Actor* target, RE::Actor* aggressor, RE::HitData* hitData);

	void PreProcessHit(RE::Actor* target, RE::HitData* hitData);

	static void ApplyPerkEntryPoint(INT32 entry, RE::Actor* actor_a, RE::Actor* actor_b, float* out)
	{
		using func_t = decltype(&ApplyPerkEntryPoint);
		REL::Relocation<func_t> func{ REL::RelocationID(23073, 23526) };  // 1.5.97 14032ECE0
		return func(entry, actor_a, actor_b, out);
	}

protected:
	struct Hooks
	{
		struct ProcessHitEvent
		{
			static void thunk(RE::Actor* target, RE::HitData* hitData)
			{
				auto handler = GetSingleton();
				handler->PreProcessHit(target, hitData);
				func(target, hitData);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Filter_ApplyPerkEntryPoint_Aggressor
		{
			static void thunk(INT32 entry, RE::Actor* target, RE::Actor* aggressor, float& staggerMult)
			{
				if (staggerMult > 0) {
					func(entry, target, aggressor, staggerMult);
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Filter_ApplyPerkEntryPoint_Target
		{
			static void thunk(INT32 entry, RE::Actor* target, RE::Actor* aggressor, float& staggerMult)
			{
				if (staggerMult > 0.0f) {
					func(entry, target, aggressor, staggerMult);
				} else {
					staggerMult = -staggerMult;
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<ProcessHitEvent>(REL::RelocationID(37673, 38627).address() + REL::Relocate(0x3C0, 0x4A8, 0x3C0));                    // 1.5.97 140628C20
			stl::write_thunk_call<Filter_ApplyPerkEntryPoint_Aggressor>(REL::RelocationID(36700, 37710).address() + REL::Relocate(0x9A, 0xA1, 0x9A));  // 1.5.97 1405FA1B0
			stl::write_thunk_call<Filter_ApplyPerkEntryPoint_Target>(REL::RelocationID(36700, 37710).address() + REL::Relocate(0xAE, 0xB9, 0xAE));     // 1.5.97 1405FA1B0
		}
	};

private:
	constexpr HitEventHandler() noexcept = default;
	HitEventHandler(const HitEventHandler&) = delete;
	HitEventHandler(HitEventHandler&&) = delete;

	~HitEventHandler() = default;

	HitEventHandler& operator=(const HitEventHandler&) = delete;
	HitEventHandler& operator=(HitEventHandler&&) = delete;
};
