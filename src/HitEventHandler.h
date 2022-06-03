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

	static float RecalculateStagger([[maybe_unused]] RE::Actor* target, [[maybe_unused]] RE::Actor* aggressor, [[maybe_unused]] RE::HitData& hitData);

	void PreProcessVanillaStaggerAttempt(RE::Actor* target, RE::Actor* aggressor, float& staggerMult);

	void PreProcessHitEvent(RE::Actor* target, RE::HitData& hitData);

protected:

	static void ApplyPerkEntryPoint(INT32 entry, RE::Character* actor_a, RE::Character* actor_b, float* out)
	{
		using func_t = decltype(&ApplyPerkEntryPoint);
		REL::Relocation<func_t> func{ REL::RelocationID(23073, 23073) };  // 1.5.97 14032ECE0
		return func(entry, actor_a, actor_b, out);
	}

	struct Hooks
	{
		struct ProcessHitEvent
		{
			static void thunk(RE::Actor* target, RE::HitData& hitData)
			{
				auto handler = GetSingleton();
				handler->PreProcessHitEvent(target, hitData);
				func(target, hitData);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Filter_ApplyPerkEntryPoint_Aggressor
		{
			static void thunk(INT32 entry, RE::Actor* target, RE::Actor* aggressor, float& staggerMult)
			{
				if (staggerMult >= 0) {
					func(entry, target, aggressor, staggerMult);
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Filter_ApplyPerkEntryPoint_Target
		{
			static void thunk(INT32 entry, RE::Actor* target, RE::Actor* aggressor, float& staggerMult)
			{
				if (staggerMult >= 0) {
					func(entry, target, aggressor, staggerMult);
					auto handler = GetSingleton();
					handler->PreProcessVanillaStaggerAttempt(target, aggressor, staggerMult);
				} else {
					staggerMult = -staggerMult;
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<ProcessHitEvent>(REL::RelocationID(37673, 37673).address() + REL::Relocate(0x3C0, 0x3C0, 0x3C0));					// 1.5.97 140628C20
			stl::write_thunk_call<Filter_ApplyPerkEntryPoint_Aggressor>(REL::RelocationID(36700, 36700).address() + REL::Relocate(0x9A, 0x90, 0x9A));	// 1.5.97 1405FA1B0
			stl::write_thunk_call<Filter_ApplyPerkEntryPoint_Target>(REL::RelocationID(36700, 36700).address() + REL::Relocate(0xAE, 0xAE, 0xAE));		// 1.5.97 1405FA1B0
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