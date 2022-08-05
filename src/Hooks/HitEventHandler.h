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
	float GetUnarmedDamage(RE::Actor* a_actor);
	float GetShieldDamage(RE::TESObjectARMO* a_shield);
	float GetMiscDamage();

	float RecalculateStagger(RE::Actor* target, RE::Actor* aggressor, RE::HitData* hitData);

	void PreProcessHit(RE::Actor* target, RE::HitData* hitData);

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

		static void Install()
		{
			stl::write_thunk_call<ProcessHitEvent>(REL::RelocationID(37673, 38627).address() + REL::Relocate(0x3C0, 0x4A8, 0x3C0));  // 1.5.97 140628C20
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
