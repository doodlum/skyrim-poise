#include "Hooks.h"
#include "PoiseAV.h"
#include "HitEventHandler.h"

namespace Hooks
{
	void Hooks::Install()
	{
		PoiseAV::InstallHooks();
		HitEventHandler::InstallHooks();
		logger::info("Installed all hooks");
	}
}
