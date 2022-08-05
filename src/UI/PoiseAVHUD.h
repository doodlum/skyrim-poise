#pragma once

#include "TrueHUDAPI.h"


class PoiseAVHUD
{
public:
	static PoiseAVHUD* GetSingleton()
	{
		static PoiseAVHUD manager;
		return &manager;
	}

	static float GetMaxSpecial(RE::Actor* a_actor);
	static float GetCurrentSpecial(RE::Actor* a_actor);

	static inline TRUEHUD_API::IVTrueHUD3* trueHUDInterface;

private:
	constexpr PoiseAVHUD() noexcept = default;
	PoiseAVHUD(const PoiseAVHUD&) = delete;
	PoiseAVHUD(PoiseAVHUD&&) = delete;

	~PoiseAVHUD() = default;

	PoiseAVHUD& operator=(const PoiseAVHUD&) = delete;
	PoiseAVHUD& operator=(PoiseAVHUD&&) = delete;
};
