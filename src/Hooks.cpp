#include "Hooks.h"
#include "PoiseAV.h"
#include "HitEventHandler.h"
#include "ActiveEffectHandler.h"

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
