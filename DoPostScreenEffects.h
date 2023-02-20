#pragma once
#include "Hooks.h"
void Hooked_ImpactCallback(const CEffectData& data)
{
	auto org = data.origin;

	if (vars.visuals.bullet_impact && !org.IsZero() && csgo->impact_time > 0.f
		&& (csgo->impact_time - interfaces.global_vars->curtime)
		<= (interfaces.global_vars->interval_per_tick + TICKS_TO_TIME(2))) {
		csgo->impact_origin = data.origin; // �� ����� ��������� ������� ���������� ���? ���� ������� ������ �������, �������

		csgo->impact_time = 0.f;
	}

	H::ImpactCallback(data);
}
bool __fastcall Hooked_DoPostScreenEffects(void* ecx, void* edx, CViewSetup* setup)
{
	static auto DoPostScreenEffects = g_pClientModeHook->GetOriginal< DoPostScreenEffectsFn >(44);

	if (csgo->DoUnload)
		return DoPostScreenEffects(ecx, setup);
	Chams->OnPostScreenEffects();
	glow->Draw();

	return DoPostScreenEffects(ecx, setup);
}