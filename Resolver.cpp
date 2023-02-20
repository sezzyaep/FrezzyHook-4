#include "Hooks.h"
#include "Resolver.h"
#include "RageBacktracking.h"
#include "Ragebot.h"
#include "AnimationFix.h"

std::string ResolverMode[65];
int last_ticks[65];
int IBasePlayer::GetChokedPackets() {
	auto ticks = TIME_TO_TICKS(GetSimulationTime() - GetOldSimulationTime());
	if (ticks == 0 && last_ticks[GetIndex()] > 0) {
		return last_ticks[GetIndex()] - 1;
	}
	else {
		last_ticks[GetIndex()] = ticks;
		return ticks;
	}
}

float CResolver::GetAngle(IBasePlayer* player) {
	return Math::NormalizeYaw(player->GetEyeAngles().y);
}

float CResolver::GetForwardYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(GetBackwardYaw(player) - 180.f);
}

float CResolver::GetBackwardYaw(IBasePlayer* player) {
	return Math::CalculateAngle(player->GetOrigin(), csgo->local->GetOrigin()).y;
}

float CResolver::GetLeftYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y - 90.f);
}

float CResolver::GetRightYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y + 90.f);
}
bool CResolver::TargetJitter(IBasePlayer* player, bool v2) {
	float yaw = v2 ? GetRightYaw(player) : GetLeftYaw(player);
	return fabsf(GetAngle(player) - Math::NormalizeYaw(yaw + 90.f))
		>= fabsf(GetAngle(player) - Math::NormalizeYaw(yaw - 90.f));
}
bool CResolver::TargetSide(IBasePlayer* player) {
	float yaw = Math::NormalizeYaw(GetBackwardYaw(player));
	float angle = GetAngle(player);
	return fabsf(angle - Math::NormalizeYaw(yaw + 90.f))
		>= fabsf(angle - Math::NormalizeYaw(yaw - 90.f));
}

float CResolver::max_desync_delta(IBasePlayer* player)
{
	uintptr_t animstate = uintptr_t(player->GetPlayerAnimState());


	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = std::fmax(0, std::fmin(*reinterpret_cast<float*>(animstate + 0xF8), 1));

	float speedfactor = std::fmax(0, std::fmin(1, *reinterpret_cast<float*> (animstate + 0xFC)));

	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.1f;
	float unk3;

	if (duckammount > 0) {

		unk2 += ((duckammount * speedfactor) * (0.5 - unk2));

	}
	else
		unk2 += ((duckammount * speedfactor) * (0.5 - 0.58));

	unk3 = *(float*)(animstate + 0x334) * unk2;
	return unk3;
}

void CResolver::DetectSide(IBasePlayer* player)
{
	Vector src3D, dst3D, forward, right, up, src, dst;
	float back_two, right_two, left_two;
	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;

	Math::AngleVectors(Vector(0, GetBackwardYaw(player), 0), &forward, &right, &up);

	filter.pSkip = player;
	src3D = player->GetEyePosition();
	dst3D = src3D + (forward * 384); //Might want to experiment with other numbers, incase you don't know what the number does, its how far the trace will go. Lower = shorter.

	ray.Init(src3D, dst3D);
	interfaces.trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	back_two = (tr.endpos - tr.startpos).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);
	interfaces.trace->TraceRay(ray2, MASK_SHOT, &filter, &tr);
	right_two = (tr.endpos - tr.startpos).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);
	interfaces.trace->TraceRay(ray3, MASK_SHOT, &filter, &tr);
	left_two = (tr.endpos - tr.startpos).Length();

	if (left_two > right_two) {
		side[player->EntIndex()] = -1;
		//Body should be right
	}
	else if (right_two > left_two) {
		side[player->EntIndex()] = 1;
	}
	else
		side[player->EntIndex()] = 0;
}

bool CResolver::DoesHaveJitter(IBasePlayer* player) {
	static float LastAngle[64];
	static int LastBrute[64];
	static bool Switch[64];
	static float LastUpdateTime[64];

	int i = player->GetIndex();

	float CurrentAngle = player->GetEyeAngles().y;
	if (!Math::IsNearEqual(CurrentAngle, LastAngle[i], (max_desync_delta(player) - 4.f))) {
		Switch[i] = !Switch[i];
		LastAngle[i] = CurrentAngle;
		LastUpdateTime[i] = interfaces.global_vars->curtime;
		return true;
	}
	else {
		if (fabsf(LastUpdateTime[i] - interfaces.global_vars->curtime >= TICKS_TO_TIME(15))
			|| player->GetSimulationTime() != player->GetOldSimulationTime()) {
			LastAngle[i] = CurrentAngle;
		}
	}
	return false;
}

float Resolve(IBasePlayer* player, float EyeAngle, int Side) {
	static auto GetSmoothedVelocity = [](float min_delta, Vector a, Vector b) {
		Vector delta = a - b;
		float L = delta.Length();

		if (L <= min_delta)
		{


			if (-min_delta <= L)
				return a;
			else
			{
		
				return b - ((delta * (1.0f / (L + 1.192092896e-07F))) * min_delta);
			}
		}
		else
		{
			return b + ((delta * (1.0f / (L + 1.192092896e-07F))) * min_delta);
		}
	};

	auto animstate = player->GetPlayerAnimState();

	if (!animstate)
		return 0;

	float yaw = player->GetEyeAngles().y;
	float predicteddesync = fabs(yaw - player->GetLBY());

	float PredictedEyeYaw = yaw + predicteddesync * Side;

	Vector velocity = player->GetVelocity();

	if (velocity.LengthSqr() > 97344.f)
	{
		velocity = velocity.Normalized() * 312.f;
	}

	float chokedpackets = *(float*)((uintptr_t)animstate + 0x0074);
	float crouchfix = std::clamp(player->GetDuckAmount() + *(float*)((uintptr_t)animstate + 0x00A8), 0.0f, 1.0f);
	float actualcrouch = *(float*)((uintptr_t)animstate + 0x00A4);
	float maxchange = chokedpackets * 6.0f;
	float finalcrouch;

	if ((crouchfix - actualcrouch) <= maxchange) {
		if (-maxchange <= (crouchfix - actualcrouch))
			finalcrouch = crouchfix;
		else
			finalcrouch = actualcrouch - maxchange;
	}
	else {
		finalcrouch = actualcrouch + maxchange;
	}

	float duck = std::clamp(finalcrouch, 0.0f, 1.0f);

	Vector animationmovement = GetSmoothedVelocity(chokedpackets * 2000.0f, velocity, player->GetVelocity());
//	float velocity2 = std::fminf(animationmovement.Length(), 260.0f);
	float velocity2 = animationmovement.Length();
	static float maxmovespeed = 260.0f; //this ching chong must be removed

	IBaseCombatWeapon* gun = player->GetWeapon();

	if (gun && gun->GetCSWpnData())
		maxmovespeed = std::fmaxf(gun->GetCSWpnData()->m_flMaxSpeedAlt, 0.001f);

	float actualrun = velocity2 / (maxmovespeed * 0.520f);
	float actualducksp = velocity2 / (maxmovespeed * 0.340f);

	actualrun = std::clamp(actualrun, 0.0f, 1.0f);

	float maxdesync = (((*(float*)((uintptr_t)animstate + 0x011C) * -0.30000001) - 0.19999999) * actualrun) + 1.0f;

	if (duck > 0.0f)
	{
		float actualducksp = std::clamp(actualducksp, 0.0f, 1.0f);
		maxdesync += (duck * actualducksp) * (0.5f - maxdesync);
	}

	float mi = min(*(float*)((uintptr_t)animstate + 0x0330),-58.f);
	float ma = max(*(float*)((uintptr_t)animstate + 0x0334),58.f);

	float minyaw = mi * maxdesync;
	float maxyaw = ma * maxdesync;
	float resolvedgoalfeetyaw = 0;
	if (PredictedEyeYaw <= maxyaw)
	{
		if (minyaw > predicteddesync)
			resolvedgoalfeetyaw = fabs(minyaw) + yaw;
	}
	else
	{
		resolvedgoalfeetyaw = yaw - fabs(maxyaw);
	}

	Math::NormalizeYaw(resolvedgoalfeetyaw);

	if (velocity2 > 0.1f || fabs(velocity.z) > 100.0f)
	{
		resolvedgoalfeetyaw = Math::ApproachAngle(
			player->GetLowerBodyYaw(),
			resolvedgoalfeetyaw,
			*(float*)((uintptr_t)animstate + 0x011C) * chokedpackets);
	}
	else
	{
	
		resolvedgoalfeetyaw = Math::ApproachAngle(
			player->GetLowerBodyYaw(),
			resolvedgoalfeetyaw,
			chokedpackets * 100.0f);
	}

	return resolvedgoalfeetyaw;
}

float build_server_abs_yaw2(IBasePlayer* m_player, float angle) //ty gamesense
{
	Vector velocity = m_player->GetVelocity();
	auto anim_state = m_player->GetPlayerAnimState();
	float m_flEyeYaw = angle;
	float resolvedgoalfeetyaw = 0.f;

	float eye_feet_delta = Math::AngleDiff(m_flEyeYaw, resolvedgoalfeetyaw);

	static auto GetSmoothedVelocity = [](float min_delta, Vector a, Vector b) {
		Vector delta = a - b;
		float delta_length = delta.Length();

		if (delta_length <= min_delta)
		{
			Vector result;

			if (-min_delta <= delta_length)
				return a;
			else
			{
				float iradius = 1.0f / (delta_length + FLT_EPSILON);
				return b - ((delta * iradius) * min_delta);
			}
		}
		else
		{
			float iradius = 1.0f / (delta_length + FLT_EPSILON);
			return b + ((delta * iradius) * min_delta);
		}
	};

	float spd = velocity.LengthSqr();

	if (spd > std::powf(1.2f * 260.0f, 2.f))
	{
		Vector velocity_normalized = velocity.Normalized();
		velocity = velocity_normalized * (1.2f * 260.0f);
	}

	float m_flChokedTime = anim_state->m_last_update_increment;
	float crouchfix = std::clamp(m_player->GetDuckAmount() + anim_state->m_duck_additional, 0.0f, 1.0f);
	float actualcrouch = anim_state->m_anim_duck_amount;
	float maxchange = m_flChokedTime * 6.0f;
	float finalcrouch;

	// clamp
	if ((crouchfix - actualcrouch) <= maxchange) {
		if (-maxchange <= (crouchfix - actualcrouch))
			finalcrouch = crouchfix;
		else
			finalcrouch = actualcrouch - maxchange;
	}
	else {
		finalcrouch = actualcrouch + maxchange;
	}

	float duck = std::clamp(finalcrouch, 0.0f, 1.0f);

	Vector animationmovement = GetSmoothedVelocity(m_flChokedTime * 2000.0f, velocity, m_player->GetVelocity());
	float velocity2 = std::fminf(animationmovement.Length(), 260.0f);

	float maxmovespeed = 260.0f;

	IBaseCombatWeapon* gun = m_player->GetWeapon();

	if (gun && gun->GetCSWpnData())
		maxmovespeed = std::fmaxf(gun->GetCSWpnData()->m_flMaxSpeedAlt, 0.001f);

	float actualrun = velocity2 / (maxmovespeed * 0.520f);
	float actualducksp = velocity2 / (maxmovespeed * 0.340f);

	actualrun = std::clamp(actualrun, 0.0f, 1.0f);

	float maxdesync = (((anim_state->ground_fraction * -0.30000001) - 0.19999999) * actualrun) + 1.0f;

	if (duck > 0.0f)
	{
		float actualducksp = std::clamp(actualducksp, 0.0f, 1.0f);
		maxdesync += (duck * actualducksp) * (0.5f - maxdesync);
	}

	const float v60 = -58.f;
	const float v61 = 58.f;

	float minyaw = v60 * maxdesync;
	float maxyaw = v61 * maxdesync;

	if (eye_feet_delta <= maxyaw)
	{
		if (minyaw > eye_feet_delta)
			resolvedgoalfeetyaw = fabs(minyaw) + m_flEyeYaw;
	}
	else
	{
		resolvedgoalfeetyaw = m_flEyeYaw - fabs(maxyaw);
	}

	Math::NormalizeYaw(resolvedgoalfeetyaw);

	if (velocity2 > 0.1f || fabs(velocity.z) > 100.0f)
	{
		resolvedgoalfeetyaw = Math::ApproachAngle(
			m_flEyeYaw,
			resolvedgoalfeetyaw,
			((anim_state->ground_fraction * 20.0f) + 30.0f)
			* m_flChokedTime);
	}
	else
	{
		resolvedgoalfeetyaw = Math::ApproachAngle(
			m_player->GetLowerBodyYaw(),
			resolvedgoalfeetyaw,
			m_flChokedTime * 100.0f);
	}

	return resolvedgoalfeetyaw;
}

void CResolver::StoreAntifreestand()
{
	if (!csgo->local->isAlive())
		return;

	if (!csgo->weapon->IsGun())
		return;

	for (int i = 1; i < interfaces.engine->GetMaxClients(); ++i)
	{
		auto player = interfaces.ent_list->GetClientEntity(i);
		if (!player || !player->isAlive() || player->IsDormant() || player->GetTeam() == csgo->local->GetTeam()) {
			continue;
		}
		bool Autowalled = false, HitSide1 = false, HitSide2 = false;
		auto idx = player->EntIndex();
		UseFreestand[idx] = false;
		float angToLocal = Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
		Vector ViewPoint = csgo->local->GetOrigin() + Vector(0, 0, 90);
		Vector2D Side1 = { (45 * sin(DEG2RAD(angToLocal))),(45 * cos(DEG2RAD(angToLocal))) };
		Vector2D Side2 = { (45 * sin(DEG2RAD(angToLocal + 180))) ,(45 * cos(DEG2RAD(angToLocal + 180))) };

		Vector2D Side3 = { (50 * sin(DEG2RAD(angToLocal))),(50 * cos(DEG2RAD(angToLocal))) };
		Vector2D Side4 = { (50 * sin(DEG2RAD(angToLocal + 180))) ,(50 * cos(DEG2RAD(angToLocal + 180))) };

		Vector Origin = player->GetOrigin();

		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };
		for (int side = 0; side < 2; side++)
		{
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 90 };
			Vector ViewPointAutowall = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

			if (g_AutoWall.CanHitFloatingPoint(OriginAutowall, ViewPoint))
			{
				if (side == 0)
				{
					UseFreestand[idx] = true;
					FreestandSide[idx] = 1;
				}
				else if (side == 1)
				{
					UseFreestand[idx] = true;
					FreestandSide[idx] = -1;
				}
			}
			else
			{
				for (int sidealternative = 0; sidealternative < 2; sidealternative++)
				{
					Vector ViewPointAutowallalternative = { Origin.x + OriginLeftRight[sidealternative].x,  Origin.y - OriginLeftRight[sidealternative].y , Origin.z + 90 };

					if (g_AutoWall.CanHitFloatingPoint(ViewPointAutowallalternative, ViewPointAutowall))
					{
						if (sidealternative == 0)
						{
							UseFreestand[idx] = true;
							FreestandSide[idx] = 1;
						}
						else if (sidealternative == 1)
						{
							UseFreestand[idx] = true;
							FreestandSide[idx] = -1;
						}
					}
				}
			}
		}
	}
}

/*
void CResolver::StoreAntifreestand()
{
	if (!csgo->local->isAlive())
		return;

	if (!csgo->weapon->IsGun())
		return;

	for (int i = 1; i < interfaces.engine->GetMaxClients(); ++i)
	{
		auto player = interfaces.ent_list->GetClientEntity(i);
		if (!player || !player->isAlive() || player->IsDormant() || player->GetTeam() == csgo->local->GetTeam()) {
			continue;
		}
		bool Autowalled = false, HitSide1 = false, HitSide2 = false;
		auto idx = player->EntIndex();
		UseFreestand[idx] = false;
		float angToLocal = Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
		Vector ViewPoint = csgo->local->GetOrigin() + Vector(0, 0, 90);
		Vector2D Side1 = { (45 * sin(DEG2RAD(angToLocal))),(45 * cos(DEG2RAD(angToLocal))) };
		Vector2D Side2 = { (45 * sin(DEG2RAD(angToLocal + 180))) ,(45 * cos(DEG2RAD(angToLocal + 180))) };

		Vector2D Side3 = { (50 * sin(DEG2RAD(angToLocal))),(50 * cos(DEG2RAD(angToLocal))) };
		Vector2D Side4 = { (50 * sin(DEG2RAD(angToLocal + 180))) ,(50 * cos(DEG2RAD(angToLocal + 180))) };

		Vector Origin = player->GetOrigin();

		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };
		for (int side = 0; side < 2; side++)
		{
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 90 };
			Vector ViewPointAutowall = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

			if (g_AutoWall.CanHitFloatingPoint(OriginAutowall, ViewPoint))
			{
				if (side == 0)
				{
					UseFreestand[idx] = true;
					FreestandSide[idx] = 1;
				}
				else if (side == 1)
				{
					UseFreestand[idx] = true;
					FreestandSide[idx] = -1;
				}
			}
			else
			{
				for (int sidealternative = 0; sidealternative < 2; sidealternative++)
				{
					Vector ViewPointAutowallalternative = { Origin.x + OriginLeftRight[sidealternative].x,  Origin.y - OriginLeftRight[sidealternative].y , Origin.z + 90 };

					if (g_AutoWall.CanHitFloatingPoint(ViewPointAutowallalternative, ViewPointAutowall))
					{
						if (sidealternative == 0)
						{
							UseFreestand[idx] = true;
							FreestandSide[idx] = 1;
						}
						else if (sidealternative == 1)
						{
							UseFreestand[idx] = true;
							FreestandSide[idx] = -1;
						}
					}
				}
			}
		}
	}
}
*/ //antifreestand temporarily removed

#define CalculateDelta(angle1, angle2) remainderf(fabsf(angle1 - angle2), 360.0f)
#define NY(yaw) Math::NormalizeYaw(fabsf(yaw))
#define CA(angle1, angle2) Math::CalculateAngle(angle1, angle2)

void CResolver::Do(IBasePlayer* player) {
	if (vars.ragebot.resolver == 2)
		return;

	if (!csgo->local)
		return;
	if (!player->IsValid())
		return;
	if (!csgo->local->isAlive())
		return;
	if (player == csgo->local)
		return;
	//if (player->GetChokedPackets() <= 0)
		//return;
	int index = player->GetIndex();
	if (player->GetPlayerInfo().fakeplayer)
		return;
	if (!player->isAlive() || player->GetTeam() == csgo->local->GetTeam())
		return;
	auto animstate = player->GetPlayerAnimState();
	if (!animstate)
		return;
	static auto GetSide = [](IBasePlayer* ent) -> int {
		
		auto EyeYaw = ent->GetEyeAngles().y;
		auto FeetYaw = ent->GetLBY();
		auto Left = Math::NormalizeYaw(EyeYaw - 60);
		auto Right = Math::NormalizeYaw(EyeYaw + 60);
		Left -= FeetYaw;
		Left = fabs(Left);
		Right -= FeetYaw;
		Right = fabs(Right);

		if (Left <= Right) {
			return -1;
		}
		else {
			return 1;
		}


	};
	int Side = -1;//GetSide(player);
	//if ((csgo->missedshots[index] % 2) == 1)
		//Side = 1;

	animstate->m_fGoalFeetYaw = Resolve(player, player->GetEyeAngles().y,Side);
	player->GetLBY() = animstate->m_fGoalFeetYaw;
	auto anim = g_Animfix->get_latest_animation(player);
	if (anim.has_value()) {
		anim.value()->lby = animstate->m_fGoalFeetYaw;
		if (anim.value()->anim_state)
			anim.value()->anim_state->m_fGoalFeetYaw = animstate->m_fGoalFeetYaw;
	}
	/*
	if (vars.keybind.safepoint->active)
	{
		ResolverMode[index] = "Safepoint";
		animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + (std::clamp(max_desync_delta(player) / 2, 0.f, 58.f) * side[index]));
	}
	else if (vars.ragebot.resolver == 1) {


		int idx = player->GetIndex();
		float yourmomeye = player->GetEyeAngles().y;
		float oppositelbycheck = yourmomeye - player->GetLowerBodyYaw();
		bool PitchZero = Math::NormalizePitch(player->GetEyeAngles().x) <= 45.f;
		bool is_tside = player->GetTeam() == 2;
		float deltadifference = yourmomeye - animstate->m_fGoalFeetYaw;
		bool slowwalking = animstate->m_feet_yaw_rate > 0.01f && animstate->m_feet_yaw_rate < 0.8f && player->GetVelocity().Length2D() < 110.f && player->GetVelocity().Length2D() > 10.f;
		float angToLocal = Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
		float predicted_desync[65];
		bool running = animstate->m_feet_yaw_rate > 0.8f && player->GetVelocity().Length2D() >= 110.f;
		float maxdelta = (deltadifference / (*(float*)((DWORD)animstate + 0x334))) * 58.f;
		float mindelta = (deltadifference / (*(float*)((DWORD)animstate + 0x330))) * -58.f;
		auto animLayer = player->GetAnimOverlays();
		int act = player->GetSequenceActivity(animLayer->m_sequence);
		int layer3delta = player->GetSequenceActivity(animLayer[3].m_sequence);
		float forwardlegacy = 180;


		if (layer3delta == 979 && player->GetVelocity().Length2D() <= 0.f)
		{
			ResolverMode[idx] = "Extended Desync";
			float eyegoalfeetdelta = Math::AngleDiff(player->GetEyeAngles().y - animstate->m_fGoalFeetYaw, 360.f);

			float new_body_yaw_pose = 0.0f;

			if (eyegoalfeetdelta < 0.0f)
			{
				new_body_yaw_pose = (eyegoalfeetdelta / (*(float*)((DWORD)animstate + 0x330))) * -58.f;
			}
			else
			{
				new_body_yaw_pose = (eyegoalfeetdelta / (*(float*)((DWORD)animstate + 0x334))) * 58.f;
			}

			if (new_body_yaw_pose > 10.f)
				predicted_desync[idx] = new_body_yaw_pose;
			else if (new_body_yaw_pose < -10.f)
				predicted_desync[idx] = -new_body_yaw_pose;
		}


		if (running || !(player->GetFlags() & FL_ONGROUND)) {

			predicted_desync[idx] = 14.f;
		}

		if (slowwalking && player->GetDuckAmount() == 0) {
			predicted_desync[idx] = 28.f;
		}

		if (is_tside && player->GetDuckAmount() > 0) {

			if (player->GetWeapon()->IsBomb() || player->GetWeapon()->IsKnife() || player->GetWeapon()->IsZeus() || player->GetWeapon()->isPistol()) {

				predicted_desync[idx] = 35.f;
			}
			else {
				predicted_desync[idx] = 15.f;
			}
		}

		if (PitchZero) {
			ResolverMode[idx] = "Legit AntiAim";
			switch (csgo->missedshots[idx] % 3) {
			case 0:
				animstate->m_fGoalFeetYaw = GetAngle(player);
				break;
			case 1:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) + 58.f * FreestandSide[idx]);
				break;
			case 2:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) - 58.f * FreestandSide[idx]);
				break;
			}
		}

		if (DoesHaveJitter(player) && player->GetVelocity().Length2D() <= 110.f && (player->GetFlags() & FL_ONGROUND)) {
			ResolverMode[idx] = "Jitter";
			switch (csgo->missedshots[idx] % 2) {
			case 0:
				animstate->m_fGoalFeetYaw = angToLocal + 90.f * FreestandSide[idx];
				break;
			case 1:
				animstate->m_fGoalFeetYaw = angToLocal - 90.f * FreestandSide[idx];
				break;
			}
		}
		else if (!DoesHaveJitter(player) && !PitchZero) {
			if (predicted_desync[idx] == 35.f || predicted_desync[idx] == 15.f)
				ResolverMode[idx] = "Crouched";
			if (predicted_desync[idx] == 13.f)
				ResolverMode[idx] = "Running";
			if (predicted_desync[idx] == 13.f && !(player->GetFlags() & FL_ONGROUND))
				ResolverMode[idx] = "In Air";
			if (predicted_desync[idx] == 28.f)
				ResolverMode[idx] = "Slowwalking";
			if (predicted_desync[idx] == 58.f)
				ResolverMode[idx] = "Normal";
			switch (csgo->missedshots[idx] % 5) {
			case 0:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) + predicted_desync[idx] * side[idx]);
				break;
			case 1:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) - 13.f * side[idx]);
				break;
			case 2:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) - predicted_desync[idx] * side[idx]);
				break;
			case 3:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) + 58.f);
				break;
			case 4:
				animstate->m_fGoalFeetYaw = Math::NormalizeYaw(GetAngle(player) - 58.f);
				break;
			}
		}
		animstate->m_fGoalFeetYaw = Math::NormalizeYaw(animstate->m_fGoalFeetYaw);
	}
	else if (vars.ragebot.resolver == 0)
	{

		static float lastbruteforce = 0;
		auto ex = player->GetAnimOverlay(3);
		bool ExtendedDesync = ex->m_cycle == 0.f && ex->m_weight == 0.f;
		bool Running = player->GetVelocity().Length2D() >= 95.f;  //fuck up any desync as long as they are running
		//bool SlowWalking = animstate->m_feet_yaw_rate > 0.01f && animstate->m_feet_yaw_rate < 0.8f && player->GetVelocity().Length2D() < 110.f && player->GetVelocity().Length2D() > 10.f; // added for later use
		if (ExtendedDesync) // ez resolve for extended desync dogs [includes micromove dogs like V4]
		{
			ResolverMode[index] = "Extended Desync";
			animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + (max_desync_delta(player) * side[index]));
		}
		else if (Running) //safepoint head cause ez overlap GG
		{
			ResolverMode[index] = "Prediction";
			animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + (std::clamp(max_desync_delta(player) / 2, 0.f, 58.f) * side[index]));
		}
		else // non extended desync [prob low delta] [safepoint baim cause why waste time lol xD] [Head safepoint from delta to not miss]
		{

			// goal feet yaw for low delta
			float EyeAngle = player->GetEyeAngles().y;
			float LowerBody = player->GetLowerBodyYaw();
			float PredictedDesyncDelta = CalculateDelta(EyeAngle, animstate->m_fGoalFeetYaw);
			PredictedDesyncDelta = std::clamp(PredictedDesyncDelta, -58.f, 58.f); // clamp

			float lYaw = NY(EyeAngle - 60.0);
			float rYaw = NY(EyeAngle + 60.0);

			float lLowDelta = NY(EyeAngle - 35.0);
			float rLowDelta = NY(EyeAngle + 35.0);
			auto target_yaw = Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
			auto target_left_direction = Math::NormalizeYaw(target_yaw - lYaw);
			auto target_right_direction = Math::NormalizeYaw(target_yaw - rYaw);

			float max_rotation = animstate->max_yaw;

			if (fabsf(PredictedDesyncDelta) < 35.f) {
			
				switch (csgo->missedshots[index] % 3) {
				case 0: {
					ResolverMode[index] = "DL";
					animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + (std::clamp(((58 - 28.f) / 58) * (max_desync_delta(player)), 0.f, 58.f)));
				} break;
				case 1: {
					ResolverMode[index] = "Low Delta";
					animstate->m_fGoalFeetYaw = lLowDelta;

				} break;
				case 2: {
					ResolverMode[index] = "Low Delta";
					animstate->m_fGoalFeetYaw = rLowDelta;

				} break;
				}
			}
			else
			{
				switch (csgo->missedshots[index] % 3) {
				case 0: {
					ResolverMode[index] = "Desync";
					animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + (((58 - 10) / 58) * max_desync_delta(player)));
				} break;
				case 1: {
					ResolverMode[index] = "Desync 2";
					animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + (((58 - 15) / 58) * max_desync_delta(player)));

				} break;
				case 2: {
					ResolverMode[index] = "Desync 3";
					animstate->m_fGoalFeetYaw = build_server_abs_yaw2(player, player->GetEyeAngles().y + max_rotation * side[index]);

				} break;
				}
			}



		}
	}
	*/
}