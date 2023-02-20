#pragma once
#include "Hooks.h"
#include "RageBacktracking.h"
#include "Resolver.h"
#include "AnimationFix.h"
#include "SetupAnimation.h"
#include <intrin.h>
#include <random>
#include "Ragebot.h"

bool __fastcall Hooked_GetBool(void* pConVar, void* edx)
{
	static auto SvCheatsGetBool = g_pGetBoolHook->GetOriginal< SvCheatsGetBoolFn >(13);

	if (csgo->DoUnload)
		return SvCheatsGetBool(pConVar);

	static auto CAM_THINK = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::retn_camera::s().c_str());
	if (_ReturnAddress() == CAM_THINK)
		return true;

	return SvCheatsGetBool(pConVar);
}

static bool enabledtp = false, check = false;

static DWORD* FindHudElement(const char* name)
{
	static DWORD* pThis = nullptr;

	if (!pThis)
	{
		static auto pThisSignature = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 5D 08");

		if (pThisSignature)
			pThis = *reinterpret_cast<DWORD**>(pThisSignature + 0x1);
	}

	if (!pThis)
		return 0;

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (DWORD*)find_hud_element(pThis, name); //-V204
}

void UpdateCSGOKeyBinds()
{
	if (vars.antiaim.zi180)
	{
		vars.ragebot.autoinvert = false;
		vars.antiaim.LBYFlick = false;
	}
	if (vars.antiaim.LBYFlick)
	{
		vars.ragebot.autoinvert = false;
	}
	if (vars.misc.thirdperson)
	{
		if (!check)
			enabledtp = !enabledtp;
		check = true;
	}
	else
		check = false;

	if (!vars.ragebot.autoinvert && !vars.antiaim.LBYFlick)
	{
		if (vars.antiaim.zi180)
		{
			static bool Sw;
			static bool FL_Switch;
			static bool FL_Switch2;
			if (Sw) {
				csgo->SwitchAA = true;
				Sw = false;
				if (FL_Switch)
				{
					vars.antiaim.FakelagVarr = 1;
					FL_Switch = false;
				}
				else
				{
					vars.antiaim.FakelagVarr = 16;
					FL_Switch = true;
				}
			}
			else {
				csgo->SwitchAA = false;
				Sw = true;
				if (FL_Switch2)
				{
					vars.antiaim.FakelagVarr = 16;
					FL_Switch2 = false;
				}
				else
				{
					vars.antiaim.FakelagVarr = 1;
					FL_Switch2 = true;
				}
			}
			vars.antiaim.speciallag = true;
		}
		else
		{
			vars.antiaim.speciallag = false;
			if(!vars.antiaim.onshottt && vars.antiaim.antionshotretard)
			{
				if (csgo->visiblee)
				{
					csgo->SwitchAA = !csgo->SwitchAA;
				}
			}
			else if (vars.antiaim.antionshotretard && vars.antiaim.onshottt)
			{
				if (csgo->visiblee)
				{
					csgo->SwitchAA = !csgo->SwitchAA;
				}
			}
			else if (vars.antiaim.inverter->active)
			{
				csgo->SwitchAA = true;
			}
			else if (!vars.antiaim.inverter->active)
			{
				csgo->SwitchAA = false;
			}

		}
	}
	else if(vars.ragebot.autoinvert && !vars.antiaim.LBYFlick)
	{
		vars.antiaim.AI = true;
		if (!vars.ragebot.autospeed)
		{
			static int counter = 0;
			int S1 = vars.ragebot.invertspeed;
			int S2 = vars.ragebot.invertspeed * 2;
			if (counter < S1)
			{
				csgo->SwitchAA = true;
				vars.antiaim.LBYRecharge = false;
			}
			else if (counter > S1 && counter < S2)
			{
				csgo->SwitchAA = false;
				vars.antiaim.LBYRecharge = true;
			}
			else if (counter > S2)
			{
				counter = 0;
			}
			counter++;
		}
		else
		{
			static bool Sw;
			static bool Sw2;
			if (Sw) {
				csgo->SwitchAA = true;
				vars.antiaim.LBYRecharge = false;
		
			}
			else {
				csgo->SwitchAA = false;
				vars.antiaim.LBYRecharge = true;
	
			}
			
			Sw2 = !Sw2;
			if (Sw2)
			{
				Sw = !Sw;
			}
		}
		vars.antiaim.speciallag = false;
	}
	else if (vars.antiaim.LBYFlick && !vars.ragebot.autoinvert)
	{
		static int counter = 0;
		int S1 = vars.antiaim.LBYFlick_Intervals;
		int S2 = S1 + vars.antiaim.LBYFlick_Intervals_S + 6;
		if (vars.antiaim.LBYF->active)
		{
			if (counter < S1)
			{
				vars.antiaim.LBYRecharge = true;
				if (vars.antiaim.inv_LBYF->active)
				{
					
					csgo->SwitchAA = false;
				}
				else
				{
					csgo->SwitchAA = true;
				}
			}
			else if (counter > S1 && counter < S2)
			{
				vars.antiaim.LBYRecharge = false;
				if (vars.antiaim.inv_LBYF->active)
				{
					csgo->SwitchAA = true;
				}
				else
				{
					csgo->SwitchAA = false;
				}
			}
			else if (counter > S2)
			{
				vars.antiaim.LBYRecharge = true;
				counter = 0;
			}
			counter++;

		}
		vars.antiaim.speciallag = false;
	}

}

void __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto FrameStageNotify = g_pClientHook->GetOriginal< FrameStageNotifyFn >(37);

	csgo->updatelocalplayer();

	if (csgo->client_state != nullptr) {
		if (csgo->g_pNetChannelHook && csgo->g_pNetChannelHook->hooked) {
			if (csgo->client_state->pNetChannel != nullptr) {
				uintptr_t* vtable = *(uintptr_t**)csgo->client_state->pNetChannel;

				if (vtable != csgo->g_pNetChannelHook->shadow_vtable) {
					csgo->g_pNetChannelHook.reset();
				}
			}
			else
				csgo->g_pNetChannelHook.reset();
		}
	}

	if (!interfaces.engine->IsConnected() || !interfaces.engine->IsInGame())
		csgo->mapChanged = true;

	if (!csgo->local || !csgo->local->isAlive()) {
		csgo->ForceOffAA = false;

		if (curStage == FRAME_NET_UPDATE_END) {
			for (int i = 1; i < 65; i++) {
				auto entity = interfaces.ent_list->GetClientEntity(i);
				if (entity != nullptr && entity->IsPlayer() && entity != csgo->local) {
					entity->GetClientSideAnims() = csgo->EnableBones = true;
					entity->UpdateClientSideAnimation();
					entity->GetClientSideAnims() = csgo->EnableBones = false;
				}
			}
			csgo->disable_dt = false;
		}
		return FrameStageNotify(curStage);
	}

	if (curStage == FRAME_RENDER_START)
	{
		if (csgo->game_rules == nullptr || csgo->mapChanged) {
			csgo->game_rules = **reinterpret_cast<CCSGameRules***>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
				g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
				hs::game_rules::s().c_str()) + 0x1);
		}
	}

	if (curStage == FRAME_RENDER_START) {
		features->Visuals->NightMode();
	}

	UpdateCSGOKeyBinds();

	static auto cycle = 0.f;
	static auto anim_time = 0.f;

	if (interfaces.engine->IsConnected() && interfaces.engine->IsInGame())
	{
		static int m_iLastCmdAck = 0;
		static float m_flNextCmdTime = 0.f;

		if (csgo->local && csgo->local->isAlive()) {
			if (csgo->client_state && (m_iLastCmdAck != csgo->client_state->nLastCommandAck || m_flNextCmdTime != csgo->client_state->flNextCmdTime))
			{
				if (csgo->g_flVelMod != csgo->local->GetVelocityModifier())
				{
					*(bool*)((uintptr_t)interfaces.prediction + 0x24) = true; // magic value
					csgo->g_flVelMod = csgo->local->GetVelocityModifier();
				}

				m_iLastCmdAck = csgo->client_state->nLastCommandAck;
				m_flNextCmdTime = csgo->client_state->flNextCmdTime;
			}

		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
		{
			if (csgo->local && csgo->local->isAlive()) {
				auto viewmodel =csgo->local->GetViewModel();

				if (viewmodel && CEnginePrediction::Get().viewmodel_data.weapon == viewmodel->GetWeapon() && CEnginePrediction::Get().viewmodel_data.sequence == viewmodel->GetSequence()) //-V807
				{
					viewmodel->GetCycle() = CEnginePrediction::Get().viewmodel_data.cycle;
					viewmodel->GetAnimtime() = CEnginePrediction::Get().viewmodel_data.animation_time;
				}
			}
			for (int i = 1; i < 65; i++) {
				auto entity = interfaces.ent_list->GetClientEntity(i);
				if (!entity || !entity->isAlive() || !entity->IsPlayer() || entity->IsDormant() || entity->EntIndex() == interfaces.engine->GetLocalPlayer())
					continue;

				const auto var_map = reinterpret_cast<uintptr_t>(entity) + 36;

				for (auto index = 0; index < *reinterpret_cast<int*>(var_map + 20); index++)
					*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(var_map) + index * 12) = 0;
			}
		}

		if (curStage == FRAME_NET_UPDATE_END) {
			g_Animfix->UpdatePlayers();
		}

		if (curStage == FRAME_RENDER_START)
		{
			if (csgo->local->isAlive()) {
				if (enabledtp) {
					interfaces.input->m_fCameraInThirdPerson = true;
					if (vars.ragebot.random_resolver) {
						*reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(csgo->local) + 0x31D8) = csgo->VisualAngle;
					}
					else {
						*reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(csgo->local) + 0x31D8) = csgo->cmd->viewangles;
					}
				}
				g_Animfix->ApplyLocalPlayer();
				interfaces.input->m_fCameraInThirdPerson = false;


				if (vars.visuals.remove & 4)
					csgo->local->GetFlashDuration() = 0.f;

				static float old_simtime = 0.f;
				if (old_simtime != csgo->local->GetSimulationTime())
				{
					csgo->all_data[0].sim_time = old_simtime;
					csgo->all_data[0].interp_time = 0.f;
					csgo->all_data[0].origin = csgo->local->GetAbsOriginVec();
					old_simtime = csgo->local->GetSimulationTime();
				}

				csgo->all_data[1].sim_time = csgo->local->GetSimulationTime();

				if (!csgo->send_packet) {
					csgo->all_data[1].interp_time = 0.f;
					csgo->all_data[1].origin = csgo->local->GetAbsOriginVec();
				}
				auto animstate = csgo->local->GetPlayerAnimState();
				if (csgo->animstate && animstate)
					csgo->desync_angle = clamp(fabsf(Math::NormalizeYaw(csgo->animstate->m_eye_yaw - animstate->m_fGoalFeetYaw)), 0.f, 58.f);
			}
			features->BulletTracer->Proceed();
		}

		csgo->disable_dt = false;
		csgo->ForceOffAA = false;
	}
	else {
		csgo->disable_dt = false;
		csgo->mapChanged = true;
		csgo->dt_charged = false;
		csgo->skip_ticks = 0;
	}
	FrameStageNotify(curStage);

	static DWORD* death_thing = nullptr;

	if (csgo->local->isAlive()) {
		if (!death_thing) {
			death_thing = FindHudElement("CCSGO_HudDeathNotice");
		}
		else
		{
			auto local_death_notice = (float*)((uintptr_t)death_thing + 0x50);

			if (local_death_notice)
				*local_death_notice = vars.visuals.PreserveKills ? FLT_MAX : 1.5f;

			if (csgo->ResetKillfeed)
			{
				csgo->ResetKillfeed = false;

				using Fn = void(__thiscall*)(uintptr_t);
				static auto clear_notices = (Fn)csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 0C 53 56 8B 71 58");

				clear_notices((uintptr_t)death_thing - 0x14);
			}
		}
	}
	else
	{
		death_thing = 0;
	}
}

bool __fastcall Hooked_ShouldDrawFog(void* ecx, void* edx) {
	return !(vars.visuals.remove & 32);
}

bool __fastcall Hooked_ShouldDrawShadow(void*, uint32_t) {
	return !(vars.visuals.remove & 64);
}
