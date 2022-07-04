#include "Hooks.h"
#include "ActiveEffectHandler.h"
#include "HitEventHandler.h"
#include "PoiseAV.h"

void Hooks::Install()
{
	PoiseAV::InstallHooks();
	HitEventHandler::InstallHooks();
	ActiveEffectHandler::InstallHooks();
}
