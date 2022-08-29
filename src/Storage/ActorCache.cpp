#include "Storage/ActorCache.h"

bool TESFormDeleteEventHandler::Register()
{
	static TESFormDeleteEventHandler singleton;
	auto                        ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();

	if (!ScriptEventSource) {
		logger::error("Script event source not found");
		return false;
	}

	ScriptEventSource->AddEventSink(&singleton);

	logger::info("Registered {}", typeid(singleton).name());

	return true;
}

bool TESEquipEventEventHandler::Register()
{
	static TESEquipEventEventHandler singleton;
	auto                             ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();

	if (!ScriptEventSource) {
		logger::error("Script event source not found");
		return false;
	}

	ScriptEventSource->AddEventSink(&singleton);

	logger::info("Registered {}", typeid(singleton).name());

	return true;
}

RE::BSEventNotifyControl TESFormDeleteEventHandler::ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*)
{
	ActorCache::GetSingleton()->FormDelete(a_event->formID);
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TESEquipEventEventHandler::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*)
{
	ActorCache::GetSingleton()->EquipEvent(a_event);
	return RE::BSEventNotifyControl();
}



void ActorCache::FormDelete(RE::FormID a_formID)
{
	std::lock_guard<std::shared_mutex> lk(formCacheLock);
	formCache.erase(a_formID);
}

void ActorCache::EquipEvent(const RE::TESEquipEvent* a_event)
{
	std::lock_guard<std::shared_mutex> lk(formCacheLock);
	if (a_event->actor) {
		if (auto ref = a_event->actor.get()) {
			if (auto actor = ref->As<RE::Actor>()) {
				if (auto form = RE::TESForm::LookupByID(a_event->baseObject)) {
					bool  valid = false;
					float weight = 0.0f;
					if (auto armor = form->As<RE::TESObjectARMO>()) {
						weight = armor->GetWeight();
						valid = true;
					} else if (auto weapon = form->As<RE::TESObjectARMO>()) {
						weight = weapon->GetWeight();
						valid = true;
					}
					if (valid) {
						if (formCache.contains(actor->formID)) {
							ActorData& data = formCache.at(actor->formID);
							data.trueWeightValue += a_event->equipped ? weight : -weight;
							logger::debug("{} {} weight ", a_event->equipped ? "Added" : "Removed", weight);
							logger::debug("True weight is {} vanilla weight is {}", data.trueWeightValue, actor->equippedWeight);
						}
					}
				}
			}
		}
	}
}

float ActorCache::CalculateEquippedWeight(RE::Actor* a_actor)
{
	using BipedObjectSlot = RE::BIPED_MODEL::BipedObjectSlot;

	float equippedWeight = 0.0f;

	int slotsChecked = 0x0;
	int thisSlot = 0x01;

	while (thisSlot < 0x80000000) {
		if ((slotsChecked & thisSlot) != thisSlot) {
			if (auto thisArmor = a_actor->GetWornArmor((BipedObjectSlot)thisSlot)) {
				equippedWeight += thisArmor->GetWeight();
				logger::debug("Actor {} Object {} Weight {}", a_actor->GetName(), thisArmor->GetName(), thisArmor->GetWeight());
				slotsChecked += (int)thisArmor->GetSlotMask();
			} else {
				slotsChecked += thisSlot;
			}
		}
		thisSlot *= 2;
	}
	if (auto rightHand = a_actor->GetEquippedObject(false)) {
		if (auto weapon = rightHand->As<RE::TESObjectWEAP>()) {
			equippedWeight += weapon->GetWeight();
			logger::debug("Actor {} Object {} Weight {}", a_actor->GetName(), weapon->GetName(), weapon->GetWeight());
		}
	}
	if (auto leftHand = a_actor->GetEquippedObject(true)) {
		if (auto weapon = leftHand->As<RE::TESObjectWEAP>()) {
			equippedWeight += weapon->GetWeight();
			logger::debug("Actor {} Object {} Weight {}", a_actor->GetName(), weapon->GetName(), weapon->GetWeight());
		}
	}
	return equippedWeight;
}

void ActorCache::RecalculateCacheData(RE::Actor* a_actor)
{
	std::lock_guard<std::shared_mutex> lk(formCacheLock);
	if (formCache.contains(a_actor->formID)) {
		ActorData& data = formCache.at(a_actor->formID);
		data.trueWeightValue = CalculateEquippedWeight(a_actor);
	}
}

float ActorCache::GetOrCreateCachedWeight(RE::Actor* a_actor)
{
	std::lock_guard<std::shared_mutex> lk(formCacheLock);
	if (!formCache.contains(a_actor->formID)) {
		ActorData data;
		data.trueWeightValue = CalculateEquippedWeight(a_actor);
		logger::debug("True weight is {} vanilla weight is {}", data.trueWeightValue, a_actor->equippedWeight);
		formCache.insert({ a_actor->formID, data });
		return data.trueWeightValue;
	}
	return formCache.at(a_actor->formID).trueWeightValue;
}
void ActorCache::Revert() 
{
	std::lock_guard<std::shared_mutex> lk(formCacheLock);
	formCache.clear();
}

void ActorCache::RegisterEvents()
{
	TESFormDeleteEventHandler::Register();
	TESEquipEventEventHandler::Register();
}
