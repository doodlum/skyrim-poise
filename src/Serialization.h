#pragma once

#include "AVManager.h"

namespace Serialization
{
	enum : std::uint32_t
	{
		kSerializationVersion = 1,
		kActorValues = 'AV'
	};

	inline void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto avManager = AVManager::GetSingleton();

		avManager->mtx.lock();

		if (!avManager->SerializeSave(a_intfc, Serialization::kActorValues, Serialization::kSerializationVersion)) {
			logger::error("Failed to save actor values!\n");
		}

		avManager->mtx.unlock();
	}

	inline void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto avManager = AVManager::GetSingleton();
		avManager->mtx.lock();

		uint32_t type;
		uint32_t version;
		uint32_t length;
		bool     loaded = false;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version != Serialization::kSerializationVersion) {
				logger::error("Loaded data is out of date! Read (%u), expected (%u) for type code (%s)", version, Serialization::kSerializationVersion, type);
				continue;
			}

			switch (type) {
			case Serialization::kActorValues:
				if (!avManager->DeserializeLoad(a_intfc))
					logger::info("Failed to load actor values!\n");
				else
					loaded = true;
				break;
			default:
				logger::error(FMT_STRING("Unrecognized signature type! {}"), type);
				break;
			}
		}
		avManager->mtx.unlock();
	}

	inline bool Save(SKSE::SerializationInterface* a_intfc, json& root)
	{
		std::string elem = root.dump();
		std::size_t size = elem.length();

		if (!a_intfc->WriteRecordData(size)) {
			logger::error("Failed to write size of record data!");
			return false;
		} else {
			if (!a_intfc->WriteRecordData(elem.data(), static_cast<uint32_t>(elem.length()))) {
				logger::error("Failed to write element!");
				return false;
			}
			logger::info(FMT_STRING("Serialized {}"), elem);
		}
		return true;
	}

	inline bool Load(SKSE::SerializationInterface* a_intfc, json& parsedJson)
	{
		std::size_t size;
		if (!a_intfc->ReadRecordData(size)) {
			logger::error("Failed to load size!");
			return false;
		}

		std::string elem;
		elem.resize(size);

		if (!a_intfc->ReadRecordData(elem.data(), static_cast<uint32_t>(size))) {
			logger::error("Failed to load element!");
			return false;
		} else {
			logger::info(FMT_STRING("Deserialized {}"), elem);
			parsedJson = json::parse(elem);
		}

		return true;
	}

	inline void RevertCallback(SKSE::SerializationInterface*)
	{
		auto avManager = AVManager::GetSingleton();
		avManager->Revert();
	}

	void SaveCallback(SKSE::SerializationInterface* a_intfc);
	void LoadCallback(SKSE::SerializationInterface* a_intfc);
	void RevertCallback(SKSE::SerializationInterface*);
}
