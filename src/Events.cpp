#pragma once

#include "Events.h"
#include "PoiseAV.h"

void Events::Register()
{
	cellLoadEventHandler::Register();
	fastTravelEventHandler::Register();
	waitEventHandler::Register();
}

RE::BSEventNotifyControl cellLoadEventHandler::ProcessEvent(const RE::TESCellFullyLoadedEvent*, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*)
{
	auto poiseAV = PoiseAV::GetSingleton();
	poiseAV->GarbageCollection();
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl fastTravelEventHandler::ProcessEvent(const RE::TESFastTravelEndEvent*, RE::BSTEventSource<RE::TESFastTravelEndEvent>*)
{
	auto avManager = AVManager::GetSingleton();
	avManager->Revert();
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl waitEventHandler::ProcessEvent(const RE::TESWaitStopEvent*, RE::BSTEventSource<RE::TESWaitStopEvent>*)
{
	auto avManager = AVManager::GetSingleton();
	avManager->Revert();
	return RE::BSEventNotifyControl::kContinue;
}
