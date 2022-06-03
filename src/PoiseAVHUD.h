#pragma once
#include "TrueHUDAPI.h"

#include "AVManager.h"

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

    TRUEHUD_API::IVTrueHUD3* g_trueHUD = NULL;

private:
	constexpr PoiseAVHUD() noexcept = default;
	PoiseAVHUD(const PoiseAVHUD&) = delete;
	PoiseAVHUD(PoiseAVHUD&&) = delete;

	~PoiseAVHUD() = default;

	PoiseAVHUD& operator=(const PoiseAVHUD&) = delete;
	PoiseAVHUD& operator=(PoiseAVHUD&&) = delete;
};

