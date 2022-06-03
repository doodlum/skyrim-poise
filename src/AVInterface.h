#pragma once

class AVInterface
{
public:

	virtual float GetBaseActorValue(RE::Actor* a_actor) = 0;
	float         GetBaseAV(RE::Actor* a_actor)
	{
		return GetBaseActorValue(a_actor);
	}

	virtual float GetActorValueMax(RE::Actor* a_actor) = 0;
	float         GetAVMax(RE::Actor* a_actor)
	{
		return GetActorValueMax(a_actor);
	}

	//virtual void DamageActorValue(RE::Actor* a_actor, float afDamage) = 0;
	//void         DamageAV(RE::Actor* a_actor, float afDamage)
	//{
	//	DamageActorValue(a_actor, afDamage);
	//}

	//virtual void RestoreActorValue(RE::Actor* a_actor, float afAmount) = 0;
	//void         RestoreAV(RE::Actor* a_actor, float afAmount)
	//{
	//	return RestoreActorValue(a_actor, afAmount);
	//}

	//virtual void ModActorValue(RE::Actor* a_actor, float afAmount) = 0;
	//void         ModAV(RE::Actor* a_actor, float afAmount)
	//{
	//	return ModActorValue(a_actor, afAmount);
	//}
};
