
#include "PGrenadePrediction.h"
static throwngrenade_t nadeendforpaint[65];
static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
static auto molotov_throw_detonate_time = interfaces.cvars->FindVar("molotov_throw_detonate_time");
static float fGravity = interfaces.cvars->FindVar("sv_gravity")->GetFloat() / 8.0f;//800.0f / 8.f;

static bool AlreadyFirst[999];
static bool AlreadyEverythink[999];
static Vector eyeangle[999];
static Vector position[999];
static float firstthrowtime[999];
static Vector position2[999];
static Vector position3[999];

void GrenadePrediction::Tick(int buttons)
{

	
}

void GrenadePrediction::View(CViewSetup* setup, IBaseCombatWeapon* weapon, IBasePlayer* entitytodraw, int type2332)
{
	if (csgo->local->isAlive())
	{
		if (!csgo->game_rules->IsFreezeTime())
		{
			if (!entitytodraw)
				return;
			if (entitytodraw == NULL)
				return;
			if (AlreadyEverythink[entitytodraw->GetIndex()] != true) {
				if (AlreadyFirst[entitytodraw->GetIndex()] != true) {
					//set first
					position[entitytodraw->GetIndex()] = entitytodraw->GetAbsOrigin();
					firstthrowtime[entitytodraw->GetIndex()] = csgo->curtime;
					AlreadyFirst[entitytodraw->GetIndex()] = true;
					float current;
					float lowest;
					bool notsettetfirst = false;
					for (auto i = 1; i < 64; i++) //-V807
					{
						lowest = abs(position[entitytodraw->GetIndex()].DistTo(csgo->EyePosForGrenade[i]));//	Vector eyepositionforgren[65];
						if (notsettetfirst == false || current > lowest) {
							auto entity = entitytodraw;
							if (!entity)
								return;
							if (entity == NULL)
								return;
							current = lowest;
							notsettetfirst = true;
							position3[entitytodraw->GetIndex()] = entity->GetEyePosition();// 176
							VecVelocity[entitytodraw->GetIndex()] = entity->GetVelocity();
						}
					}
					return;
				}
				else {
					//set second
					if (csgo->curtime - firstthrowtime[entitytodraw->GetIndex()] > 0.1f) { // wait time for grenade to go
						position2[entitytodraw->GetIndex()] = entitytodraw->GetOrigin();
						AlreadyEverythink[entitytodraw->GetIndex()] = true;

					}


				}
			}
			else {
				eyeangle[entitytodraw->GetIndex()] = Math::CalculateAngle(position[entitytodraw->GetIndex()], position2[entitytodraw->GetIndex()]);
				type[entitytodraw->GetIndex()] = type2332;
				Simulate(setup, entitytodraw);
			}


		}
		else {

			type[entitytodraw->GetIndex()] = 0;
		}

	}
}

inline float CSGO_Armor(float flDamage, int ArmorValue) {
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}
float GetGrenadeDetonateTime(short item)
{
	switch (item)
	{
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return 1.5f;
		break;
	case WEAPON_INCGRENADE:
	case WEAPON_MOLOTOV:
		return molotov_throw_detonate_time->GetFloat();
		break;
	case WEAPON_DECOY:
		return 5.f;
		break;
	}

	return 3.f;
};

bool CheckDetonate32(short m_iItemDefinitionIndex, float speed, float timeAlive, trace_t& tr)
{

	switch (m_iItemDefinitionIndex)
	{
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		if (timeAlive > 2.5f)
			return true;
		break;

	case WEAPON_MOLOTOV:
	case WEAPON_INCGRENADE:
		if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f || timeAlive > 3.5f)
			return true;
		break;

	case WEAPON_DECOY:
	case WEAPON_SMOKEGRENADE:
		if (speed < 0.1f)
			return true;

		break;
	}
	// you can calculate this differently this will not be quite right
	return false;
}
void GrenadePrediction::Paint(IBasePlayer* entitytodraw)
{
	if (!csgo->local->isAlive())
		return;

	if (AlreadyEverythink[entitytodraw->GetIndex()] != true)
		return;
	if (path[entitytodraw->GetIndex()].size() < 2)
		return;

	std::pair <float, IBasePlayer*> target{ 0.f, nullptr };

	Vector prev = path[entitytodraw->GetIndex()][0];


	if (path[entitytodraw->GetIndex()].size() > 1)
	{
		Vector nadeStart, nadeEnd;
		nadeendforpaint[entitytodraw->GetIndex()].m_fllife = csgo->curtime;
		for (auto it = path[entitytodraw->GetIndex()].begin(); it != path[entitytodraw->GetIndex()].end(); ++it)
		{
			if (Math::WorldToScreen2(prev, nadeStart) && Math::WorldToScreen2(*it, nadeEnd)) {
				//render::get().line((int)nadeStart.x, (int)nadeStart.y, (int)nadeEnd.x, (int)nadeEnd.y, g_cfg.esp.grenade_prediction_tracer_color);

			}


			nadeendforpaint[entitytodraw->GetIndex()].m_vecnadeendforpaint = *it;
			prev = *it;
		}
		nadeendforpaint[entitytodraw->GetIndex()].m_vecnadebeginforpaint = *path[entitytodraw->GetIndex()].begin();

		for (auto it = others.begin(); it != others.end(); ++it)
		{
			Vector screen;
		}

		Vector endpos = path[entitytodraw->GetIndex()][path[entitytodraw->GetIndex()].size() - 1];

	}
	if (csgo->curtime - nadeendforpaint[entitytodraw->GetIndex()].m_fllife > GetGrenadeDetonateTime(type[entitytodraw->GetIndex()])) {
		nadeendforpaint[entitytodraw->GetIndex()].m_veceyeang.Zero();
		nadeendforpaint[entitytodraw->GetIndex()].m_vecnadebeginforpaint.Zero();
		nadeendforpaint[entitytodraw->GetIndex()].m_vecnadeendforpaint.Zero();
		nadeendforpaint[entitytodraw->GetIndex()].m_vecvelocity.Zero();
		AlreadyFirst[entitytodraw->GetIndex()] = false;
		AlreadyEverythink[entitytodraw->GetIndex()] = false;
		eyeangle[entitytodraw->GetIndex()].Zero();
		position[entitytodraw->GetIndex()].Zero();
		position3[entitytodraw->GetIndex()].Zero();
		firstthrowtime[entitytodraw->GetIndex()] = 0.f;
		position2[entitytodraw->GetIndex()].Zero();
		VecVelocity[entitytodraw->GetIndex()].Zero();
		type[entitytodraw->GetIndex()] = 0;
	}
	if (nadeendforpaint[entitytodraw->GetIndex()].m_vecnadeendforpaint.x != 0.0f && nadeendforpaint[entitytodraw->GetIndex()].m_vecnadeendforpaint.y != 0.0f) {
		Vector nadeend;
		if (Math::WorldToScreen2(nadeendforpaint[entitytodraw->GetIndex()].m_vecnadeendforpaint, nadeend)) {
			auto timetoexplode = csgo->curtime - nadeendforpaint[entitytodraw->GetIndex()].m_fllife;
			auto screen_origin = nadeend;
			static auto size = Vector2D(35.0f, 5.0f);
			g_Render->CircleFilled(screen_origin.x, screen_origin.y - size.y * 0.5f, 60, color_t(15, 15, 15, 187),35);
			//render::get().rainbowfilledarc(screen_origin.x, screen_origin.y - size.y * 0.5f, 21, 0, (timetoexplode / GetGrenadeDetonateTime(nadeendforpaint[entitytodraw->GetIndex()].m_itemdefindx)) * 360, g_cfg.esp.molotov_timer_color, 2);// 67
			//render::get().rect_filled(screen_origin.x - 2, screen_origin.y - size.y * 0.5f - 15, 5, 20, g_cfg.esp.molotov_timer_color);
			//render::get().rect_filled(screen_origin.x - 2, screen_origin.y - size.y * 0.5f + 8, 5, 5, g_cfg.esp.molotov_timer_color);
		}
	}


}

void GrenadePrediction::Setup(Vector& vecSrc, Vector& vecThrow, const Vector& viewangles, IBasePlayer* entitytodraw)
{
	Vector angThrow = viewangles;
	float pitch = Math::NormalizePitch(angThrow.x);

	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.x = a;

	float flVel = 750.0f * 0.9f;
	static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	float b = power[1];
	b = b * 0.7f; b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	Math::AngleVectors(angThrow, &vForward, &vRight, &vUp);

	vecSrc = position3[entitytodraw->GetIndex()];
	float off = power[1] * 12.0f - 12.0f;
	vecSrc.z += off;

	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f;

	TraceHull(vecSrc, vecDest, tr);

	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	vecThrow = VecVelocity[entitytodraw->GetIndex()]; vecThrow *= 1.25f;
	vecThrow += vForward * flVel;
}

void GrenadePrediction::Simulate(CViewSetup* setup, IBasePlayer* entitytodraw)
{
	Vector vecSrc, vecThrow;
	Vector angles = eyeangle[entitytodraw->GetIndex()];
	Setup(vecSrc, vecThrow, angles, entitytodraw);

	float interval = interfaces.global_vars->interval_per_tick;
	int logstep = (int)(0.05f / interval);
	int logtimer = 0;

	path[entitytodraw->GetIndex()].clear();
	others.clear();

	for (auto i = 0; i < 4096; ++i)
	{
		if (!logtimer)
			path[entitytodraw->GetIndex()].push_back(vecSrc);

		int s = Step(vecSrc, vecThrow, i, interval, entitytodraw);

		if (s & 1)
			break;

		if (s & 2 || logtimer >= logstep)
			logtimer = 0;
		else
			++logtimer;

		if (vecThrow.IsZero())
			break;
	}

	path[entitytodraw->GetIndex()].push_back(vecSrc);
	others.emplace_back(std::make_pair(vecSrc, color_t(255,0,0,255)));
}



int GrenadePrediction::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval, IBasePlayer* entitytodraw)
{
	Vector move; AddGravityMove(move, vecThrow, interval, false);
	trace_t tr; PushEntity(vecSrc, move, tr);

	int result = 0;

	if (CheckDetonate(vecThrow, tr, tick, interval, entitytodraw))
		result |= 1;

	if (tr.fraction != 1.0f) //-V550
	{
		result |= 2;
		ResolveFlyCollisionCustom(tr, vecThrow, move, interval);

		Vector angles;
		Math::VectorAngles((tr.endpos - tr.startpos).Normalized(), angles);
		others.emplace_back(std::make_pair(tr.endpos, color_t(255,255,255,255)));
	}

	vecSrc = tr.endpos;
	return result;
}

bool GrenadePrediction::CheckDetonate(Vector& vecThrow, const trace_t& tr, int tick, float interval, IBasePlayer* entitytodraw)
{
	static auto molotov_throw_detonate_time = interfaces.cvars->FindVar("molotov_throw_detonate_time");
	static auto weapon_molotov_maxdetonateslope = interfaces.cvars->FindVar("weapon_molotov_maxdetonateslope");

	auto time = TICKS_TO_TIME(tick);

	switch (type[entitytodraw->GetIndex()])
	{
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return time >= 1.5f && !(tick % TIME_TO_TICKS(0.2f));

	case WEAPON_SMOKEGRENADE:
		return vecThrow.Length() <= 0.1f && !(tick % TIME_TO_TICKS(0.2f));

	case WEAPON_DECOY:
		return vecThrow.Length() <= 0.2f && !(tick % TIME_TO_TICKS(0.2f));

	case WEAPON_MOLOTOV:
	case WEAPON_FIREBOMB:
		// detonate when hitting the floor.
		if (tr.fraction != 1.f && (std::cos(DEG2RAD(weapon_molotov_maxdetonateslope->GetFloat())) <= tr.plane.normal.z)) //-V550
			return true;

		// detonate if we have traveled for too long.
		// checked every 0.1s
		return time >= molotov_throw_detonate_time->GetFloat() && !(tick % TIME_TO_TICKS(0.1f));

	default:
		return false;
	}

	return false;
}

void GrenadePrediction::TraceHull(Vector& src, Vector& end, trace_t& tr)
{
	Ray_t ray;
	CTraceFilterWorldAndPropsOnly filter;

	ray.Init(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));
	interfaces.trace->TraceRay(ray, 0x200400B, &filter, &tr);
}

void GrenadePrediction::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	static auto sv_gravity = interfaces.cvars->FindVar("sv_gravity");

	// gravity for grenades.
	float gravity = sv_gravity->GetFloat() * 0.4f;

	// move one tick using current velocity.
	move.x = vel.x * interfaces.global_vars->interval_per_tick;
	move.y = vel.y * interfaces.global_vars->interval_per_tick;

	// apply linear acceleration due to gravity.
	// calculate new z velocity.
	float z = vel.z - (gravity * interfaces.global_vars->interval_per_tick);

	// apply velocity to move, the average of the new and the old.
	move.z = ((vel.z + z) / 2.f) * interfaces.global_vars->interval_per_tick;

	// write back new gravity corrected z-velocity.
	vel.z = z;
}

void GrenadePrediction::PushEntity(Vector& src, const Vector& move, trace_t& tr)
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;
	TraceHull(src, vecAbsEnd, tr);
}

void GrenadePrediction::ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, const Vector& move, float interval)
{
	

	float flSurfaceElasticity = 1.0, flGrenadeElasticity = 0.45f;
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f;

	if (flSpeedSqr < flMinSpeedSqr)
	{
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	if (tr.plane.normal.z > 0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval);
		PushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
		vecVelocity = vecAbsVelocity;
}

int GrenadePrediction::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float backoff, change, angle;
	int   i, blocked;

	blocked = 0;
	angle = normal[2];

	if (angle > 0) blocked |= 1;
	if (!angle) blocked |= 2; //-V550

	backoff = in.Dot(normal) * overbounce;
	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	return blocked;
}