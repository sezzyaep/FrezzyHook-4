#include "Chams.h"
#include "Hooks.h"
#include "AnimationFix.h"
#include "Ragebot.h"
MatrixRecorded g_Records[64];
void matrix_set_origin(Vector pos, matrix& m)
{
	m[0][3] = pos.x;
	m[1][3] = pos.y;
	m[2][3] = pos.z;
}

Vector matrix_get_origin(const matrix& src)
{
	return { src[0][3], src[1][3], src[2][3] };
}

bool GetBacktrackMaxtrix(IBasePlayer* player, matrix* out)
{
	if (vars.visuals.interpolated_bt)
	{
		auto New = g_Animfix->get_latest_animation(player);
		auto Old = g_Animfix->get_oldest_animation(player);

		if (!New.has_value()
			|| !Old.has_value())
			return false;

		const auto& first_invalid = Old.value();
		const auto& last_valid = New.value();

		if ((first_invalid->origin - player->GetAbsOrigin()).Length2D() < 7.5f)
			return false;

		if (first_invalid->dormant)
			return false;

		if (last_valid->sim_time - first_invalid->sim_time > 0.5f)
			return false;

		const auto next = last_valid->origin;
		const auto curtime = interfaces.global_vars->curtime;

		auto delta = 1.f - (curtime - last_valid->interp_time) / (last_valid->sim_time - first_invalid->sim_time);
		if (delta < 0.f || delta > 1.f)
			last_valid->interp_time = curtime;

		delta = 1.f - (curtime - last_valid->interp_time) / (last_valid->sim_time - first_invalid->sim_time);
		auto Interpolate = [](const Vector from, const Vector to, const float percent) {
			return to * percent + from * (1.f - percent);
		};
		const auto lerp = Interpolate(next, first_invalid->origin, std::clamp<float>(delta, 0.f, 1.f));

		matrix ret[128];
		memcpy(ret, first_invalid->bones, sizeof(matrix[128]));

		for (size_t i{}; i < 128; ++i)
		{
			const auto matrix_delta = matrix_get_origin(first_invalid->bones[i]) - first_invalid->origin;
			matrix_set_origin(matrix_delta + lerp, ret[i]);
		}

		memcpy(out, ret, sizeof(matrix[128]));
		return true;
	}
	else
	{
		const auto last = g_Animfix->get_oldest_animation(player);

		if (!last.has_value())
			return false;

		memcpy(out, last.value()->bones, sizeof(matrix[128]));
		return true;
	}

}
void CChams::AddMatrixToHitChams(animation* record) {
	auto player = record->player;

	int idx = player->EntIndex();

	auto curBones = record->bones;
	if (!curBones)
		return;
	auto& hit = this->PHitChams.emplace_back();

	std::memcpy(hit.pBoneToWorld, curBones, sizeof(matrix) * 127);


	int m_nSkin = player->m_nSkin();
	int m_nBody = player->m_nBody();

	hit.info.origin = player->GetAbsOrigin();
	hit.info.angles = player->GetAbsAngles();
	hit.Alpha = 255.f;
	hit.TimeToEnd = interfaces.global_vars->realtime;
	auto renderable = player->GetClientRenderable();

	if (!renderable)
		return;

	auto model = player->GetModel();

	if (!model)
		return;

	auto hdr = interfaces.models.model_info->GetStudioModel(model);
	if (!hdr)
		return;

	hit.state.pStudioHdr = hdr;
	hit.state.pHardwareData = interfaces.model_cache->GetHardwareData(model->studio);

	hit.state.pClientEntity = renderable;
	//hit.state.m_drawFlags = 0;

	hit.info.pRenderable = renderable;
	hit.info.pModel = model;
	hit.info.pLightingOffset = nullptr;
	hit.info.pLightingOrigin = nullptr;
	hit.info.hitboxset = player->GetHitboxSet();
	hit.info.skin = (int)(uintptr_t(player) + m_nSkin);
	hit.info.body = (int)(uintptr_t(player) + m_nBody);
	hit.info.entity_index = player->GetIndex();
	hit.info.instance = getvfunc<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30)(renderable);
	hit.info.flags = 0x1;

	hit.info.pModelToWorld = &hit.model_to_world;

	Math::AngleMatrix(hit.info.angles, hit.info.origin, hit.model_to_world);
}
void CChams::OnPostScreenEffects() {
	static IMaterial* Glow = interfaces.material_system->FindMaterial(hs::glow_armsrace::s().c_str(), nullptr);
	if (PHitChams.empty() || !Glow) {
		return;
	}
	static auto DrawModelExecute = g_pModelRenderHook->GetOriginal< DrawModelExecuteFn >(21);

	if (!interfaces.models.model_render)
		return;

	auto ctx = interfaces.material_system->GetRenderContext();

	if (!ctx)
		return;

	auto it = PHitChams.begin();


	if (!this->PHitChams.empty()) {

		bool should_draw = false;
		auto it = PHitChams.begin();
		while (it != PHitChams.end()) {

			if (!it->state.pClientEntity || !it->state.pStudioHdr || !it->state.pHardwareData ||
				!it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
				++it;
				should_draw = false;
				continue;
			}
			auto ent = interfaces.ent_list->GetClientEntity(it->info.entity_index);
			if (!ent) {
				it = PHitChams.erase(it);
				should_draw = false;
				continue;
			}

			auto alpha = 1.0f;
			auto delta = interfaces.global_vars->realtime - it->TimeToEnd;
			if (delta > 0.0f) {
				alpha -= delta;
				if (delta > 1.0f) {
					it = PHitChams.erase(it);
			
					continue;
				}
			}
			auto pVar = Glow->FindVar(hs::envmaptint::s().c_str(), nullptr);
			pVar->SetVecValue(vars.ragebot.shot_clr[0] / 255.f,
				vars.ragebot.shot_clr[1] / 255.f, vars.ragebot.shot_clr[2] / 255.f);

			Glow->AlphaModulate((vars.ragebot.shot_clr[3] / 255.f) * it->Alpha);
			Glow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			interfaces.models.model_render->ForcedMaterialOverride(Glow);
			interfaces.models.model_render->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);
			interfaces.models.model_render->ForcedMaterialOverride(nullptr);
			++it;
		}
	}
	/*
	while (it != PHitChams.end()) {

		if (!it->state.mod || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
			++it;

			continue;
		}
		auto ent = interfaces.ent_list->GetClientEntity(it->info.entity_index);
		if (!ent) {
			it = PHitChams.erase(it);
			continue;
		}

		if (it->TimeToEnd >= interfaces.global_vars->realtime) {

			if (it->Alpha <= 0) {

				it = PHitChams.erase(it);
				continue;

			}
		}

		
		pVar->SetVecValue(vars.ragebot.shot_clr[0] / 255.f,
			vars.ragebot.shot_clr[1] / 255.f, vars.ragebot.shot_clr[2] / 255.f);

		Glow->AlphaModulate((vars.ragebot.shot_clr[3] / 255.f) * it->Alpha);
		Glow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
		
		
		interfaces.models.model_render->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);

	}
	*/


}


bool GetSmoothMatrix(IBasePlayer* player, matrix* out)
{
	auto Interpolate = [](const Vector from, const Vector to, const float percent) {
		return to * percent + from * ((1.f - percent));
	};

	auto first_invalid = csgo->all_data[1];
	auto last_valid = csgo->all_data[0];

	auto orig_delta = (last_valid.origin - first_invalid.origin).Length2D();
	const auto next = last_valid.origin;
	const auto curtime = interfaces.global_vars->curtime;

	const float correct = interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING)
		+ interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + Ragebot::Get().LerpTime() - (last_valid.sim_time - first_invalid.sim_time);

	float time_delta = first_invalid.sim_time - last_valid.sim_time;
	float add = 0.2f;
	float deadtime = last_valid.sim_time + correct + add;

	float delta = deadtime - curtime;

	float mul = 1.f / add;

	Vector lerp = Interpolate(first_invalid.origin, next, std::clamp(delta * mul, 0.f, 1.f));

	matrix ret[128];
	memcpy(ret, csgo->fakelag_matrix, sizeof(matrix) * 128);

	for (size_t i{}; i < 128; ++i)
	{
		auto matrix_delta = matrix_get_origin(csgo->fakelag_matrix[i]) - last_valid.origin;

		matrix_set_origin(matrix_delta + lerp, ret[i]);
	}

	memcpy(out, ret, sizeof(matrix) * 128);
	return true;
}


void CChams::Draw(void* ecx, void* results, const DrawModelInfo_t& info,
	matrix* bone_to_world, float* flex_weights, float* flex_delayed_weights, const Vector& model_origin, int flags)
{
	static int Optimization = 0;
	static auto DrawModel = g_pStudioRenderHook->GetOriginal<fnDrawModel>(29);
	const auto ModelName = info.pStudioHdr->name;
	static IMaterial* Regular = interfaces.material_system->FindMaterial(hs::debugambientcube::s().c_str(), nullptr);
	static IMaterial* Flat = interfaces.material_system->FindMaterial(hs::debugdrawflat::s().c_str(), nullptr);
	static IMaterial* Glow = interfaces.material_system->FindMaterial(hs::glow_armsrace::s().c_str(), nullptr);
	static IMaterial* GlowFade = interfaces.material_system->FindMaterial(hs::regular_glow::s().c_str(), nullptr);
	static IMaterial* GlowP = interfaces.material_system->FindMaterial("models/inventory_items/trophy_majors/gloss", nullptr);
	static IMaterial* Bubble = interfaces.material_system->FindMaterial(hs::regular_glow::s().c_str(), nullptr);
	// говнокод, мб потом переделаю 
	if (Optimization != vars.visuals.op_strength + 1 && vars.visuals.optimize && vars.visuals.To_Op & 2)
		goto skip;

	if (strstr(ModelName, "player/") != nullptr)
	{
		if (!info.pClientEntity)
			return DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

		auto Entity = info.pClientEntity->GetIClientUnknown()->GetBaseEntity();

		if (!Entity)
			return DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

		if (Entity && Entity->IsValid() && Entity->GetTeam() != csgo->local->GetTeam())
		{
			static IMaterial* OverlayMaterial = nullptr;
			static IMaterial* OverlayMaterialXqz = nullptr;

			static IMaterial* OverridedMaterial = nullptr;
			static IMaterial* OverridedMaterialXqz = nullptr;

			static IMaterial* OverridedMaterialBck = nullptr;
			static IMaterial* OverlayMaterialBck = nullptr;

			switch (vars.visuals.overlay)
			{
			case 1: OverlayMaterial = Glow; break;
			case 2: OverlayMaterial = GlowFade; break;
			case 3: OverlayMaterial = GlowP; break;
			}

			switch (vars.visuals.misc_chams[history].overlay)
			{
			case 1: OverlayMaterialBck = Glow; break;
			case 2: OverlayMaterialBck = GlowFade; break;
			case 3: OverlayMaterialBck = GlowP; break;
			}

			switch (vars.visuals.overlay_xqz)
			{
			case 1: OverlayMaterialXqz = Glow; break;
			case 2: OverlayMaterialXqz = GlowFade; break;
			case 3: OverlayMaterialXqz = GlowP; break;
			}

			switch (vars.visuals.chamstype)
			{
			case 0: OverridedMaterial = Regular; break;
			case 1: OverridedMaterial = Flat; break;
			case 2: OverridedMaterial = Bubble; break;
			case 3: OverridedMaterial = Bubble; break;
			}

			OverridedMaterial->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, true);

			switch (vars.visuals.misc_chams[history].material)
			{
			case 0: OverridedMaterialBck = Regular; break;
			case 1: OverridedMaterialBck = Flat; break;
			case 2: OverridedMaterialBck = Bubble; break;
			case 3: OverridedMaterialBck = Bubble; break;
			}

			matrix pMat[128];
			if (vars.visuals.misc_chams[history].enable && GetBacktrackMaxtrix(Entity, pMat)) {
			

				OverridedMaterialBck->ColorModulate(
					vars.visuals.misc_chams[history].clr[0] / 255.f,
					vars.visuals.misc_chams[history].clr[1] / 255.f,
					vars.visuals.misc_chams[history].clr[2] / 255.f);
				OverridedMaterialBck->AlphaModulate(vars.visuals.misc_chams[history].clr[3] / 255.f);
				OverridedMaterialBck->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
				float clr[3] = {
					(vars.visuals.misc_chams[history].clr[0] / 255.f) * vars.visuals.misc_chams[history].chams_brightness / 100.f,
					(vars.visuals.misc_chams[history].clr[1] / 255.f) * vars.visuals.misc_chams[history].chams_brightness / 100.f,
					(vars.visuals.misc_chams[history].clr[2] / 255.f) * vars.visuals.misc_chams[history].chams_brightness / 100.f
				};
				float backup_clr[3];
				interfaces.render_view->GetColorModulation(backup_clr);
				interfaces.render_view->SetColorModulation(clr);

				if (vars.visuals.misc_chams[history].material == 2)
				{
					OverridedMaterialBck->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
						(vars.visuals.misc_chams[history].clr[0] / 705.f) * 0.24f,
						(vars.visuals.misc_chams[history].clr[1] / 705.f) * 0.24f,
						(vars.visuals.misc_chams[history].clr[2] / 705.f) * 0.24f);

					OverridedMaterialBck->AlphaModulate((vars.visuals.misc_chams[history].clr[3]/ 500.f) * 0.43f);

				}

				interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterialBck);
				DrawModel(ecx, results, info, pMat, flex_weights, flex_delayed_weights, model_origin, flags);

				if (vars.visuals.misc_chams[history].overlay > 0) {
					auto pVar = OverlayMaterialBck->FindVar(hs::envmaptint::s().c_str(), nullptr);
					pVar->SetVecValue(vars.visuals.misc_chams[history].glow_clr[0] / 255.f,
						vars.visuals.misc_chams[history].glow_clr[1] / 255.f, vars.visuals.misc_chams[history].glow_clr[2] / 255.f);

					OverlayMaterialBck->AlphaModulate(vars.visuals.misc_chams[history].glow_clr[3] / 255.f);
					OverlayMaterialBck->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					if (vars.visuals.misc_chams[history].overlay == 3) {
						pVar->SetVecValue(vars.visuals.misc_chams[history].glow_clr[0] / 510.f,
							vars.visuals.misc_chams[history].glow_clr[1] / 510.f, vars.visuals.misc_chams[history].glow_clr[2] / 510.f);
						OverlayMaterialBck->AlphaModulate(vars.visuals.misc_chams[history].glow_clr[3] / 510.f);
					}
					interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterialBck);
					DrawModel(ecx, results, info, pMat, flex_weights, flex_delayed_weights, model_origin, flags);
				}
			}



			
			if (vars.visuals.chamsxqz)
			{
				OverridedMaterial->ColorModulate(
					vars.visuals.chamscolor_xqz[0] / 255.f, vars.visuals.chamscolor_xqz[1] / 255.f, vars.visuals.chamscolor_xqz[2] / 255.f);
				OverridedMaterial->AlphaModulate(vars.visuals.chamscolor_xqz[3] / 255.f);
				OverridedMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
				float clr[3] = {
				(vars.visuals.chamscolor_xqz[0] / 255.f) * vars.visuals.chams_brightness / 100.f,
				(vars.visuals.chamscolor_xqz[1] / 255.f) * vars.visuals.chams_brightness / 100.f,
				(vars.visuals.chamscolor_xqz[2] / 255.f) * vars.visuals.chams_brightness / 100.f
				};
				
				float backup_clr[3];

				if (vars.visuals.chamstype == 2)
				{
					OverridedMaterialBck->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
						(vars.visuals.chamscolor_xqz[0] / 705.f) * 0.24f,
						(vars.visuals.chamscolor_xqz[1] / 705.f) * 0.24f,
						(vars.visuals.chamscolor_xqz[2] / 705.f) * 0.24f);

					OverridedMaterialBck->AlphaModulate((vars.visuals.chamscolor_xqz[3]/ 500.f) * 0.43f);
				}


				interfaces.render_view->GetColorModulation(backup_clr);
				interfaces.render_view->SetColorModulation(clr);
				interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterial);
				DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

				if (vars.visuals.overlay_xqz > 0) {
					auto pVar = OverlayMaterialXqz->FindVar(hs::envmaptint::s().c_str(), nullptr);
					pVar->SetVecValue(vars.visuals.glow_col_xqz[0] / 255.f, vars.visuals.glow_col_xqz[1] / 255.f, vars.visuals.glow_col_xqz[2] / 255.f);

					OverlayMaterialXqz->AlphaModulate(vars.visuals.glow_col_xqz[3] / 255.f);
					OverlayMaterialXqz->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					if (vars.visuals.overlay_xqz == 3) {
						pVar->SetVecValue(vars.visuals.glow_col_xqz[0] / 510.f,
							vars.visuals.glow_col_xqz[1] / 510.f, vars.visuals.glow_col_xqz[2] / 510.f);
						OverlayMaterialXqz->AlphaModulate(vars.visuals.glow_col_xqz[3] / 510.f);
					}
					interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterialXqz);
					DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
					interfaces.render_view->SetColorModulation(backup_clr);
				}
			}

			if (vars.visuals.chams)
			{
				OverridedMaterial->ColorModulate(
					vars.visuals.chamscolor[0] / 255.f, vars.visuals.chamscolor[1] / 255.f, vars.visuals.chamscolor[2] / 255.f);
				OverridedMaterial->AlphaModulate(vars.visuals.chamscolor[3] / 255.f);
				OverridedMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				float clr[3] = {
					(vars.visuals.chamscolor[0] / 255.f) * vars.visuals.chams_brightness / 100.f,
					(vars.visuals.chamscolor[1] / 255.f) * vars.visuals.chams_brightness / 100.f,
					(vars.visuals.chamscolor[2] / 255.f) * vars.visuals.chams_brightness / 100.f
				};
				float backup_clr[3];

				if (vars.visuals.chamstype == 2)
				{
					OverridedMaterialBck->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
						(vars.visuals.chamscolor[0] / 705.f) * 0.24f,
						(vars.visuals.chamscolor[1] / 705.f) * 0.24f,
						(vars.visuals.chamscolor[2] / 705.f) * 0.24f);
			
					OverridedMaterialBck->AlphaModulate((vars.visuals.chamscolor[3]/ 500.f) * 0.43f);
				}


				interfaces.render_view->GetColorModulation(backup_clr);
				interfaces.render_view->SetColorModulation(clr);
				interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterial);
				DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

				if (vars.visuals.overlay > 0) {
					auto pVar = OverlayMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr);
					pVar->SetVecValue(vars.visuals.glow_col[0] / 255.f, vars.visuals.glow_col[1] / 255.f, vars.visuals.glow_col[2] / 255.f);

					OverlayMaterial->AlphaModulate(vars.visuals.glow_col[3] / 255.f);
					OverlayMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					if (vars.visuals.overlay == 3) {
						pVar->SetVecValue(vars.visuals.glow_col[0] / 510.f,
							vars.visuals.glow_col[1] / 510.f, vars.visuals.glow_col[2] / 510.f);
						OverlayMaterial->AlphaModulate(vars.visuals.glow_col[3] / 510.f);
					}
					interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterial);
					DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
					interfaces.render_view->SetColorModulation(backup_clr);
				}
			}
		}
		else if (Entity == csgo->local && csgo->local->isAlive())
		{

			static IMaterial* OverridedMaterialLocal = nullptr;
			static IMaterial* OverlayMaterialLocal = nullptr;
			if (interfaces.input->m_fCameraInThirdPerson) {

				switch (vars.visuals.localchamstype)
				{
				case 0: OverridedMaterialLocal = Regular; break;
				case 1: OverridedMaterialLocal = Flat; break;
				case 2: OverridedMaterialLocal = Bubble; break;
				case 3: OverridedMaterialLocal = Bubble; break;
				}

				switch (vars.visuals.local_chams.overlay)
				{
				case 1: OverlayMaterialLocal = Glow; break;
				case 2: OverlayMaterialLocal = GlowFade; break;
				case 3: OverlayMaterialLocal = GlowP; break;
				}

				//if (csgo->local->IsScoped())
				//	interfaces.render_view->SetBlend(vars.visuals.trnsparency / 100.f);
				
				/*for (auto& i : csgo->realmatrix)
				{
					i[0][3] += info.origin.x;
					i[1][3] += info.origin.y;
					i[2][3] += info.origin.z;
				}*/
				if (csgo->fakematrix != nullptr && !csgo->game_rules->IsFreezeTime())
				{
					if (!vars.visuals.interpolated_dsy) {
						for (auto& i : csgo->fakematrix)
						{
							i[0][3] += model_origin.x;
							i[1][3] += model_origin.y;
							i[2][3] += model_origin.z;
						}
					}

					matrix mat1[128];
					if (GetSmoothMatrix(csgo->local, mat1))
					{
						static IMaterial* OverridedMaterial = nullptr;
						static IMaterial* OverlayMaterial = nullptr;

						switch (vars.visuals.misc_chams[desync].overlay)
						{
						case 1: OverlayMaterial = Glow; break;
						case 2: OverlayMaterial = GlowFade; break;
						case 3: OverlayMaterial = GlowP; break;
						}

						switch (vars.visuals.misc_chams[desync].material)
						{
						case 0: OverridedMaterial = Regular; break;
						case 1: OverridedMaterial = Flat; break;
						case 2: OverridedMaterial = Bubble; break;
						case 3: OverridedMaterial = Bubble; break;
						}

						

						if (vars.visuals.misc_chams[desync].enable)
						{
							OverridedMaterial->ColorModulate(
								vars.visuals.misc_chams[desync].clr[0] / 255.f,
								vars.visuals.misc_chams[desync].clr[1] / 255.f, vars.visuals.misc_chams[desync].clr[2] / 255.f);

							OverridedMaterial->AlphaModulate(vars.visuals.misc_chams[desync].clr[3] / 255.f);
							OverridedMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

							float clr[3] = {
								(vars.visuals.misc_chams[desync].clr[0] / 255.f) * vars.visuals.misc_chams[desync].chams_brightness / 100.f,
								(vars.visuals.misc_chams[desync].clr[1] / 255.f) * vars.visuals.misc_chams[desync].chams_brightness / 100.f,
								(vars.visuals.misc_chams[desync].clr[2] / 255.f) * vars.visuals.misc_chams[desync].chams_brightness / 100.f
							};
							if (vars.visuals.misc_chams[desync].material == 2)
							{
								OverridedMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
									(vars.visuals.misc_chams[desync].clr[0] / 705.f) * 0.24f,
									(vars.visuals.misc_chams[desync].clr[1] / 705.f) * 0.24f,
									(vars.visuals.misc_chams[desync].clr[2] / 705.f) * 0.24f);
					
								OverridedMaterial->AlphaModulate((vars.visuals.misc_chams[desync].clr[3]/ 500.f) * 0.43f);
							}
							float backup_clr[3];
							interfaces.render_view->GetColorModulation(backup_clr);
							interfaces.render_view->SetColorModulation(clr);
							interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterial);
							DrawModel(ecx, results, info, !vars.visuals.interpolated_dsy ? csgo->fakematrix : mat1, flex_weights, flex_delayed_weights, model_origin, flags);
							interfaces.render_view->SetColorModulation(backup_clr);

							if (vars.visuals.misc_chams[desync].overlay > 0) {
								auto pVar = OverlayMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr);
								pVar->SetVecValue(vars.visuals.misc_chams[desync].glow_clr[0] / 255.f,
									vars.visuals.misc_chams[desync].glow_clr[1] / 255.f, vars.visuals.misc_chams[desync].glow_clr[2] / 255.f);

								OverlayMaterial->AlphaModulate(vars.visuals.misc_chams[desync].glow_clr[3] / 255.f);
								OverlayMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
								if (vars.visuals.misc_chams[desync].overlay == 3) {
									pVar->SetVecValue(vars.visuals.misc_chams[desync].glow_clr[0] / 510.f,
										vars.visuals.misc_chams[desync].glow_clr[1] / 510.f, vars.visuals.misc_chams[desync].glow_clr[2] / 510.f);
									OverlayMaterial->AlphaModulate(vars.visuals.misc_chams[desync].glow_clr[3] / 510.f);
								}
								interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterial);
								DrawModel(ecx, results, info, !vars.visuals.interpolated_dsy ? csgo->fakematrix : mat1, flex_weights, flex_delayed_weights, model_origin, flags);
								interfaces.models.model_render->ForcedMaterialOverride(nullptr);
								interfaces.render_view->SetColorModulation(backup_clr);
							}
							//
						}
					}

					if (!vars.visuals.interpolated_dsy) {
						for (auto& i : csgo->fakematrix)
						{
							i[0][3] -= model_origin.x;
							i[1][3] -= model_origin.y;
							i[2][3] -= model_origin.z;
						}
					}
				}

				if (vars.visuals.localchams || vars.antiaim.fakepeeking)
				{
					OverridedMaterialLocal->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, true);

					
					OverridedMaterialLocal->ColorModulate(
						vars.visuals.localchams_color[0] / 255.f, vars.visuals.localchams_color[1] / 255.f, vars.visuals.localchams_color[2] / 255.f);
					float clr[3] = {
						(vars.visuals.localchams_color[0] / 255.f) * vars.visuals.local_chams_brightness / 100.f,
						(vars.visuals.localchams_color[1] / 255.f) * vars.visuals.local_chams_brightness / 100.f,
						(vars.visuals.localchams_color[2] / 255.f) * vars.visuals.local_chams_brightness / 100.f
					};
					float backup_clr[3];
					interfaces.render_view->GetColorModulation(backup_clr);
					interfaces.render_view->SetColorModulation(clr);

					OverridedMaterialLocal->AlphaModulate(vars.visuals.localchams_color[3] / 255.f);
					OverridedMaterialLocal->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					if (vars.antiaim.fakepeeking) {
						OverridedMaterialLocal->AlphaModulate(0.0);
					}
					if (vars.visuals.localchamstype == 2)
					{
						OverridedMaterialLocal->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
							(vars.visuals.localchams_color[0] / 705.f) * 0.24f,
							(vars.visuals.localchams_color[1] / 705.f) * 0.24f,
							(vars.visuals.localchams_color[2] / 705.f) * 0.24f);
			
						OverridedMaterialLocal->AlphaModulate((vars.visuals.localchams_color[3]/ 500.f) * 0.43f);
					}
					interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterialLocal);
					DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

					if (vars.visuals.local_chams.overlay > 0 && (!vars.antiaim.fakepeeking)) {
						auto pVar = OverlayMaterialLocal->FindVar(hs::envmaptint::s().c_str(), nullptr);
						pVar->SetVecValue(vars.visuals.local_glow_color[0] / 255.f,
							vars.visuals.local_glow_color[1] / 255.f, vars.visuals.local_glow_color[2] / 255.f);

						OverlayMaterialLocal->AlphaModulate(vars.visuals.local_glow_color[3] / 255.f);
						OverlayMaterialLocal->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, false);
						if (vars.visuals.local_chams.overlay == 3) {
							pVar->SetVecValue(vars.visuals.local_glow_color[0] / 510.f,
								vars.visuals.local_glow_color[1] / 510.f, vars.visuals.local_glow_color[2] / 510.f);
							OverlayMaterialLocal->AlphaModulate(vars.visuals.local_glow_color[3] / 510.f);
						}
						interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterialLocal);
						DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
						interfaces.render_view->SetColorModulation(backup_clr);
					}
				}
				else {
					interfaces.models.model_render->ForcedMaterialOverride(nullptr);
					DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
					if (vars.visuals.local_chams.overlay > 0) {
						auto pVar = OverlayMaterialLocal->FindVar(hs::envmaptint::s().c_str(), nullptr);
						pVar->SetVecValue(vars.visuals.local_glow_color[0] / 255.f,
							vars.visuals.local_glow_color[1] / 255.f, vars.visuals.local_glow_color[2] / 255.f);

						OverlayMaterialLocal->AlphaModulate(vars.visuals.local_glow_color[3] / 255.f);
						OverlayMaterialLocal->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, false);
						if (vars.visuals.local_chams.overlay == 3) {
							pVar->SetVecValue(vars.visuals.local_glow_color[0] / 510.f,
								vars.visuals.local_glow_color[1] / 510.f, vars.visuals.local_glow_color[2] / 510.f);
							OverlayMaterialLocal->AlphaModulate(vars.visuals.local_glow_color[3] / 510.f);
						}
						interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterialLocal);
						DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
					}
				}
			}
		}
	}
	else if (strstr(ModelName, "arms") != nullptr)
	{
		if (csgo->local && csgo->local->isAlive() && !interfaces.input->m_fCameraInThirdPerson)
		{
			static IMaterial* OverridedMaterial = nullptr;
			static IMaterial* OverlayMaterial = nullptr;

			switch (vars.visuals.misc_chams[arms].overlay)
			{
			case 1: OverlayMaterial = Glow; break;
			case 2: OverlayMaterial = GlowFade; break;
			case 3: OverlayMaterial = GlowP; break;
			}

			switch (vars.visuals.misc_chams[arms].material)
			{
			case 0: OverridedMaterial = Regular; break;
			case 1: OverridedMaterial = Flat; break;
			case 2: OverridedMaterial = Bubble; break;
			case 3: OverridedMaterial = Bubble; break;
			}

			

			if (vars.visuals.misc_chams[arms].enable)
			{
				OverridedMaterial->ColorModulate(
					vars.visuals.misc_chams[arms].clr.get_red() / 255.f,
					vars.visuals.misc_chams[arms].clr.get_green() / 255.f,
					vars.visuals.misc_chams[arms].clr.get_blue() / 255.f);
				OverridedMaterial->AlphaModulate(vars.visuals.misc_chams[arms].clr.get_alpha() / 255.f);
				OverridedMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				float clr[3] = {
					(vars.visuals.misc_chams[arms].clr.get_red() / 255.f) * vars.visuals.misc_chams[arms].chams_brightness / 100.f,
					(vars.visuals.misc_chams[arms].clr.get_green() / 255.f) * vars.visuals.misc_chams[arms].chams_brightness / 100.f,
					(vars.visuals.misc_chams[arms].clr.get_blue() / 255.f) * vars.visuals.misc_chams[arms].chams_brightness / 100.f
				};
				float backup_clr[3];
				interfaces.render_view->GetColorModulation(backup_clr);
				interfaces.render_view->SetColorModulation(clr);
				if (vars.visuals.misc_chams[arms].material == 2)
				{
					OverridedMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
						(vars.visuals.misc_chams[arms].clr[0] / 705.f) * 0.24f,
						(vars.visuals.misc_chams[arms].clr[1] / 705.f) * 0.24f,
						(vars.visuals.misc_chams[arms].clr[2] / 705.f) * 0.24f);
			
					OverridedMaterial->AlphaModulate((vars.visuals.misc_chams[arms].clr[3]/ 500.f) * 0.43f);
				}
				interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterial);
				DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

				if (vars.visuals.misc_chams[arms].overlay > 0) {
					auto pVar = OverlayMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr);
					pVar->SetVecValue(vars.visuals.misc_chams[arms].glow_clr[0] / 255.f,
						vars.visuals.misc_chams[arms].glow_clr[1] / 255.f, vars.visuals.misc_chams[arms].glow_clr[2] / 255.f);

					OverlayMaterial->AlphaModulate(vars.visuals.misc_chams[arms].glow_clr[3] / 255.f);
					OverlayMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					if (vars.visuals.misc_chams[arms].overlay == 3) {
						pVar->SetVecValue(vars.visuals.misc_chams[arms].glow_clr[0] / 510.f,
							vars.visuals.misc_chams[arms].glow_clr[1] / 510.f, vars.visuals.misc_chams[arms].glow_clr[2] / 510.f);
						OverlayMaterial->AlphaModulate(vars.visuals.misc_chams[arms].glow_clr[3] / 510.f);
					}
					interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterial);
					DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
				}
			}
		}
	}
	else if (strstr(ModelName, "v_") != nullptr || (strstr(ModelName, "w_") != nullptr && strstr(ModelName, "weapons") != nullptr))
	{
		if (csgo->local && csgo->local->isAlive() && ((!interfaces.input->m_fCameraInThirdPerson && strstr(ModelName, "v_") != nullptr) || strstr(ModelName, "w_") != nullptr))
		{
			static IMaterial* OverridedMaterial = nullptr;
			static IMaterial* OverlayMaterial = nullptr;

			switch (vars.visuals.misc_chams[weapon].overlay)
			{
			case 1: OverlayMaterial = Glow; break;
			case 2: OverlayMaterial = GlowFade; break;
			case 3: OverlayMaterial = GlowP; break;
			}

			switch (vars.visuals.misc_chams[weapon].material)
			{
			case 0: OverridedMaterial = Regular; break;
			case 1: OverridedMaterial = Flat; break;
			case 2: OverridedMaterial = Bubble; break;
			case 3: OverridedMaterial = Bubble; break;
			}

		
			if (vars.visuals.misc_chams[weapon].enable)
			{
				OverridedMaterial->ColorModulate(
					vars.visuals.misc_chams[weapon].clr[0] / 255.f, vars.visuals.misc_chams[weapon].clr[1] / 255.f, vars.visuals.misc_chams[weapon].clr[2] / 255.f);
				OverridedMaterial->AlphaModulate(vars.visuals.misc_chams[weapon].clr[3] / 255.f);
				OverridedMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				float clr[3] = {
					(vars.visuals.misc_chams[weapon].clr[0] / 255.f) * vars.visuals.misc_chams[weapon].chams_brightness / 100.f,
					(vars.visuals.misc_chams[weapon].clr[1] / 255.f) * vars.visuals.misc_chams[weapon].chams_brightness / 100.f,
					(vars.visuals.misc_chams[weapon].clr[2] / 255.f) * vars.visuals.misc_chams[weapon].chams_brightness / 100.f
				};
				float backup_clr[3];
				interfaces.render_view->GetColorModulation(backup_clr);
				interfaces.render_view->SetColorModulation(clr);
				if (vars.visuals.misc_chams[weapon].material == 2)
				{
					OverridedMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr)->SetVecValue(
						(vars.visuals.misc_chams[weapon].clr[0] / 705.f) * 0.24f,
						(vars.visuals.misc_chams[weapon].clr[1] / 705.f) * 0.24f,
						(vars.visuals.misc_chams[weapon].clr[2] / 705.f) * 0.24f);
			
					OverridedMaterial->AlphaModulate((vars.visuals.misc_chams[weapon].clr[3]/ 500.f) * 0.43f);
				}

				interfaces.models.model_render->ForcedMaterialOverride(OverridedMaterial);
				DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);

				if (vars.visuals.misc_chams[weapon].overlay > 0) {
					auto pVar = OverlayMaterial->FindVar(hs::envmaptint::s().c_str(), nullptr);
					pVar->SetVecValue(vars.visuals.misc_chams[weapon].glow_clr[0] / 255.f,
						vars.visuals.misc_chams[weapon].glow_clr[1] / 255.f, vars.visuals.misc_chams[weapon].glow_clr[2] / 255.f);

					OverlayMaterial->AlphaModulate(vars.visuals.misc_chams[weapon].glow_clr[3] / 255.f);
					OverlayMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					if (vars.visuals.misc_chams[weapon].overlay == 3) {
						pVar->SetVecValue(vars.visuals.misc_chams[weapon].glow_clr[0] / 510.f,
							vars.visuals.misc_chams[weapon].glow_clr[1] / 510.f, vars.visuals.misc_chams[weapon].glow_clr[2] / 510.f);
						OverlayMaterial->AlphaModulate(vars.visuals.misc_chams[weapon].glow_clr[3] / 510.f);
					}
					interfaces.models.model_render->ForcedMaterialOverride(OverlayMaterial);
					DrawModel(ecx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
				}
				interfaces.render_view->SetColorModulation(backup_clr);
			}
		}
	}
	skip:
	if (Optimization > 3)
		Optimization = 0;
	Optimization++;
}