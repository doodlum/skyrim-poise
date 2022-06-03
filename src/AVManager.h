#pragma once

#include "AVInterface.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class AVManager
{
public:
	static AVManager* GetSingleton()
	{
		static AVManager manager;
		return &manager;
	}

	bool SerializeSave(SKSE::SerializationInterface* a_intfc);
	bool SerializeSave(SKSE::SerializationInterface* a_intfc, uint32_t a_type, uint32_t a_version);
	bool DeserializeLoad(SKSE::SerializationInterface* a_intfc);
	void Revert();

	bool RegisterActorValue(std::string avName, AVInterface* avInterface);

	float GetBaseActorValue(std::string a_actorValue, RE::Actor* a_actor);
	float GetActorValueMax(std::string a_actorValue, RE::Actor* a_actor);
	void  DamageActorValue(std::string a_actorValue, RE::Actor* a_actor, float a_damage);
	void  RestoreActorValue(std::string a_actorValue, RE::Actor* a_actor, float a_restore) 
	{
		DamageActorValue(a_actorValue, a_actor, -a_restore);
	}
	float GetActorValue(std::string a_actorValue, RE::Actor* a_actor);
	float GetActorValuePercentage(std::string a_actorValue, RE::Actor* a_actor);

	json                                          avStorage;
	std::unordered_map<std::string, AVInterface*> registeredInterfaces;
	std::mutex                                    mtx;

private:

};
