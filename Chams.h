#pragma once
#include "Hooks.h"

class CChams
{
public:
	struct HitChams_t
	{
		float TimeToEnd;
		float Alpha;
		color_t Color;
		ModelRenderInfo_t info;
		DrawModelInfo_t state;
		matrix pBoneToWorld[128] = {};
		matrix model_to_world;
	};
	std::vector<HitChams_t> PHitChams;
	void AddMatrixToHitChams(animation* record);
	void OnPostScreenEffects();
	void Draw(void* ecx, void* results, const DrawModelInfo_t& info,
		matrix* bone_to_world, float* flex_weights, float* flex_delayed_weights, const Vector& model_origin, int flags);
};

extern CChams* Chams;