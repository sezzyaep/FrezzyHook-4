#pragma once
#include "Hooks.h"

struct HitChams_t
{
	IBasePlayer* player;
	int endtime;
	animation* record;
	Vector Origin;
	matrix* pMat;


};

extern std::vector<HitChams_t> hitchams_;

namespace Hitchams {
	void Draw(void* ecx, void* results, const DrawModelInfo_t& info,
		matrix* bone_to_world, float* flex_weights, float* flex_delayed_weights, const Vector& model_origin, int flags, IMaterial* mat) {

		static auto DrawModel = g_pStudioRenderHook->GetOriginal<fnDrawModel>(29);
		static IMaterial* Shot = mat;
		if (!hitchams_.empty()) {

			auto pVar = Shot->FindVar(hs::envmaptint::s().c_str(), nullptr);
			pVar->SetVecValue(vars.ragebot.shot_clr[0] / 255.f,
				vars.ragebot.shot_clr[1] / 255.f, vars.ragebot.shot_clr[2] / 255.f);

			Shot->AlphaModulate(vars.ragebot.shot_clr[3] / 255.f);
			Shot->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
			interfaces.models.model_render->ForcedMaterialOverride(Shot);
			for (HitChams_t hitcham : hitchams_) {



				DrawModel(ecx, results, info, hitcham.pMat, flex_weights, flex_delayed_weights, hitcham.Origin, flags);

			}
			if (hitchams_[hitchams_.size()].endtime > csgo->tickcount) {
				hitchams_.pop_back();
			}
			interfaces.models.model_render->ForcedMaterialOverride(nullptr);
		}
		
	}
};

