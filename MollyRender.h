#pragma once
#include "Hooks.h"


void __fastcall Hooked_RandomColor_InitNewParticlesScalar(C_INIT_RandomColor* thisPtr, void* edx, CParticleCollection* pParticles, int start_p, int nParticleCount, int nAttributeWriteMask, void* pContext) {
	Vector o_min = thisPtr->m_flNormColorMin;
	Vector o_max = thisPtr->m_flNormColorMax;

	const char* mat_name = *(char**)(*(uintptr_t*)((uintptr_t)pParticles + 0x48) + 0x40);
	if(mat_name == "particle\\fire_burning_character\\fire_env_fire.vmt" || mat_name == "particle\\fire_burning_character\\fire_env_fire_depthblend.vmt" ||
		mat_name == "particle\\fire_burning_character\\fire_burning_character_depthblend.vmt" || mat_name == "particle\\fire_burning_character\\fire_burning_character.vmt"
		|| mat_name == "particle\\fire_burning_character\\fire_burning_character_nodepth.vmt" || mat_name == "particle\\particle_flares\\particle_flare_001.vmt"
		|| mat_name == "particle\\particle_flares\\particle_flare_004.vmt" || mat_name == "particle\\particle_flares\\particle_flare_004b_mod_ob.vmt" || mat_name ==
		"particle\\particle_flares\\particle_flare_004b_mod_z.vmt" || mat_name == "particle\\fire_explosion_1\\fire_explosion_1_bright.vmt"
		|| mat_name == "particle\\fire_explosion_1\\fire_explosion_1b.vmt" || mat_name == "particle\\fire_particle_4\\fire_particle_4.vmt" || 
		mat_name == "particle\\fire_explosion_1\\fire_explosion_1_oriented.vmt") {
		thisPtr->m_flNormColorMin = thisPtr->m_flNormColorMax = Vector(0, 1, 0);
	}

	H::oInitNewParticalScallar(thisPtr,edx, pParticles, start_p, nParticleCount, nAttributeWriteMask, pContext);

	thisPtr->m_flNormColorMin = o_min;
	thisPtr->m_flNormColorMax = o_max;
}