#include "Hooks/Hooks.h"

#include "Hooks/ActiveEffectHandler.h"
#include "Hooks/HitEventHandler.h"
#include "Hooks/PoiseAV.h"


void Hooks::Install()
{
	PoiseAV::InstallHooks();
	HitEventHandler::InstallHooks();
	ActiveEffectHandler::InstallHooks();
}
