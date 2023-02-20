#pragma once
#include "Hooks.h"
#include <intrin.h>

int32_t __fastcall Hooked_IsBoxVisible(IEngineClient* engine_client, uint32_t, Vector& min, Vector& max)
{
	static auto BoxVisible = g_pEngineHook->GetOriginal< BoxVisibleFn >(32);

	static const auto ret = (hs::ret_engine::s().c_str());

	if (!memcmp(_ReturnAddress(), ret, 10))
		return 1;

	return BoxVisible(engine_client, min, max);
}


Vector* __fastcall hkGetEyeAngles(IBasePlayer* ecx) {

	static int* WantedReturnAddress1 = (int*)csgo->Utils.FindPatternIDA(GetModuleHandle("client.dll"), "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?"); //Update Animations X/Y
	static int* WantedReturnAddress2 = (int*)csgo->Utils.FindPatternIDA(GetModuleHandle("client.dll"), "F3 0F 10 55 ? 51 8B 8E ? ? ? ?");                                    //Update Animations X/Y
	static int* WantedReturnAddress3 = (int*)csgo->Utils.FindPatternIDA(GetModuleHandle("client.dll"), "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5");                       //Retarded valve fix

	//static auto oGetEyeAngles = hooker::h.original(&hkGetEyeAngles);

	if (!csgo->local || !csgo->local->isAlive())
		return H::GetEyeAnglesO(ecx);

	if (_ReturnAddress() != WantedReturnAddress1 && _ReturnAddress() != WantedReturnAddress2 && _ReturnAddress() != WantedReturnAddress3)
		return H::GetEyeAnglesO(ecx);

	if (!ecx || ecx != csgo->local)
		return H::GetEyeAnglesO(ecx);

	return &csgo->FakeAngle;
}

bool __fastcall Hooked_IsHLTV(IEngineClient* IEngineClient, uint32_t)
{
	static auto IsHLTV = g_pEngineHook->GetOriginal< IsHLTVFn >(93);

	static const auto return_to_setup_velocity = (csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::setup_vel::s().c_str()));
	static const auto return_to_accumulate_layers = (csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::accum_layers::s().c_str()));

	if (_ReturnAddress() == (uint32_t*)(return_to_accumulate_layers) && IEngineClient->IsInGame())
		return true;

	if (_ReturnAddress() == (uint32_t*)(return_to_setup_velocity) && csgo->UpdateMatrix && IEngineClient->IsInGame())
		return true;

	return IsHLTV(IEngineClient);
}

void __fastcall Hooked_FireEvent(IEngineClient* ecx)
{
	static auto FireEvent = g_pEngineHook->GetOriginal< fire_event_t >(59);

	auto ei = *(CEventInfo**)(std::uintptr_t(csgo->client_state) + 0x4E64);

	CEventInfo* next = nullptr;

	if (!ei)
		return FireEvent(ecx);

	do {
		next = ei->m_next;
		ei->m_fire_delay = 0.f;
		ei = next;
	} while (next);

	return FireEvent(ecx);
}