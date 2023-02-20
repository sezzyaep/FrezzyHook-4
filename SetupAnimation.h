#pragma once
#include "Hooks.h"

void __fastcall Hooked_StandardBlendingRules(IBasePlayer* player, void* edx, c_studio_hdr* hdr, Vector* pos, Quaternion* q, float curTime, int boneMask)
{
	if (!player) {
		return H::StandardBlendingRules(player, hdr, pos, q, curTime, boneMask);
	}

	player->GetEffects() |= 8;
	H::StandardBlendingRules(player, hdr, pos, q, curTime, boneMask);
	player->GetEffects() &= ~8;
}

void __fastcall Hooked_BuildTransformations(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7) {
	H::BuildTransformations(ecx, a2, a3, a4, a5, a6, a7);
}


bool __fastcall Hooked_SetupBones(void* ecx, void* edx, matrix* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	

	if (csgo->DoUnload)
		return H::SetupBones(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	auto entity = (IBasePlayer*)((uintptr_t)ecx - 4);
	if (entity == nullptr || csgo->local == nullptr || !pBoneToWorldOut)
		return H::SetupBones(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	if (entity->IsPlayer() && entity->isAlive()) {

		*(int*)((DWORD)entity + 0x2698) = 0;
		boneMask |= 0x200;

		if (entity == csgo->local) {
			return H::SetupBones(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
		}
		else {
			if (entity->GetTeam() == csgo->local->GetTeam())
				return H::SetupBones(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
			else {
				if (csgo->UpdateMatrix)
					return H::SetupBones(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
				else {
					memcpy(pBoneToWorldOut, entity->GetBoneCache().Base(), sizeof(matrix) * nMaxBones);
					return true;
				}
			}
		}
	}
	else
		return H::SetupBones(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}

bool __fastcall Hooked_ShouldSkipAnimFrame()
{
	return false;
}

void __fastcall Hooked_DoExtraBoneProcessing(IBasePlayer* player, uint32_t, c_studio_hdr* hdr, Vector* pos, Quaternion* q,
	const matrix& mat, uint8_t* bone_computed, void* context)
{
	if (!player) {
		H::DoExtraBoneProcessing(player, hdr, pos, q, mat, bone_computed, context);
		return;
	}

	if (player == csgo->local)
		return;

	auto animstate = player->GetPlayerAnimState();
	if (!animstate || !player) {
		H::DoExtraBoneProcessing(player, hdr, pos, q, mat, bone_computed, context);
		return;
	}

	/* backup on ground */
	const auto o_on_ground = animstate->m_on_ground;

	/* set it to false */
	animstate->m_on_ground = false;

	/* set csgo do their stuff */
	H::DoExtraBoneProcessing(player, hdr, pos, q, mat, bone_computed, context);

	/* restore */
	animstate->m_on_ground = o_on_ground;
}

void __fastcall Hooked_UpdateClientSideAnimation(IBasePlayer* player, uint32_t)
{
	H::UpdateClientSideAnimation(player);
}