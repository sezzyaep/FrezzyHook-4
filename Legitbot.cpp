#include "Hooks.h"
#include "Legitbot.h"
#include "RageBackTracking.h"
#include "EnginePrediction.h"
#include "AntiAims.h"
#include "AnimationFix.h"
void PLegitbot::TraceLineNonMemoryIntensive(Vector& start, Vector& end, unsigned int mask, IBasePlayer* ignore, trace_t* trace)
{
	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = ignore;

	interfaces.trace->TraceRay(ray, mask, &filter, trace);
}

bool PLegitbot::IsBreakableEntity(IBasePlayer* e)
{
	if (!e || !e->EntIndex())
		return false;

	static auto is_breakable_fn = reinterpret_cast<bool(__thiscall*)(IBasePlayer*)>(
		csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()), hs::is_breakable::s().c_str()));

	const auto result = is_breakable_fn(e);
	auto class_id = e->GetClientClass()->m_ClassID;
	if (!result &&
		(class_id == 10 ||
			class_id == 31 ||
			(class_id == 11 && e->GetCollideable()->get_solid() == 1)))
		return true;

	return result;
}

bool PLegitbot::PTraceToExit(trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace)
{
	Vector end;
	float distance = 0.f;
	signed int distance_check = 20;
	int first_contents = 0;

	do
	{
		distance += 3.5f;
		end = start + dir * distance;

		if (!first_contents) first_contents = interfaces.trace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		int point_contents = interfaces.trace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		if (!(point_contents & (MASK_SHOT_HULL | CONTENTS_HITBOX)) || point_contents & CONTENTS_HITBOX && point_contents != first_contents)
		{
			Vector new_end = end - (dir * 4.f);

			Ray_t ray;
			ray.Init(end, new_end);

			interfaces.trace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

			if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX)
			{
				TraceLineNonMemoryIntensive(end, start, MASK_SHOT_HULL | CONTENTS_HITBOX, exit_trace->m_pEnt, exit_trace);

				if (exit_trace->DidHit() && !exit_trace->startsolid) return true;

				continue;
			}

			if (exit_trace->DidHit() && !exit_trace->startsolid)
			{
				if (enter_trace->surface.flags & SURF_NODRAW || !(exit_trace->surface.flags & SURF_NODRAW)) {
					if (exit_trace->plane.normal.Dot(dir) <= 1.f)
						return true;

					continue;
				}

				if (IsBreakableEntity(enter_trace->m_pEnt)
					&& IsBreakableEntity(exit_trace->m_pEnt))
					return true;

				continue;
			}

			if (exit_trace->surface.flags & SURF_NODRAW)
			{
				if (IsBreakableEntity(enter_trace->m_pEnt)
					&& IsBreakableEntity(exit_trace->m_pEnt))
				{
					return true;
				}
				else if (!(enter_trace->surface.flags & SURF_NODRAW))
				{
					continue;
				}
			}

			if ((!enter_trace->m_pEnt
				|| enter_trace->m_pEnt->EntIndex() == 0)
				&& (IsBreakableEntity(enter_trace->m_pEnt)))
			{
				exit_trace = enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;
}


float PLegitbot::LegitHandleBulletPenetration(surfacedata_t* enterSurfaceData, trace_t& enterTrace, const Vector& direction, Vector& result, float penetration, float damage)
{
    trace_t exitTrace;

    if (!PTraceToExit(&enterTrace, enterTrace.endpos, direction, &exitTrace))
        return -1.0f;

	surfacedata_t* exitSurfaceData = interfaces.phys_props->GetSurfaceData(exitTrace.surface.surfaceProps);

    float damageModifier = 0.16f;
    float penetrationModifier = (enterSurfaceData->game.flPenetrationModifier + exitSurfaceData->game.flPenetrationModifier) / 2.0f;

    if (enterSurfaceData->game.material == 71 || enterSurfaceData->game.material == 89) {
        damageModifier = 0.05f;
        penetrationModifier = 3.0f;
    }
    else if (enterTrace.contents >> 3 & 1 || enterTrace.surface.flags >> 7 & 1) {
        penetrationModifier = 1.0f;
    }

    if (enterSurfaceData->game.material == exitSurfaceData->game.material) {
        if (exitSurfaceData->game.material == 85 || exitSurfaceData->game.material == 87)
            penetrationModifier = 3.0f;
        else if (exitSurfaceData->game.material == 76)
            penetrationModifier = 2.0f;
    }

    damage -= 11.25f / penetration / penetrationModifier + damage * damageModifier + (exitTrace.endpos - enterTrace.endpos).LengthSqr() / 24.0f / penetrationModifier;

    result = exitTrace.endpos;
    return damage;
}

void PLegitbot::DScaleDamage(IBasePlayer* e, CCSWeaponInfo* weapon_info, int hitgroup, float& current_damage)
{
	static auto mp_damage_scale_ct_head = interfaces.cvars->FindVar(hs::mp_damage_scale_ct_head::s().c_str());
	static auto mp_damage_scale_t_head = interfaces.cvars->FindVar(hs::mp_damage_scale_t_head::s().c_str());
	static auto mp_damage_scale_ct_body = interfaces.cvars->FindVar(hs::mp_damage_scale_ct_body::s().c_str());
	static auto mp_damage_scale_t_body = interfaces.cvars->FindVar(hs::mp_damage_scale_t_body::s().c_str());

	auto team = e->GetTeam();
	auto head_scale = team == 2 ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();
	auto body_scale = team == 2 ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();

	auto armor_heavy = e->HeavyArmor();
	auto armor_value = static_cast<float>(e->GetArmor());

	if (armor_heavy) head_scale *= 0.5f;

	// ref: CCSPlayer::TraceAttack

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		current_damage = (current_damage * 4.f) * head_scale;
		break;
	case HITGROUP_CHEST:
	case 8:
		current_damage *= body_scale;
		break;
	case HITGROUP_STOMACH:
		current_damage = (current_damage * 1.45f) * body_scale;
		break;
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		current_damage *= body_scale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage = (current_damage * 0.75f) * body_scale;
		break;
	default:
		break;
	}

	static auto IsArmored = [](IBasePlayer* player, int hitgroup)
	{
		auto has_helmet = player->HasHelmet();
		auto armor_value = static_cast<float>(player->GetArmor());

		if (armor_value > 0.f)
		{
			switch (hitgroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case 8:
				return true;
				break;
			case HITGROUP_HEAD:
				return has_helmet || (bool)player->HeavyArmor();
				break;
			default:
				return (bool)player->HeavyArmor();
				break;
			}
		}

		return false;
	};

	if (IsArmored(e, hitgroup))
	{
		auto armor_scale = 1.f;
		auto armor_ratio = (weapon_info->m_flArmorRatio * 0.5f);
		auto armor_bonus_ratio = 0.5f;

		if (armor_heavy)
		{
			armor_ratio *= 0.2f;
			armor_bonus_ratio = 0.33f;
			armor_scale = 0.25f;
		}

		float new_damage = current_damage * armor_ratio;
		float estiminated_damage = (current_damage - (current_damage * armor_ratio)) * (armor_scale * armor_bonus_ratio);
		if (estiminated_damage > armor_value) new_damage = (current_damage - (armor_value / armor_bonus_ratio));

		current_damage = new_damage;
	}
}

float PLegitbot::GetMultiplier(int hitGroup) 
{
	switch (hitGroup) {
	case HITGROUP_HEAD:
		return 4.0f;
	case HITGROUP_STOMACH:
		return 1.25f;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return 0.75f;
	default:
		return 1.0f;
	}
}

bool PLegitbot::IsPropArmour(int H, bool G) 
{
	switch (H) {
	case HITGROUP_HEAD:
		return G;

	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		return true;
	default:
		return false;
	}
}

bool PLegitbot::CanScanEntity(IBasePlayer* entity, const Vector& destination, int minDamage) {
	if (!csgo->local || !entity->isAlive())
		return false;

	auto WeapInfo = csgo->local->GetWeapon()->GetCSWpnData();

	float damage = WeapInfo->m_iDamage;

	Vector start = csgo->local->GetEyePosition();
	Vector direction = destination - start;
	direction /= direction.Length(); 

	int hitsLeft = 4;

	while (damage >= 1.0f && hitsLeft) {
		trace_t trace;
		Ray_t T;
		T.init(start, destination);
		CTraceFilter f;
		f.pSkip = csgo->local;
		interfaces.trace->TraceRay(T, 0x4600400B, &f, &trace);

		if (trace.m_pEnt && trace.m_pEnt->IsPlayer() && trace.m_pEnt->GetTeam() != csgo->local->GetTeam())
			return false;

		if (trace.fraction == 1.0f)
			break;

		if (trace.m_pEnt == entity && trace.hitgroup > HITGROUP_GENERIC && trace.hitgroup <= HITGROUP_RIGHTLEG) {
			damage = GetMultiplier(trace.hitgroup) * damage * pow(WeapInfo->m_flRangeModifier, trace.fraction * WeapInfo->m_flRange / 500.0f);

			if (float armorRatio{ WeapInfo->m_flArmorRatio / 2.0f }; IsPropArmour(trace.hitgroup, trace.m_pEnt->HasHelmet()))
				damage -= (trace.m_pEnt->GetArmor() < damage * armorRatio / 2.0f ? trace.m_pEnt->GetArmor() * 4.0f : damage) * (1.0f - armorRatio);

			return damage >= minDamage;
		}
		const auto surfaceData = interfaces.phys_props->GetSurfaceData(trace.surface.surfaceProps);

		if (surfaceData->game.flPenetrationModifier < 0.1f)
			break;

		damage = LegitHandleBulletPenetration(surfaceData, trace, direction, start, WeapInfo->m_flPenetration, damage);
		hitsLeft--;
	}
	return false;
}

PLegitConfigInfo PLegitbot::GetConfig() {
	PLegitConfigInfo d;
	return d;
}

bool PLegitbot::NeedRecoilControl(IBaseCombatWeapon* weapon) {
	return !(weapon->isSniper() || weapon->isPistol() || weapon->isShotgun() || weapon->IsHeavyPistol() || weapon->IsZeus() || weapon->IsKnife() || weapon->IsNade());
}

Vector PLegitbot::GetPoint(IBasePlayer* pBaseEntity, int iHitbox, matrix BoneMatrix[128])
{
	std::vector<Vector> vPoints;

	if (!pBaseEntity)
		return Vector(0, 0, 0);

	studiohdr_t* pStudioModel = interfaces.models.model_info->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(0);

	if (!set)
		return Vector(0, 0, 0);

	mstudiobbox_t* untransformedBox = set->pHitbox(iHitbox);
	if (!untransformedBox)
		return Vector(0, 0, 0);

	Vector vecMin = { 0, 0, 0 };
	Math::VectorTransform_Wrapper(untransformedBox->bbmin, BoneMatrix[untransformedBox->bone], vecMin);

	Vector vecMax = { 0, 0, 0 };
	Math::VectorTransform_Wrapper(untransformedBox->bbmax, BoneMatrix[untransformedBox->bone], vecMax);

	float mod = untransformedBox->radius != -1.f ? untransformedBox->radius : 0.f;
	Vector max;
	Vector min;

	Math::VectorTransform(untransformedBox->bbmax + mod, BoneMatrix[untransformedBox->bone], max);
	Math::VectorTransform(untransformedBox->bbmin - mod, BoneMatrix[untransformedBox->bone], min);

	return (min + max) * 0.5f;
}

Vector PLegitbot::GetHitboxPos(IBasePlayer* ent, int ID) {
	static matrix BoneMatrix[128];
	static animation* Anim;
	if (g_Animfix->get_latest_animation(ent).has_value()) {
		Anim = g_Animfix->get_latest_animation(ent).value();
	}
	if (Anim != nullptr) {
		memcpy(BoneMatrix, Anim->bones, sizeof(matrix[128]));
		return GetPoint(Anim->player, ID, BoneMatrix);
	}
	return Vector(0, 0, 0);
}

float rad2deg(float radians) {
	return radians * (180.0f / PI); 
}

float deg2rad(float deg) { return deg * (PI / 180.0f); }

Vector toang(Vector v) {
	return Vector( rad2deg(std::atan2(-v.z, std::hypot(v.x, v.y))),
					   rad2deg(std::atan2(v.y, v.x)),
					   0.0f );
}

Vector PLegitbot::calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles)
{

	return (toang(destination - source) - viewAngles).Normalize();
}
bool PLegitbot::IsVisible(IBasePlayer* ent, int ID) {
	trace_t t;
	TraceLineNonMemoryIntensive(csgo->local->GetEyePosition(), ent->GetEyePosition(), MASK_SOLID, csgo->local, &t);
	return t.fraction == 1.f;
}
void PLegitbot::Run(CUserCmd* cmd)
{
	if (!csgo->local || !csgo->local->isAlive())
		return;
	auto time = TICKS_TO_TIME(csgo->local->GetTickBase() - csgo->m_nTickbaseShift);

	if (csgo->local->m_flNextAttack() > time)
		return;

	const auto activeWeapon = csgo->local->GetWeapon();
	if (!activeWeapon || !activeWeapon->GetAmmo(false) || activeWeapon->InReload())
		return;

	if (csgo->local->GetShootsFired() > 0 && !activeWeapon->GetCSWpnData()->m_nFullAuto)
		return;

	auto weaponIndex = activeWeapon->GetItemDefinitionIndex();
	auto config = GetConfig();
	if (!weaponIndex)
		return;

	if (!config.Enabled)
		return;

	if (!config.BShots && activeWeapon->NextPrimaryAttack() >= time)
		return;

	if (!config.IgnoreFlash && csgo->local->IsFlashed())
		return;

	if (config.Bind && !config.KeyActive)
		return;


	if ((cmd->buttons & IN_ATTACK || config.AutoFire || config.LockOnType == 1) && activeWeapon->GetInaccuracy() <= config.MaxInnacuracy) {

		if (config.ScopedOnly && activeWeapon->isSniper() && !csgo->local->IsScoped())
			return;

		auto bestFov = config.FOV;
		Vector bestTarget{ };
		const auto localPlayerEyePosition = csgo->local->GetEyePosition();

		const auto aimPunch = NeedRecoilControl(activeWeapon) ? csgo->local->GetPunchAngle() : Vector{ };
	
		for (int i = 1; i <= interfaces.engine->GetMaxClients(); i++) {
			auto entity = interfaces.ent_list->GetClientEntity(i);
			if (!entity || entity == csgo->local || entity->IsDormant() || !entity->isAlive()
				|| entity->GetTeam() != csgo->local->GetTeam() || entity->HasGunGameImmunity())
				continue;

			for (auto bone : { 8, 4, 3, 7, 6, 5 }) {
				const auto bonePosition = GetHitboxPos(entity,config.bone > 1 ? 10 - config.bone : bone);
				const auto angle = calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch);

				const auto fov = std::hypot(angle.x, angle.y);
				if (fov > bestFov)
					continue;


				if (!IsVisible(entity, bonePosition) && (config->aimbot[weaponIndex].visibleOnly || !canScan(entity, bonePosition, activeWeapon->getWeaponData(), config->aimbot[weaponIndex].killshot ? entity->health() : config->aimbot[weaponIndex].minDamage, config->aimbot[weaponIndex].friendlyFire)))
					continue;

				if (fov < bestFov) {
					bestFov = fov;
					bestTarget = bonePosition;
				}
				if (config->aimbot[weaponIndex].bone)
					break;
			}
		}

		if (bestTarget.notNull()) {
			static Vector lastAngles{ cmd->viewangles };
			static int lastCommand{ };

			if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull() && config->aimbot[weaponIndex].silent)
				cmd->viewangles = lastAngles;

			auto angle = calculateRelativeAngle(localPlayerEyePosition, bestTarget, cmd->viewangles + aimPunch);
			bool clamped{ false };

			if (std::abs(angle.x) > Misc::maxAngleDelta() || std::abs(angle.y) > Misc::maxAngleDelta()) {
				angle.x = std::clamp(angle.x, -Misc::maxAngleDelta(), Misc::maxAngleDelta());
				angle.y = std::clamp(angle.y, -Misc::maxAngleDelta(), Misc::maxAngleDelta());
				clamped = true;
			}

			angle /= config->aimbot[weaponIndex].smooth;
			cmd->viewangles += angle;
			if (!config->aimbot[weaponIndex].silent)
				interfaces->engine->setViewAngles(cmd->viewangles);

			if (config->aimbot[weaponIndex].autoScope && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
				cmd->buttons |= UserCmd::IN_ATTACK2;

			if (config->aimbot[weaponIndex].autoShot && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && !clamped && activeWeapon->getInaccuracy() <= config->aimbot[weaponIndex].maxShotInaccuracy)
				cmd->buttons |= UserCmd::IN_ATTACK;

			if (clamped)
				cmd->buttons &= ~UserCmd::IN_ATTACK;

			if (clamped || config->aimbot[weaponIndex].smooth > 1.0f) lastAngles = cmd->viewangles;
			else lastAngles = Vector{ };

			lastCommand = cmd->commandNumber;
		}
	}
}