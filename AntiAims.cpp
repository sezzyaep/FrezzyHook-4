#include "Hooks.h"
#include "AntiAims.h"
#include "Ragebot.h"

bool JitterSwitch = false;
int FreestandAngle = 0;

int RandomIntBetweenCandF(int floor, int ceiling)
{
	int Difference = ceiling - floor + 1;
	return((rand() % Difference) + floor);
}

bool CanDT() {
	int idx = csgo->weapon->GetItemDefinitionIndex();
	return vars.ragebot.double_tap->active;
}

bool CanHS() {
	return vars.ragebot.hideshots->active;
}

int ClampIntT(int value, int min, int max) {
	int intermediate = value;
	if (value < min)
		intermediate = min;
	if (value > max)
		intermediate = max;

	return(intermediate);
}
int MaxInt(int Value, int Max) {
	if (Value > Max)
		return(Max);
	else
		return(Value);
}
void TraceLine(Vector start, Vector end, unsigned int mask, IBasePlayer* ignore, trace_t* trace)
{
	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = ignore;

	interfaces.trace->TraceRay(ray, mask, &filter, trace);
}


int UpdateDirection2(float e) {
	if (!csgo->local->isAlive())
		return -1337;
	IBasePlayer* target = nullptr;
	auto best_fov = FLT_MAX;
	Vector localviewangles;
	interfaces.engine->GetViewAngles(localviewangles);

	for (auto i = 1; i < interfaces.global_vars->maxClients; i++)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e->IsValid() || !e->GetWeapon())
			continue;




		auto fov = Math::GetFov(localviewangles, Math::CalculateAngle(csgo->local->GetEyePosition(), e->GetOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}





	if (!target)
		return -1337;

	float RelativeEAngle = Math::CalculateAngle(target->GetOrigin(), csgo->local->GetEyePosition()).y + 180;
	float LocalRelativeAngle = Math::CalculateAngle(csgo->local->GetEyePosition(), target->GetOrigin()).y + 180;

	//setup Vectors
	Vector ETraceL;
	Vector ETraceR;
	Vector LTraceL;
	Vector LTraceR;
	Vector PStart = Vector(target->GetOrigin().x, target->GetOrigin().y, target->GetOrigin().z + target->GetVecViewOffset().z / 2);
	Vector PStartL = Vector(csgo->local->GetOrigin().x, csgo->local->GetOrigin().y, csgo->local->GetEyePosition().z);
	//Setup traces
	Math::AngleVectors(Vector(0, RelativeEAngle - 90, 0), ETraceL);
	Math::AngleVectors(Vector(0, RelativeEAngle + 90, 0), ETraceR);

	Math::AngleVectors(Vector(0, LocalRelativeAngle - 90, 0), LTraceL);
	Math::AngleVectors(Vector(0, LocalRelativeAngle + 90, 0), LTraceR);

	//trace_t Left;
	//trace_t Right;
	//TraceLine(PStartL + LTraceL * 50, PStart + ETraceL * 50, MASK_SOLID, csgo->local, &Left);
	//TraceLine(PStartL + LTraceR * 50, PStart + ETraceR * 50, MASK_SOLID, csgo->local, &Right);
	bool Left;
	bool Right;
	Left = g_AutoWall.CanHitFloatingPoint(PStart, PStartL + LTraceL * e);
	Right = g_AutoWall.CanHitFloatingPoint(PStart, PStartL + LTraceR * e);
	if (Left == Right) { //both hit /  both not hit
		return -1337;
	}
	else if (Left && !Right) { //left hit rigth not hit
	//	if(g_AutoWall.CanHitFloatingPoint(PStart, PStartL + LTraceL * 23))
		//	return -1337;
		
		return 1;
	}
	else if (!Left && Right) { //left didnt hit but right hit
		//if (g_AutoWall.CanHitFloatingPoint(PStart, PStartL + LTraceR * 23))
			//return -1337;

		return -1;
	}
	return -1337;
}

bool IsPeeking() {
	if (!csgo->local->isAlive())
		return false;

	IBasePlayer* target = nullptr;
	auto best_fov = FLT_MAX;
	Vector localviewangles;
	interfaces.engine->GetViewAngles(localviewangles);

	for (auto i = 1; i < interfaces.global_vars->maxClients; i++)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e->IsValid() || !e->GetWeapon())
			continue;




		auto fov = Math::GetFov(localviewangles, Math::CalculateAngle(csgo->local->GetEyePosition(), e->GetOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}





	if (!target)
		return -1337;

	float RelativeEAngle = Math::CalculateAngle(target->GetOrigin(), csgo->local->GetEyePosition()).y + 180;
	float LocalRelativeAngle = Math::CalculateAngle(csgo->local->GetEyePosition(), target->GetOrigin()).y + 180;


	Vector PStart = Vector(target->GetOrigin().x, target->GetOrigin().y, target->GetOrigin().z + target->GetVecViewOffset().z / 2);
	Vector PStartL = Vector(csgo->local->GetOrigin().x, csgo->local->GetOrigin().y, csgo->local->GetEyePosition().z);

	PStartL += csgo->local->GetVelocity() * TICKS_TO_TIME(Ragebot::Get().GetTicksToStop());

	if (g_AutoWall.CanHitFloatingPoint(PStart, PStartL))
		return true;
	return false;
}

int UpdateDirection10() {
	if (!csgo->local->isAlive())
		return -1337;
	IBasePlayer* target = nullptr;
	auto best_fov = FLT_MAX;
	Vector localviewangles;
	interfaces.engine->GetViewAngles(localviewangles);

	for (auto i = 1; i < interfaces.global_vars->maxClients; i++)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e->IsValid() || !e->GetWeapon())
			continue;




		auto fov = Math::GetFov(localviewangles, Math::CalculateAngle(csgo->local->GetEyePosition(), e->GetOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}





	if (!target)
		return -1337;

	float RelativeEAngle = Math::CalculateAngle(target->GetOrigin(), csgo->local->GetEyePosition()).y + 180;
	float LocalRelativeAngle = Math::CalculateAngle(csgo->local->GetEyePosition(), target->GetOrigin()).y + 180;

	//setup Vectors
	Vector ETraceL;
	Vector ETraceR;
	Vector LTraceL;
	Vector LTraceR;
	Vector PStart = Vector(target->GetOrigin().x, target->GetOrigin().y, (target->GetOrigin().z + target->GetEyePosition().z) / 2);
	Vector PStartL = Vector(csgo->local->GetOrigin().x, csgo->local->GetOrigin().y, (csgo->local->GetOrigin().z + csgo->local->GetEyePosition().z) / 2);;
	//Setup traces
	Math::AngleVectors(Vector(0, RelativeEAngle - 90, 0), ETraceL);
	Math::AngleVectors(Vector(0, RelativeEAngle + 90, 0), ETraceR);

	Math::AngleVectors(Vector(0, LocalRelativeAngle - 90, 0), LTraceL);
	Math::AngleVectors(Vector(0, LocalRelativeAngle + 90, 0), LTraceR);

	trace_t Left;
	trace_t Right;
	TraceLine(PStartL + LTraceL * 50, PStart + ETraceL * 50, MASK_SOLID, csgo->local, &Left);
	TraceLine(PStartL + LTraceR * 50, PStart + ETraceR * 50, MASK_SOLID, csgo->local, &Right);
	if ((Left.fraction == 1.0f && Right.fraction == 1.0f) || (Left.fraction != 1.0f && Right.fraction != 1.0f)) { //both hit /  both not hit
		return -1337;
	}
	else if (Left.fraction != 1.0f && Right.fraction == 1.0f) { //left hit rigth not hit
		trace_t Left2;
		TraceLine(PStartL + LTraceL * 23, PStart + ETraceL * 23, MASK_SOLID, csgo->local, &Left2);
		if (Left2.fraction == 1.0f) {
			return -1337;
		}
		return -1;
	}
	else if (Left.fraction == 1.0f && Right.fraction != 1.0f) { //left didnt hit but right hit
		trace_t Right2;
		TraceLine(PStartL + LTraceR * 23, PStart + ETraceR * 23, MASK_SOLID, csgo->local, &Right2);
		if (Right2.fraction == 1.0f) {
			return -1337;
		}
		return 1;
	}
	return -1337;
}

int UpdateDirection() {
	if (!csgo->local->isAlive())
		return -1337;
	IBasePlayer* target = nullptr;
	auto best_fov = FLT_MAX;
	Vector localviewangles;
	interfaces.engine->GetViewAngles(localviewangles);

	for (auto i = 1; i < interfaces.global_vars->maxClients; i++)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e->IsValid() || !e->GetWeapon())
			continue;




		auto fov = Math::GetFov(localviewangles, Math::CalculateAngle(csgo->local->GetEyePosition(), e->GetOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}





	if (!target)
		return -1337;

	float RelativeEAngle = Math::CalculateAngle(target->GetOrigin(), csgo->local->GetEyePosition()).y + 180;
	float LocalRelativeAngle = Math::CalculateAngle(csgo->local->GetEyePosition(), target->GetOrigin()).y + 180;

	//setup Vectors
	Vector ETraceL;
	Vector ETraceR;
	Vector LTraceL;
	Vector LTraceR;
	Vector PStart = Vector(target->GetOrigin().x, target->GetOrigin().y, (target->GetOrigin().z + target->GetEyePosition().z) / 2);
	Vector PStartL = Vector(csgo->local->GetOrigin().x, csgo->local->GetOrigin().y, (csgo->local->GetOrigin().z + csgo->local->GetEyePosition().z) / 2);;
	//Setup traces
	Math::AngleVectors(Vector(0, RelativeEAngle - 90, 0), ETraceL);
	Math::AngleVectors(Vector(0, RelativeEAngle + 90, 0), ETraceR);

	Math::AngleVectors(Vector(0, LocalRelativeAngle - 90, 0), LTraceL);
	Math::AngleVectors(Vector(0, LocalRelativeAngle + 90, 0), LTraceR);

	trace_t Left;
	trace_t Right;
	TraceLine(PStartL + LTraceL * 50, PStart + ETraceL * 50, MASK_SOLID, csgo->local, &Left);
	TraceLine(PStartL + LTraceR * 50, PStart + ETraceR * 50, MASK_SOLID, csgo->local, &Right);
	if ((Left.fraction == 1.0f && Right.fraction == 1.0f) || (Left.fraction != 1.0f && Right.fraction != 1.0f)) { //both hit /  both not hit
		return -1337;
	}
	else if (Left.fraction != 1.0f && Right.fraction == 1.0f) { //left hit rigth not hit
		return -1;
	}
	else if (Left.fraction == 1.0f && Right.fraction != 1.0f) { //left didnt hit but right hit
		return 1;
	}
	return -1337;
}


void CMAntiAim::Fakelag(bool& send_packet)
{
	if (!vars.antiaim.enable)
		return;

	if (vars.antiaim.fakepeek->active || vars.ragebot.double_tap->active || vars.ragebot.hideshots->active) {

			send_packet = true;
		
		return;
	}

	



	bool dt = CanDT();
	bool hs = CanHS();

	bool exp = dt || hs;
	csgo->max_fakelag_choke = 14;
	if (csgo->fake_duck && csgo->local->GetFlags() & FL_ONGROUND && !(csgo->cmd->buttons & IN_JUMP))
	{
		if (csgo->local->GetFlags() & FL_ONGROUND)
			return;
	}

	if (CanHS()
		|| interfaces.engine->IsVoiceRecording()) {
		csgo->max_fakelag_choke = 1;
		return;
	}

	

	if ((Ragebot::Get().shot) && vars.antiaim.fakelagonshot == 0 && !vars.ragebot.double_tap->active) {
		if (vars.ragebot.double_tap->active)
		{
			send_packet = true;

			return;
		}
		else
		{
			send_packet = true;
			csgo->max_fakelag_choke = exp ? 1 : 2;
			return;
		}
	}

	if (vars.ragebot.double_tap->active)
	{
		csgo->max_fakelag_choke = 1;
	}

	if (exp)
	{
		send_packet = csgo->client_state->iChokedCommands >= 1;
		return;
	}

	auto animstate = csgo->local->GetPlayerAnimState();
	if (!animstate)
		return;

	int tick_to_choke = 1;
	static Vector oldOrigin;

	if (!(csgo->local->GetFlags() & FL_ONGROUND))
	{
		csgo->canDrawLC = true;
	}
	else {
		csgo->canDrawLC = false;
		csgo->canBreakLC = false;
	}


	if (Math::RandomInt(0, 100) > vars.antiaim.fakelagjitter)
	{
		tick_to_choke = clamp(min(vars.antiaim.fakelag_min, vars.antiaim.fakelagfactor), 1, csgo->max_fakelag_choke);
	}
	else
	{
		tick_to_choke = clamp(max(vars.antiaim.fakelag_min, vars.antiaim.fakelagfactor), 1, csgo->max_fakelag_choke);
	}


	tick_to_choke = clamp(tick_to_choke, 0, csgo->max_fakelag_choke);
	if (vars.checkbox.fakelagflags & 1 || vars.checkbox.fakelagflags & 2 || vars.checkbox.fakelagflags & 4 || vars.checkbox.fakelagflags & 8 || vars.checkbox.fakelagflags & 16 || vars.checkbox.fakelagflags & 32)
	{
		static int Time = 0;
		if (vars.checkbox.fakelagflags & 1 && (Ragebot::Get().hitchanced)) //peek
		{
			Time = interfaces.global_vars->tickcount + 75;
		}
		if (Time >= interfaces.global_vars->tickcount) {
			tick_to_choke = clamp(vars.checkbox.fakelagaa, 0, csgo->max_fakelag_choke);
		}
		else if (vars.checkbox.fakelagflags & 2 && !(csgo->local->GetFlags() & FL_ONGROUND)) //in air
		{
			tick_to_choke = clamp(vars.checkbox.fakelagaa, 0, csgo->max_fakelag_choke);
		}
		else if (vars.checkbox.fakelagflags & 4 && vars.antiaim.slowwalk->active) //slow walk
		{
			tick_to_choke = clamp(vars.checkbox.fakelagaa, 0, csgo->max_fakelag_choke);
		}
		else if (vars.checkbox.fakelagflags & 8 && csgo->local->GetVelocity().Length2D() >= 110.f) //moving
		{
			tick_to_choke = clamp(vars.checkbox.fakelagaa, 0, csgo->max_fakelag_choke);
		}
		else if (vars.checkbox.fakelagflags & 16 && csgo->cmd->buttons & IN_ATTACK) //shooting
		{
			tick_to_choke = clamp(vars.checkbox.fakelagaa, 0, csgo->max_fakelag_choke);
		}
		else if (vars.checkbox.fakelagflags & 32 && m_in_lby_update) //lby updatet
		{
			tick_to_choke = clamp(vars.checkbox.fakelagaa, 0, csgo->max_fakelag_choke);
		}
	}

	//if (csgo->game_rules->IsValveDS())
	//	tick_to_choke = ClampIntT(tick_to_choke, 0, 8);
	if (!vars.checkbox.fakelag) {
		tick_to_choke = 1;
	}
	vars.antiaim.sharedfakelag = tick_to_choke;
	send_packet = csgo->client_state->iChokedCommands >= tick_to_choke;

	static Vector sent_origin = Vector();

	if (!(csgo->local->GetFlags() & FL_ONGROUND) && vars.visuals.indicators & 2) {
		if (send_packet)
			sent_origin = csgo->local->GetAbsOrigin();

		if ((sent_origin - oldOrigin).LengthSqr() > 4096.f) {
			csgo->canBreakLC = true;
		}
		else
			csgo->canBreakLC = false;

		if (send_packet)
			oldOrigin = csgo->local->GetAbsOrigin();
	}
}

void CMAntiAim::Pitch()
{
	if (!vars.checkbox.pitch)
	{
		return;
	}
	switch (vars.antiaim.pitch) {
	case 0: csgo->cmd->viewangles.x = 89;
		break;
	case 1:	csgo->cmd->viewangles.x = 0;
		break;
	case 2:	csgo->cmd->viewangles.x = -89;
		break;
	case 3:
		switch (rand() % 2) {
		case 0:
			csgo->cmd->viewangles.x = 89;
			break;
		case 1:
			csgo->cmd->viewangles.x = -89;
			break;
		}
	case 4:
		csgo->cmd->viewangles.x = -18000000;
		break;
	case 5:
		switch (rand() % 2) {
		case 0:
			csgo->cmd->viewangles.x = 180;
			break;
		case 1:
			csgo->cmd->viewangles.x = -180;
			break;
		}
		break;
	}
}

void CMAntiAim::Sidemove() {
	if (!csgo->should_sidemove)
		return;

	const float& sideAmount = csgo->cmd->buttons & IN_DUCK || csgo->fake_duck ? 3.25f : 1.01f;
	if (csgo->local->GetVelocity().Length2D() <= 0.f || std::fabs(csgo->local->GetVelocity().z) <= 100.f)
		csgo->cmd->sidemove += csgo->cmd->command_number % 2 ? sideAmount : -sideAmount;
}

float GetCurtime() {
	if (!csgo->local)
		return 0;
	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = csgo->local->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = csgo->cmd;
	float curtime = g_tick * interfaces.global_vars->interval_per_tick;
	return curtime;
}

float CMAntiAim::corrected_tickbase()
{
	CUserCmd* last_ucmd = nullptr;
	int corrected_tickbase = 0;

	corrected_tickbase = (!last_ucmd || last_ucmd->hasbeenpredicted) ? (float)csgo->local->GetTickBase() : corrected_tickbase++;
	last_ucmd = csgo->cmd;
	float corrected_curtime = corrected_tickbase * interfaces.global_vars->interval_per_tick;
	return corrected_curtime;
}

void CMAntiAim::predict_lby_update(float sampletime, CUserCmd* ucmd, bool& sendpacket)
{
	csgo->InLbyUpdate = false;
	static float next_lby_update_time = 0;
	auto local = csgo->local;

	if (!(local->GetFlags() & 1))
		return;

	if (local->GetVelocity().Length2D() > 0.1f)
		next_lby_update_time = corrected_tickbase() + 0.22f;
	else if (next_lby_update_time - corrected_tickbase() <= 0.0f) {
		next_lby_update_time = corrected_tickbase() + 1.1f;
		csgo->InLbyUpdate = true;
		sendpacket = false;
	}
	else if (next_lby_update_time - corrected_tickbase() <= 1 * interfaces.global_vars->interval_per_tick)
		sendpacket = true;
}

void CMAntiAim::UpdateLBY()
{
	static float m_nTickBase;
	static CUserCmd* last_cmd;
	if (!last_cmd || last_cmd->hasbeenpredicted)
		m_nTickBase = csgo->local->GetTickBase();
	else
		++m_nTickBase;

	last_cmd = csgo->cmd;
	float curtime = TICKS_TO_TIME(m_nTickBase);
	static float o_lby_update_time;

	/* reset values every tick */
	m_in_lby_update = false;
	m_in_balance_update = false;

	/* do only when on ground */
	if (!(csgo->local->GetFlags() & FL_ONGROUND)) {
		return;
	}

	/* get our lby timing */
	if (csgo->local->GetVelocity().Length() > 0.1f) {
		m_lby_update_time = curtime + 0.22f;
	}
	if (m_lby_update_time < curtime) {
		m_lby_update_time = curtime + 1.1f;
		m_in_lby_update = true;
	}

	/* see if we are in balance update */
	if (m_lby_update_time - interfaces.global_vars->interval_per_tick < curtime)
		m_in_balance_update = true;
}

void FreeStanding() // cancer v1
{
	static float FinalAngle;
	bool bside1 = false;
	bool bside2 = false;
	bool autowalld = false;
	for (int i = 1; i <= interfaces.engine->GetMaxClients(); ++i)
	{
		IBasePlayer* pPlayerEntity = interfaces.ent_list->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->isAlive()
			|| pPlayerEntity->IsDormant()
			|| pPlayerEntity == csgo->local
			|| pPlayerEntity->GetTeam() == csgo->local->GetTeam())
			continue;

		float angToLocal = Math::CalculateAngle(csgo->local->GetOrigin(), pPlayerEntity->GetOrigin()).y;

		Vector ViewPoint = csgo->local->GetOrigin() + Vector(0, 0, 90);
		Vector2D Side1 = { (45 * sin(DEG2RAD(angToLocal))),(45 * cos(DEG2RAD(angToLocal))) };
		Vector2D Side2 = { (45 * sin(DEG2RAD(angToLocal + 180))) ,(45 * cos(DEG2RAD(angToLocal + 180))) };

		Vector2D Side3 = { (50 * sin(DEG2RAD(angToLocal))),(50 * cos(DEG2RAD(angToLocal))) };
		Vector2D Side4 = { (50 * sin(DEG2RAD(angToLocal + 180))) ,(50 * cos(DEG2RAD(angToLocal + 180))) };

		Vector Origin = csgo->local->GetOrigin();

		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };

		for (int side = 0; side < 2; side++)
		{
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 80 };
			Vector OriginAutowall2 = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

			if (g_AutoWall.CanHitFloatingPoint(OriginAutowall, ViewPoint))
			{
				if (side == 0)
				{
					bside1 = true;
					FreestandAngle = -1;
				}
				else if (side == 1)
				{
					bside2 = true;
					FreestandAngle = -1;
				}
				autowalld = true;
			}
			else
			{
				for (int side222 = 0; side222 < 2; side222++)
				{
					Vector OriginAutowall222 = { Origin.x + OriginLeftRight[side222].x,  Origin.y - OriginLeftRight[side222].y , Origin.z + 80 };

					if (g_AutoWall.CanHitFloatingPoint(OriginAutowall222, OriginAutowall2))
					{
						if (side222 == 0)
						{
							bside1 = true;
							FreestandAngle = -1;
						}
						else if (side222 == 1)
						{
							bside2 = true;
							FreestandAngle = 1;
						}
						autowalld = true;
					}
				}
			}
		}
	}
}





float get_max_desync_delta()
{
	uintptr_t animstate = uintptr_t(csgo->local->GetPlayerAnimState());


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

void CMAntiAim::Exploited(bool& send_packet)
{
	
	float desync = csgo->local->GetDSYDelta();
	float lby_delta = 180.0f - desync + 58.0f;
	float inverter = vars.antiaim.inverter->active ? 0 : 1;
	size_t i = 0u;
	while (i <= csgo->client_state->iChokedCommands)
	{
		if (i != csgo->client_state->iChokedCommands)
		{
			bool can_adjust_velocity = true;
			if (csgo->cmd->buttons & IN_FORWARD || csgo->cmd->buttons & IN_MOVELEFT || csgo->cmd->buttons & IN_MOVERIGHT || csgo->cmd->buttons & IN_BACK)
				can_adjust_velocity = false;

			if (can_adjust_velocity)
				csgo->cmd->sidemove = (i % 2) ? 2 : -2;

			if (interfaces.global_vars->curtime < interfaces.global_vars->curtime <= csgo->next_lby_update)
				csgo->cmd->viewangles.y += desync;

			bool Freestandside = vars.movable.freestand_side == 0;

			if (vars.checkbox.freestand && vars.movable.overidefreestand)
			{
				if (vars.movable.freestand == 0)
				{
					if (!Freestandside)
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 + desync);
					}
					else
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 - desync);
					}
				}
				else
				{
					if (Freestandside)
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 + desync);
					}
					else
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 - desync);
					}
				}
			}
			else if (!vars.ragebot.autoinvert)
			{
				if (inverter)
				{
					if (lby_delta)
						csgo->cmd->viewangles.y += (180 - desync);
				}
				else
				{
					if (lby_delta)
						csgo->cmd->viewangles.y += (180 + desync);
				}
			}
			else
			{
				if (vars.ragebot.autospeed) {
					static int counter = 0;
					if (counter < 20)
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 - desync);
					}
					else if (counter > 20 && counter < 40)
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 + desync);
					}
					else if (counter > 40)
					{
						counter = 0;
					}
					counter++;
				}
				else
				{
					if (GlobalAAJitter)
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 - desync);
					}
					else
					{
						if (lby_delta)
							csgo->cmd->viewangles.y += (180 + desync);
					}
				}
			}
			csgo->cmd->viewangles.y = Math::NormalizeYaw(csgo->cmd->viewangles.y);
		}
		i++;
	}
}


int RandomIntCeilingAndFloor(int floor, int ceiling)
{
	int r = (int)rand() / (int)RAND_MAX;
	return floor + r * (floor - ceiling);
}

float GetLocalDesync(float MaxDesync)
{
	auto player = csgo->local;
	uintptr_t animstate = uintptr_t(player->GetPlayerAnimState());


	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = std::fmax(0, std::fmin(*reinterpret_cast<float*>(animstate + 0xF8), 1));

	float speedfactor = std::fmax(0, std::fmin(1, *reinterpret_cast<float*> (animstate + 0xFC)));

	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.1f;
	float unk3;

	unk2 += ((duckammount * speedfactor) * (0.5 - 58.0f));

	unk3 = *(float*)(animstate + 0x334) * unk2;
	return std::clamp((MaxDesync / 60) * unk3, 0.f, 60.f);
}

float GetAtTargetsYaw()
{
	if (!csgo->local->isAlive())
		return -1337;
	IBasePlayer* target = nullptr;
	auto best_fov = FLT_MAX;
	Vector localviewangles;
	interfaces.engine->GetViewAngles(localviewangles);

	for (auto i = 1; i < interfaces.global_vars->maxClients; i++)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e->IsValid() || !e->GetWeapon())
			continue;




		auto fov = Math::GetFov(localviewangles, Math::CalculateAngle(csgo->local->GetEyePosition(), e->GetOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}





	if (!target)
		return -1337;

	return Math::CalculateAngle(csgo->local->GetEyePosition(), target->GetOrigin()).y + 180 + vars.antiaim.yaw_angle_start;
}



float GetEdgeYaw() {
	float DhitV = 0;
	Vector LocalEyePos = csgo->local->GetEyePosition();
	Vector Forward;
	int Hits = 0;
	Vector EndTracePos;
	for (int angle = 0; angle < 360; angle++) {
		Math::AngleVectors(Vector(0, angle, 0), Forward);
		EndTracePos = LocalEyePos + Forward * 44; //too short make a bit longer
		trace_t TraceResult;
		TraceLine(LocalEyePos, EndTracePos, MASK_SOLID, csgo->local, &TraceResult);

		if (TraceResult.fraction != 1.0f) {
			DhitV += Math::CalculateAngle(LocalEyePos, TraceResult.endpos).y;
			Hits++;
		}


		//return Math::CalculateAngle(csgo->local->GetEyePosition(), TraceResult.endpos).y + 180;

	}
	if (Hits != 0) {
		float D = DhitV / Hits;
		if (D < -180) {
			D = 180 - (-180 + D);
		}
		else if (D > 180) {
			D = -180 + (D - 180);
		}
		return D;
	}

	return -1337;
}

float GetEdgeYaw2() {
	float DhitV = 0;
	Vector LocalEyePos = csgo->local->GetEyePosition();
	Vector Forward;
	int Hits = 0;
	Vector EndTracePos;
	for (int angle = 0; angle < 360; angle++) {
		Math::AngleVectors(Vector(0, angle, 0), Forward);
		EndTracePos = LocalEyePos + Forward * 44; //too short make a bit longer
		trace_t TraceResult;
		TraceLine(LocalEyePos, EndTracePos, MASK_SOLID, csgo->local, &TraceResult);

		if (TraceResult.fraction != 1.0f) {
			float B = Math::CalculateAngle(LocalEyePos, TraceResult.endpos).y;
			if (abs(B) > abs(DhitV)) {
				DhitV = B;
			}
			Hits++;
		}


		//return Math::CalculateAngle(csgo->local->GetEyePosition(), TraceResult.endpos).y + 180;

	}
	if (Hits != 0) {
		float DD = GetEdgeYaw();
		float D2 = abs(DD - DhitV);
		float D3 = DD + D2;

		Math::AngleVectors(Vector(0, D3, 0), Forward);
		EndTracePos = LocalEyePos + Forward * 47;
		trace_t TraceResult;
		TraceLine(LocalEyePos, EndTracePos, MASK_SOLID, csgo->local, &TraceResult);



		float D = DhitV;

		if (TraceResult.fraction != 1.0f) {
			D = DD;
		}


		if (D < -180) {
			D = 180 - (-180 + D);
		}
		else if (D > 180) {
			D = -180 + (D - 180);
		}
		return D;
	}

	return -1337;
}


void CMAntiAim::Yaw(bool& send_packet)
{

	float originalyaw = csgo->cmd->viewangles.y;
	static bool cantforcejitter = true;
	static bool a = false;
	csgo->should_sidemove = false;
	
		vars.antiaim.fakepeeking = false;
		int side = csgo->SwitchAA ? 1 : -1;
		static int CFSide;
		int AutoDirD = 0;
		if (vars.movable.freestand > 0 || vars.movable.HideAA->active) {
			CFSide = UpdateDirection2(38);
			if (CFSide != -1337) {
				if (vars.movable.freestand == 1) {
					if (CFSide == -1) {
						if (!vars.antiaim.inverter->active) {
							vars.antiaim.inverter->active = true;
						}
					}
					else
					{
						if (vars.antiaim.inverter->active) {
							vars.antiaim.inverter->active = false;
						}
					}
				}
				else {
					if (CFSide == -1) {
						if (vars.antiaim.inverter->active) {
							vars.antiaim.inverter->active = false;
						}
					}
					else
					{
						if (!vars.antiaim.inverter->active) {
							vars.antiaim.inverter->active = true;
						}
					}
				}
			}
		}
		if (vars.movable.HideAA->active) {
			if (GetEdgeYaw() != -1337) 		{
				int CSide = UpdateDirection2(35);
			if (CSide == -1) {
				AutoDirD = -1;
			}
			else if (CSide == 1) {
				AutoDirD = 1;
			}
			}
		}
		int mDesync = 120;

		static int lby_break_side;
		float Spinn = interfaces.global_vars->curtime * 45.f;


		if (vars.antiaim.manual.left->active && vars.antiaim.manual.left->type == 2) {
			vars.antiaim.manual.left->active = false;
			if (vars.antiaim.manual.direction == -1) {
				vars.antiaim.manual.direction = 0;
			}
			else
			{
				vars.antiaim.manual.direction = -1;
			}
		}

		if (vars.antiaim.manual.right->active && vars.antiaim.manual.right->type == 2) {
			vars.antiaim.manual.right->active = false;
			if (vars.antiaim.manual.direction == 1) {
				vars.antiaim.manual.direction = 0;
			}
			else
			{
				vars.antiaim.manual.direction = 1;
			}
		}





		/*
		if (vars.antiaim.manual.direction == -1 && !vars.checkbox.yawbase)
			csgo->cmd->viewangles.y -= 90;
		if (vars.antiaim.manual.direction == 1 && !vars.checkbox.yawbase)
			csgo->cmd->viewangles.y += 90;*/

		if (AutoDirD == 0) {

			if (vars.antiaim.manual.direction == -1)// && vars.checkbox.yawbase)
				csgo->cmd->viewangles.y += 90;
			if (vars.antiaim.manual.direction == 1)// && vars.checkbox.yawbase)
				csgo->cmd->viewangles.y -= 90;

		}
		else
		{
			if (AutoDirD == -1)// && vars.checkbox.yawbase)
				csgo->cmd->viewangles.y += 90;
			if (AutoDirD == 1)// && vars.checkbox.yawbase)
				csgo->cmd->viewangles.y -= 90;
		}
		//csgo->cmd->viewangles.y += 180;



		if (vars.antiaim.manual.direction == 0 && AutoDirD == 0) {
			if (vars.movable.autodir || vars.keybind.autodir->active) {
				float ADC2 = GetEdgeYaw2();
				csgo->cmd->viewangles.y = ADC2 == -1337 ? csgo->cmd->viewangles.y + 180 : ADC2;
			}
			else if (vars.antiaim.manual.edgeyaw->active) {
				float ADC = GetEdgeYaw();
				csgo->cmd->viewangles.y = ADC == -1337 ? csgo->cmd->viewangles.y + 180 : ADC;
			}
			else if (vars.antiaim.baseangle == 1 && vars.checkbox.yawbase)
			{
				csgo->cmd->viewangles.y += 180;
				csgo->cmd->viewangles.y += vars.antiaim.yaw_angle_start;
			}
			else if (vars.antiaim.baseangle == 2 && vars.checkbox.yawbase) {
				float ADC = GetAtTargetsYaw();
				csgo->cmd->viewangles.y = ADC == -1337 ? csgo->cmd->viewangles.y + 180 : ADC;



			}
			else if (vars.antiaim.baseangle == 0 && vars.checkbox.yawbase)
			{
				if (vars.antiaim.baseangle_direction == 0)
				{
					csgo->cmd->viewangles.y += 180;
				}
				else if (vars.antiaim.baseangle_direction == 1)
				{
					csgo->cmd->viewangles.y += 90;
				}
				else if (vars.antiaim.baseangle_direction == 2)
				{
					csgo->cmd->viewangles.y -= 90;
				}
			}
		}

		if (vars.checkbox.yawmodifier)
		{
			switch (vars.antiaim.yawmodifier)
			{
			case 0:
				a ? csgo->cmd->viewangles.y += vars.antiaim.yawmodifier_value : csgo->cmd->viewangles.y -= vars.antiaim.yawmodifier_value;


				break;
			case 1:

				if (a)
				{
					csgo->cmd->viewangles.y -= vars.antiaim.yawmodifier_value; break;
				}

				break;
			case 2:

				a ? csgo->cmd->viewangles.y += 180 : csgo->cmd->viewangles.y -= 180;
				break;
			case 3:
				static int spin;
				csgo->cmd->viewangles.y += spin;
				if (vars.antiaim.yawmodifier_value > 10)
					spin -= vars.antiaim.yawmodifier_value / 5;
				else
					spin++;
				if (spin < -361)
					spin = 0;
				break;
			case 4:
				a ? csgo->cmd->viewangles += rand() % vars.antiaim.yawmodifier_value : csgo->cmd->viewangles += rand() % vars.antiaim.yawmodifier_value;


			}
		}



		


		vars.antiaim.zi180 = false;
		vars.ragebot.random_resolver = false;
		int dside = side;
		if(vars.antiaim.customAAT == 1)
		dside = a ? 1 : -1;
		int Desync = vars.antiaim.DesyncAMM * 2;
		static int DynamicSide = side;

		if (vars.antiaim.customAAT == 2)
		{
			//do desync check and jitter ya know
		}

		
	
		/*
		switch (vars.antiaim.yaw_mod_type)
		{
		case 1: //static
			break;

		case 2: //jitter


		case 3: //random
			break;
		} */

		

		

		if (vars.antiaim.LBYT == 0) {
			csgo->should_sidemove = true;
		}
		else 	if (vars.antiaim.LBYT == 1) {
			csgo->should_sidemove = false;
		}
		else
		{
			if (CanDT()) {
				csgo->should_sidemove = true;
			}
			else {
				csgo->should_sidemove = false;
			}
		}


		if (m_in_lby_update) {
			send_packet = false;
			csgo->cmd->viewangles.y -= Desync * -dside;
		}
		else if (!send_packet) {
			csgo->cmd->viewangles.y -= Desync * dside;
		}


		if (vars.visuals.indicators & 1 && vars.antiaim.enable)
		{
			vars.movable.shareddesync = GetLocalDesync(vars.antiaim.DesyncAMM);
		}

		if (send_packet)
		{
			a = !a;
			GlobalAAJitter = a;
		}
	
}


void CMAntiAim::LegitAA(bool& send_packet)
{
	
	vars.antiaim.fakepeeking = false;
	int side = csgo->SwitchAA ? 1 : -1;

	
	vars.antiaim.zi180 = false;
	vars.ragebot.random_resolver = false;


	/*
	switch (vars.antiaim.yaw_mod_type)
	{
	case 1: //static
		break;

	case 2: //jitter


	case 3: //random
		break;
	} */
	vars.antiaim.zi180 = false;
	vars.ragebot.random_resolver = false;
	int dside = side;
	if (vars.antiaim.customAAT == 1)
		dside = GlobalAAJitter ? 1 : -1;
	int Desync = vars.antiaim.DesyncAMM * 2;




	/*
	switch (vars.antiaim.yaw_mod_type)
	{
	case 1: //static
		break;

	case 2: //jitter


	case 3: //random
		break;
	} */



	if (vars.antiaim.LBYT == 0) {
		csgo->should_sidemove = true;
	}
	else 	if (vars.antiaim.LBYT == 1) {
		csgo->should_sidemove = false;
	}
	else
	{
		if (CanDT()) {
			csgo->should_sidemove = true;
		}
		else {
			csgo->should_sidemove = false;
		}
	}


	if (m_in_lby_update && !CanDT()) {
		send_packet = false;
		csgo->cmd->viewangles.y -= Desync * -dside;
	}
	else if (!send_packet) {
		csgo->cmd->viewangles.y -= Desync * dside;
	}


	if (vars.visuals.indicators & 1 && vars.antiaim.enable)
	{
		vars.movable.shareddesync = GetLocalDesync(vars.antiaim.DesyncAMM);
	}



	if (send_packet)
	{
		GlobalAAJitter = !GlobalAAJitter;
	}
}

void CMAntiAim::SlidyLegs(CUserCmd* cmd, bool T)
{
	//forget it moved to createmove

}


void CMAntiAim::Run(bool& send_packet)
{
	

	if (csgo->send_packet)
		JitterSwitch = !JitterSwitch;


	if (vars.antiaim.slowwalk->active || csgo->should_stop_slide)
	{
		const auto weapon = csgo->weapon;

		if (weapon) {

			const auto info = csgo->weapon->GetCSWpnData();

			float speed = 0.1f;
			if (info) {
				float max_speed = weapon->GetZoomLevel() == 0 ? info->m_flMaxSpeed : info->m_flMaxSpeedAlt;
				float ratio = max_speed / 250.0f;
				speed *= ratio;
			}


			csgo->cmd->forwardmove *= speed;
			csgo->cmd->sidemove *= speed;
		}
	}
	UpdateLBY();
	shouldAA = true;
	if (!vars.antiaim.enable) {
		shouldAA = false;
		return;
	}
	/*
	if (csgo->cmd->buttons & IN_USE)
	{
		shouldAA = false;
		return;
	}
	*/

	if (csgo->game_rules->IsFreezeTime()
		|| csgo->local->GetMoveType() == MOVETYPE_NOCLIP
		|| csgo->local->GetMoveType() == MOVETYPE_LADDER)
	{
		shouldAA = false;
		return;
	}
	bool shit = false;
	for (int i = 1; i < 65; i++)
	{
		auto ent = interfaces.ent_list->GetClientEntity(i);
		if (!ent)
			continue;
		if (
			!ent->isAlive()
			|| ent == csgo->local
			|| ent->GetTeam() == csgo->local->GetTeam()
			)
			continue;
		shit = true;
		break;

	}
	if (!shit)
	{
		if (csgo->ForceOffAA)
		{
			shouldAA = false;
			//return;
			//csgo->legitaaon = true;
		}
	}
	if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
	{
		if (Ragebot::Get().shot /*&& Ragebot::Get().IsAbleToShoot()*/)
		{
			shouldAA = false;
			return;
		}
	}
	else
	{
		if (F::Shooting() || csgo->TickShifted || (CanDT() && csgo->cmd->buttons & IN_ATTACK /*&& Ragebot::Get().IsAbleToShoot()*/))
		{
			shouldAA = false;
			return;
		}
		if (csgo->weapon->IsKnife()) {
			if ((csgo->cmd->buttons & IN_ATTACK || csgo->cmd->buttons & IN_ATTACK2) && Ragebot::Get().IsAbleToShoot())
			{
				shouldAA = false;
				return;
			}
		}
	}
	SlidyLegs(csgo->cmd, shouldAA);
	if (shouldAA)
	{
		if (csgo->cmd->buttons & IN_USE)
		{
			LegitAA(csgo->send_packet);


		}
		else
		{
			Pitch();
			Yaw(csgo->send_packet);
		}
	}

}