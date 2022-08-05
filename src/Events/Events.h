#pragma once

namespace Events
{
	void Register();
}

class cellLoadEventHandler : public RE::BSTEventSink<RE::TESCellFullyLoadedEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>* a_eventSource);
	static bool Register();
};

class fastTravelEventHandler : public RE::BSTEventSink<RE::TESFastTravelEndEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESFastTravelEndEvent* a_event, RE::BSTEventSource<RE::TESFastTravelEndEvent>* a_eventSource);
	static bool Register();
};

class waitEventHandler : public RE::BSTEventSink<RE::TESWaitStopEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStopEvent* a_event, RE::BSTEventSource<RE::TESWaitStopEvent>* a_eventSource);
	static bool Register();
};
