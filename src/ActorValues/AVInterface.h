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
};
