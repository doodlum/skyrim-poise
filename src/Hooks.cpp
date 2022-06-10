#include "Hooks.h"
#include "ActiveEffectHandler.h"
#include "HitEventHandler.h"
#include "PoiseAV.h"

namespace Hooks
{
	void Hooks::Install()
	{
		PoiseAV::InstallHooks();
		HitEventHandler::InstallHooks();
		ActiveEffectHandler::InstallHooks();
		logger::info("Installed all hooks");
	}
}
