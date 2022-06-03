#pragma once

#include "AVManager.h"
#include "Serialization.h"

bool AVManager::SerializeSave(SKSE::SerializationInterface* a_intfc)
{
	if (!Serialization::Save(a_intfc, this->avStorage)) {
		logger::error("Failed to write actor values");
		return false;
	}
	return true;
}

bool AVManager::SerializeSave(SKSE::SerializationInterface* a_intfc, uint32_t a_type, uint32_t a_version)
{
	if (!a_intfc->OpenRecord(a_type, a_version)) {
		logger::error("Failed to open actor values record!");
		return false;
	} else {
		return SerializeSave(a_intfc);
	}
}

bool AVManager::DeserializeLoad(SKSE::SerializationInterface* a_intfc)
{
	if (!Serialization::Load(a_intfc, this->avStorage)) {
		logger::info("Failed to load actor values!");
		return false;
	}

	return true;
}

void AVManager::Revert()
{
	this->avStorage.clear();
}

bool AVManager::RegisterActorValue(std::string avName, AVInterface* avInterface)
{
	if (registeredInterfaces[avName]) {
		return false;
	}
	registeredInterfaces[avName] = avInterface;
	return true;
}

float AVManager::GetBaseActorValue(std::string a_actorValue, RE::Actor* a_actor)
{
	std::string formID = std::to_string(a_actor->formID);
	if (avStorage[formID][a_actorValue] == nullptr)
		avStorage[formID][a_actorValue] = { 0.0f, 0.f, 0.0f };
	auto avInterface = registeredInterfaces.at(a_actorValue);
	auto value = avInterface->GetBaseActorValue(a_actor);
	return value;
}

float AVManager::GetActorValueMax(std::string a_actorValue, RE::Actor* a_actor)
{
	std::string formID = std::to_string(a_actor->formID);
	if (avStorage[formID][a_actorValue] == nullptr)
		avStorage[formID][a_actorValue] = { 0.0f, 0.f, 0.0f };
	auto avInterface = registeredInterfaces.at(a_actorValue);
	auto value = avInterface->GetActorValueMax(a_actor);
	return value;
}

void AVManager::DamageActorValue(std::string a_actorValue, RE::Actor* a_actor, float a_damage)
{
	std::string formID = std::to_string(a_actor->formID);
	if (avStorage[formID][a_actorValue] == nullptr)
		avStorage[formID][a_actorValue] = { 0.0f, 0.f, 0.0f };

	auto avInterface = registeredInterfaces.at(a_actorValue);

	float damage = avStorage[formID][a_actorValue][2];

	avStorage[formID][a_actorValue][2] = std::clamp(damage + a_damage, 0.0f, avInterface->GetActorValueMax(a_actor));
}

float AVManager::GetActorValue(std::string a_actorValue, RE::Actor* a_actor)
{
	std::string formID = std::to_string(a_actor->formID);
	if (avStorage[formID][a_actorValue] == nullptr)
		avStorage[formID][a_actorValue] = { 0.0f, 0.f, 0.0f };
	auto  avInterface = registeredInterfaces.at(a_actorValue);
	float value = avInterface->GetBaseActorValue(a_actor);
	value += avStorage[formID][a_actorValue][0];
	value += avStorage[formID][a_actorValue][1];
	value -= avStorage[formID][a_actorValue][2];
	return value;
}

float AVManager::GetActorValuePercentage(std::string a_actorValue, RE::Actor* a_actor)
{
	return GetActorValue(a_actorValue, a_actor) / GetActorValueMax(a_actorValue, a_actor);
}
