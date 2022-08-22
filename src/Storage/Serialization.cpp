#include "Storage/Serialization.h"

#include "ActorValues/AVManager.h"


namespace Serialization
{
	void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto avManager = AVManager::GetSingleton();

		avManager->mtx.lock();

		if (!avManager->SerializeSave(a_intfc, Serialization::kUniqueID, Serialization::kSerializationVersion)) {
			logger::error("Failed to save actor values!\n");
		}

		avManager->mtx.unlock();
	}

	void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto avManager = AVManager::GetSingleton();
		avManager->mtx.lock();

		uint32_t type;
		uint32_t version;
		uint32_t length;
		bool     loaded = false;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version != Serialization::kSerializationVersion) {
				logger::error("Loaded data is out of date!");
				continue;
			}

			switch (type) {
			case Serialization::kUniqueID:
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

	bool Save(SKSE::SerializationInterface* a_intfc, json& root)
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
			logger::debug(FMT_STRING("Serialized {}"), elem);
		}
		return true;
	}

	bool Load(SKSE::SerializationInterface* a_intfc, json& parsedJson)
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
			logger::debug(FMT_STRING("Deserialized {}"), elem);

			json temporaryJson = json::parse(elem);

			for (auto& el : temporaryJson.items()) {
				std::string oldFormIDs = el.key();
				try {
					RE::FormID oldFormID = static_cast<RE::FormID>(std::stoul(oldFormIDs));
					RE::FormID newFormID = 0;
					if (oldFormID != 0 && a_intfc->ResolveFormID(oldFormID, newFormID)) {
						std::string newFormIDs = std::to_string(newFormID);
						parsedJson[newFormIDs] = temporaryJson[oldFormIDs];
					} else {
						logger::debug(FMT_STRING("Discarded removed form {:X}"), oldFormID);
					}
				} catch (std::invalid_argument const&) {
					logger::error("Bad input: std::invalid_argument thrown");
				} catch (std::out_of_range const&) {
					logger::error("Integer overflow: std::out_of_range thrown");
				}
			}
		}

		return true;
	}

	void RevertCallback(SKSE::SerializationInterface*)
	{
		auto avManager = AVManager::GetSingleton();
		avManager->Revert();
	}
}
