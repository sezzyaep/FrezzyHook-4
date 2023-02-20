#include "Hooks.h"
#include "AnimationFix.h"
#include "RageBackTracking.h"
#include "Resolver.h"
#include "Ragebot.h"
#include "AntiAims.h"

CMAnimationFix* g_Animfix = new CMAnimationFix();
CResolver* resolver = new CResolver();

float calculate_lerp()
{
	static auto cl_interp = interfaces.cvars->FindVar(hs::cl_interp::s().c_str());
	static auto cl_updaterate = interfaces.cvars->FindVar(hs::cl_updaterate::s().c_str());
	const auto update_rate = cl_updaterate->GetInt();
	const auto interp_ratio = cl_interp->GetFloat();

	auto lerp = interp_ratio / update_rate;

	if (lerp <= interp_ratio)
		lerp = interp_ratio;

	return lerp;
}

void Extrapolate(IBasePlayer* player, Vector& origin, Vector& velocity, int& flags, bool on_ground)
{
	static const auto sv_gravity = interfaces.cvars->FindVar(hs::sv_gravity::s().c_str());
	static const auto sv_jump_impulse = interfaces.cvars->FindVar(hs::sv_jump_impulse::s().c_str());

	if (!(flags & FL_ONGROUND))
		velocity.z -= TICKS_TO_TIME(sv_gravity->GetFloat());
	else if (player->GetFlags() & FL_ONGROUND && !on_ground)
		velocity.z = sv_jump_impulse->GetFloat();

	const auto src = origin;
	auto end = src + velocity * interfaces.global_vars->interval_per_tick;

	Ray_t r;
	r.Init(src, end, player->GetMins(), player->GetMaxs());

	trace_t t;
	CTraceFilter filter;
	filter.pSkip = player;

	interfaces.trace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

	if (t.fraction != 1.f)
	{
		for (auto i = 0; i < 2; i++)
		{
			velocity -= t.plane.normal * velocity.Dot(t.plane.normal);

			const auto dot = velocity.Dot(t.plane.normal);
			if (dot < 0.f)
				velocity -= Vector(dot * t.plane.normal.x,
					dot * t.plane.normal.y, dot * t.plane.normal.z);

			end = t.endpos + velocity * TICKS_TO_TIME(1.f - t.fraction);

			r.Init(t.endpos, end, player->GetMins(), player->GetMaxs());
			interfaces.trace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

			if (t.fraction == 1.f)
				break;
		}
	}

	origin = end = t.endpos;
	end.z -= 2.f;

	r.Init(origin, end, player->GetMins(), player->GetMaxs());
	interfaces.trace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

	flags &= ~FL_ONGROUND;

	if (t.DidHit() && t.plane.normal.z > .7f)
		flags |= FL_ONGROUND;
}

bool animation::is_valid(float range = .2f, float max_unlag = .2f)
{
	if (!interfaces.engine->GetNetChannelInfo() || !valid)
		return false;

	const auto correct = std::clamp(interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)
		+ interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING)
		+ calculate_lerp(), 0.f, max_unlag);
	//if (CanDT() && csgo->dt_charged && !CMAntiAim::Get().did_shot && !vars.ragebot.disable_dt_delay)
	//	range += TICKS_TO_TIME(8);
	return fabsf(correct - (interfaces.global_vars->curtime - sim_time)) < range && correct < 1.f;
}


animation::animation(IBasePlayer* player)
{
	const auto weapon = player->GetWeapon();

	this->player = player;
	index = player->GetIndex();
	dormant = player->IsDormant();
	velocity = player->GetVelocity();
	origin = player->GetOrigin();
	abs_origin = player->GetAbsOrigin();
	obb_mins = player->GetMins();
	obb_maxs = player->GetMaxs();
	memcpy(layers, player->GetAnimOverlays(), sizeof(CAnimationLayer) * 13);
	poses = player->m_flPoseParameter();
	//if ((has_anim_state = player->GetPlayerAnimState()))
	anim_state = player->GetPlayerAnimState();
	//anim_time = player->GetOldSimulationTime() + interfaces.global_vars->interval_per_tick;
	sim_time = player->GetSimulationTime();
	interp_time = 0.f;
	last_shot_time = weapon ? weapon->GetLastShotTime() : 0.f;
	duck = player->GetDuckAmount();
	lby = player->GetLBY();
	flags = player->GetFlags();
	eflags = player->GetEFlags();
	effects = player->GetEffects();

	lag = TIME_TO_TICKS(player->GetSimulationTime() - player->GetOldSimulationTime());

	// animations are off when we enter pvs, we do not want to shoot yet.
	valid = lag >= 0 && lag <= 17;

	// clamp it so we don't interpolate too far : )
	lag = std::clamp(lag, 0, 17);
}

animation::animation(IBasePlayer* player, Vector last_reliable_angle) : animation(player)
{
	this->last_reliable_angle = last_reliable_angle;
}

void animation::restore(IBasePlayer* player) const
{
	player->GetVelocity() = velocity;
	player->GetFlagsPtr() = flags;
	player->GetEFlags() = eflags;
	player->GetDuckAmount() = duck;
	memcpy(player->GetAnimOverlays(), layers, sizeof(CAnimationLayer) * 13);
	player->GetLBY() = lby;
	player->GetOrigin() = origin;
	player->SetAbsOrigin(abs_origin);
	//player->m_flPoseParameter() = poses;
}

void animation::apply(IBasePlayer* player) const
{
	//player->m_flPoseParameter() = poses;
	*player->GetEyeAnglesPointer() = player->GetEyeAngles();
	player->GetVelocity() = velocity;
	player->GetLBY() = lby;
	player->GetDuckAmount() = duck;
	player->GetFlagsPtr() = flags;
	player->GetOrigin() = origin;
	player->SetAbsOrigin(abs_origin);
	/*if (anim_state) {
		player->SetAnimState(anim_state);
	}*/
}
#define ENT_VAL(ent, t, o) (*( t* )( ( uintptr_t )( ent ) + o ))

#define m_nClientEffects(ent) (ENT_VAL(ent,int,0x68))
#define m_nLastSkipFramecount(ent) (ENT_VAL(ent,int,0xA68))
#define m_nOcclusionMask(ent) (ENT_VAL(ent,int,2600))
#define m_nOcclusionFrame(ent) (ENT_VAL(ent,int,2608))
#define m_bJiggleBones(ent) (ENT_VAL(ent,bool,0x2930))
#define m_bMaintainSequenceTransition(ent) (ENT_VAL(ent,bool,0x9F0))
void BuildBones(IBasePlayer* player, matrix* mat) {
	std::array < CAnimationLayer, 13 > aAnimationLayers;

	float flCurTime = interfaces.global_vars->curtime;
	float flRealTime = interfaces.global_vars->realtime;
	float flFrameTime = interfaces.global_vars->frametime;
	float flAbsFrameTime = interfaces.global_vars->absoluteframetime;
	int iFrameCount = interfaces.global_vars->framecount;
	int iTickCount = interfaces.global_vars->tickcount;
	float flInterpolation = interfaces.global_vars->interpolation_amount;
	float sim_time = player->GetSimulationTime();
	interfaces.global_vars->curtime = sim_time;
	interfaces.global_vars->realtime = sim_time;
	interfaces.global_vars->frametime = interfaces.global_vars->interval_per_tick;
	interfaces.global_vars->absoluteframetime = interfaces.global_vars->interval_per_tick;
	interfaces.global_vars->framecount = INT_MAX;
	interfaces.global_vars->tickcount = TIME_TO_TICKS(sim_time);
	interfaces.global_vars->interpolation_amount = 0.0f;

	int nClientEffects = m_nClientEffects(player);
	int nLastSkipFramecount = m_nLastSkipFramecount(player);
	int nOcclusionMask = m_nOcclusionMask(player);
	int nOcclusionFrame = m_nOcclusionFrame(player);
	int iEffects = player->GetEffects();
	bool bJiggleBones = m_bJiggleBones(player);
	bool bMaintainSequenceTransition = m_bMaintainSequenceTransition(player);
	Vector vecAbsOrigin = player->GetAbsOrigin();

	int iMask = 0x7FF00;


	std::memcpy(aAnimationLayers.data(), player->GetAnimOverlays(), sizeof(CAnimationLayer) * 13);

	player->InvalidateBoneCache();
	player->GetBoneAccessor()->m_ReadableBones = NULL;
	player->GetBoneAccessor()->m_WritableBones = NULL;

	if (player->GetPlayerAnimState())
		player->GetPlayerAnimState()->last_weapon = player->GetPlayerAnimState()->weapon;

	m_nOcclusionFrame(player) = 0;
	m_nOcclusionMask(player) = 0;
	m_nLastSkipFramecount(player) = 0;

	if (player != csgo->local)
		player->SetAbsOrigin(player->GetOrigin());

	player->GetEffects() |= 8;
	m_nClientEffects(player) |= 2;
	m_bJiggleBones(player) = false;
	m_bMaintainSequenceTransition(player) = false;

	player->GetAnimOverlays()[12].m_weight = 0.0f;

	

	csgo->UpdateMatrix = true;
	player->SetupBones(mat, 128, iMask, sim_time);
	csgo->UpdateMatrix = false;

	m_bMaintainSequenceTransition(player) = bMaintainSequenceTransition;
	m_nClientEffects(player) = nClientEffects;
	m_bJiggleBones(player) = bJiggleBones;
	player->GetEffects() = iEffects;
	m_nLastSkipFramecount(player) = nLastSkipFramecount;
	m_nOcclusionFrame(player) = nOcclusionFrame;
	m_nOcclusionMask(player) = nOcclusionMask;

	if (player != csgo->local)
		player->SetAbsOrigin(vecAbsOrigin);

	std::memcpy(player->GetAnimOverlays(), aAnimationLayers.data(), sizeof(CAnimationLayer) * 13);

	interfaces.global_vars->curtime = flCurTime;
	interfaces.global_vars->realtime = flRealTime;
	interfaces.global_vars->frametime = flFrameTime;
	interfaces.global_vars->absoluteframetime = flAbsFrameTime;
	interfaces.global_vars->framecount = iFrameCount;
	interfaces.global_vars->tickcount = iTickCount;
	interfaces.global_vars->interpolation_amount = flInterpolation;


}

void animation::build_server_bones(IBasePlayer* player)
{
	const auto backup_occlusion_flags = player->GetOcclusionFlags();
	const auto backup_occlusion_framecount = player->GetOcclusionFramecount();

	player->GetOcclusionFlags() = 0;
	player->GetOcclusionFramecount() = 0;

	//player->GetReadableBones() = player->GetWritableBones() = 0;

	player->InvalidateBoneCache();

	player->GetEffects() |= 0x8;

	//const auto backup_bone_array = player->GetBoneArrayForWrite();
	//player->GetBoneArrayForWrite() = bones;
	


	auto ang = Math::normalize( player->GetEyeAngles());

	player->UpdateAnimationState(player->GetPlayerAnimState(), ang);
	/*
	csgo->UpdateMatrix = true;
	player->SetupBones(nullptr, -1, 0x7FF00, interfaces.global_vars->curtime);
	csgo->UpdateMatrix = false;
	*/
	BuildBones(player, bones);

	player->GetOcclusionFlags() = backup_occlusion_flags;
	player->GetOcclusionFramecount() = backup_occlusion_framecount;

	player->GetEffects() &= ~0x8;
}

void CMAnimationFix::animation_info::update_animations(animation* record, animation* from)
{
	auto Interpolate = [](const Vector from, const Vector to, const float percent) {
		return to * percent + from * (1.f - percent);
	};
	auto Interpolate2 = [](const float from, const float to, const float percent) {
		return to * percent + from * (1.f - percent);
	};

	if (!from)
	{
		// set velocity and layers.
		record->velocity = player->GetVelocity();

		// apply record.
		record->apply(player);

		// run update.
		return g_Animfix->update_player(player);
	}

	const auto new_velocity = player->GetVelocity();

	// restore old record.

	memcpy(player->GetAnimOverlays(), from->layers, sizeof(CAnimationLayer) * 13);
	//player->SetAbsOrigin(record->origin);
	//player->SetAbsAngles(from->abs_origin);
	//player->GetVelocity() = from->velocity;

	// setup velocity.
	record->velocity = new_velocity;

	// did the player shoot?

	// setup extrapolation parameters.
	auto old_origin = from->origin;
	auto old_flags = from->flags;
	if (vars.ragebot.fronttrack) {
		for (auto i = 0; i < record->lag; i++)
		{

			// move time forward.
			//const auto time = from->sim_time + TICKS_TO_TIME(i + 1);
		//	const auto lerp = 1.f - (record->sim_time - time) / (record->sim_time - from->sim_time);

			/*player->GetDuckAmount() = Interpolate2(from->duck, record->duck, lerp);*/

			// resolve player.
			if (record->lag - 1 == i)
			{
				player->GetVelocity() = new_velocity;
				player->GetFlagsPtr() = record->flags;
			}
			else // compute velocity and flags.
			{
				Extrapolate(player, old_origin, player->GetVelocity(), player->GetFlagsPtr(), old_flags & FL_ONGROUND);
				old_flags = player->GetFlags();
			}

			record->resolver = ResolverMode[player->GetIndex()];

			// run update.
			g_Animfix->update_player(player);
		}
	}
	else {
		player->GetVelocity() = new_velocity;
		player->GetFlagsPtr() = record->flags;
		record->resolver = ResolverMode[player->GetIndex()];

		// run update.
		g_Animfix->update_player(player);
	}
	if (!record->dormant && !from->dormant)
		record->didshot = record->last_shot_time > from->sim_time && record->last_shot_time <= record->sim_time;
}

void CMAnimationFix::UpdatePlayers()
{
	if (!interfaces.engine->IsInGame())
		return;

	const auto local = csgo->local;

	// erase outdated entries
	for (auto it = animation_infos.begin(); it != animation_infos.end();) {
		auto player = reinterpret_cast<IBasePlayer*>(interfaces.ent_list->GetClientEntityFromHandle(it->first));

		if (!player || player != it->second.player || !player->isAlive()
			|| !local)
		{
			if (player)
				player->GetClientSideAnims() = true;
			it = animation_infos.erase(it);
		}
		else
			it = next(it);
	}

	if (!local)
	{
		for (auto i = 1; i <= interfaces.engine->GetMaxClients(); ++i) {
			auto entity = interfaces.ent_list->GetClientEntity(i);
			if (entity && entity->IsPlayer())
				entity->GetClientSideAnims() = true;
		}
	}


	for (auto i = 1; i <= interfaces.engine->GetMaxClients(); ++i) {
		const auto entity = interfaces.ent_list->GetClientEntity(i);
		if (!entity || !entity->IsPlayer())
			continue;
		if (!entity->isAlive() || entity->IsDormant())
			continue;

		if (entity == local)
			continue;

		if (entity != local && entity->GetTeam() == local->GetTeam()) {
			csgo->EnableBones = entity->GetClientSideAnims() = true;
			continue;
		}

		if (animation_infos.find(entity->GetRefEHandle()) == animation_infos.end())
			animation_infos.insert_or_assign(entity->GetRefEHandle(), animation_info(entity, {}));
	}

	// run post update
	for (auto& info : animation_infos)
	{
		auto& _animation = info.second;
		const auto player = _animation.player;
		// erase frames out-of-range
		for (int i = 0; i < _animation.frames.size(); i++)
			if (!_animation.frames[i]->is_valid(0.45f, 0.2f))
				_animation.frames.erase(_animation.frames.begin() + i);

		resolver->Do(_animation.player);

		// have we already seen this update?
		if (player->GetSimulationTime() == player->GetOldSimulationTime())
			continue;

		// reset animstate
		if (_animation.last_spawn_time != player->GetSpawnTime())
		{
			auto state = player->GetPlayerAnimState();
			if (state)
				player->ResetAnimationState(state);

			_animation.last_spawn_time = player->GetSpawnTime();
		}

		// grab weapon
		const auto weapon = player->GetWeapon();

		// make a full backup of the player
		auto backup = new animation(player);
		backup->apply(player);

		// grab previous
		animation* previous = nullptr;

		if (!_animation.frames.empty() && !_animation.frames.front()->dormant)
			previous = _animation.frames.front();

		const auto shot = weapon && previous && weapon->LastShotTime() > previous->sim_time
			&& weapon->LastShotTime() <= player->GetSimulationTime();

		if (!shot)
			info.second.last_reliable_angle = player->GetEyeAngles();

		// store server record
		auto record = _animation.frames.emplace_front(new animation(player, info.second.last_reliable_angle));

		// run full update
		_animation.update_animations(record, previous);

		// restore correctly synced values
		backup->restore(player);

		// use uninterpolated data to generate our bone matrix
		record->build_server_bones(player);
	}
}

void CMAnimationFix::update_player(IBasePlayer* player)
{

	if (player != csgo->local) {
		static auto& enable_bone_cache_invalidation = **reinterpret_cast<bool**>(
			reinterpret_cast<uint32_t>((void*)csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"),
				"C6 05 ? ? ? ? ? 89 47 70")) + 2);

		//// make a backup of globals
		const auto backup_frametime = interfaces.global_vars->frametime;
		const auto backup_curtime = interfaces.global_vars->curtime;
		const auto old_flags = player->GetFlagsPtr();

		// get player anim state
		auto state = player->GetPlayerAnimState();

		if (state->m_last_clientside_anim_framecount == interfaces.global_vars->framecount)
			state->m_last_clientside_anim_framecount -= 1;

		// fixes for networked players
		interfaces.global_vars->frametime = interfaces.global_vars->interval_per_tick;
		interfaces.global_vars->curtime = player->GetSimulationTime();
		player->GetEFlags() &= ~4096;
		player->GetAbsVelocity() = player->GetVelocity();

		if (player->GetAnimOverlay(5)->m_weight > 0.0f)
			player->GetFlagsPtr() |= FL_ONGROUND;

		//player->InvalidatePhysicsRecursive(ANIMATION_CHANGED);

		// make sure we keep track of the original invalidation state
		const auto old_invalidation = enable_bone_cache_invalidation;

		// notify the other hooks to instruct animations and pvs fix

		player->GetClientSideAnims() = true;
		player->UpdateClientSideAnimation();
		player->GetClientSideAnims() = false;

		//player->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

		player->InvalidatePhysicsRecursive(0x2A);

		// we don't want to enable cache invalidation by accident
		enable_bone_cache_invalidation = old_invalidation;

		// restore globals
		interfaces.global_vars->curtime = backup_curtime;
		interfaces.global_vars->frametime = backup_frametime;

		player->GetFlagsPtr() = old_flags;
	}
}

CMAnimationFix::animation_info* CMAnimationFix::get_animation_info(IBasePlayer* player)
{
	auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end())
		return nullptr;

	return &info->second;
}

int get_max_tickbase_shiftS2()
{
	if (!csgo->local->isAlive())
		return 0;
	if (!CanDT())
		return csgo->game_rules->IsValveDS() ? 6 : 16;

	auto idx = csgo->local->GetWeapon()->GetItemDefinitionIndex();
	auto max_tickbase_shift = 0;

	switch (idx)
	{
	case WEAPON_M249:
	case WEAPON_MAC10:
	case WEAPON_P90:
	case WEAPON_MP5SD:
	case WEAPON_NEGEV:
	case WEAPON_MP9:
		max_tickbase_shift = 12;
		break;
	case WEAPON_ELITE:
	case WEAPON_UMP45:
	case WEAPON_BIZON:
	case WEAPON_TEC9:
	case WEAPON_MP7:
		max_tickbase_shift = 9;
		break;
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_CZ75A:
		max_tickbase_shift = 10;
		break;
	case WEAPON_FIVESEVEN:
	case WEAPON_GLOCK:
	case WEAPON_P250:
		max_tickbase_shift = 8;
	case WEAPON_SG553:
		max_tickbase_shift = 8;
		break;
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		max_tickbase_shift = 9;
		break;
	case WEAPON_DEAGLE:
		max_tickbase_shift = 12;
		break;
	case WEAPON_G3SG1:
		max_tickbase_shift = 15;
		break;
	case WEAPON_SCAR20:
		max_tickbase_shift = 15;
		break;
	case WEAPON_SSG08:
		max_tickbase_shift = 16;
		break;
	case WEAPON_AWP:
		max_tickbase_shift = 16;
		break;
	}

	if (max_tickbase_shift < 16) {
		if (vars.ragebot.BDT == 0)
		{
			max_tickbase_shift = clamp(max_tickbase_shift, 0, 13);
		}
		else if (vars.ragebot.BDT == 1)
		{
			max_tickbase_shift = clamp(max_tickbase_shift, 0, 14);
		}
		else if (vars.ragebot.BDT == 2)
		{
			max_tickbase_shift = clamp(max_tickbase_shift, 0, 15);
		}
	}
		
	


	return max_tickbase_shift;
}

bool animation::is_valid_extended()
{
	if (!interfaces.engine->GetNetChannelInfo() || !valid)
		return false;

	const auto correct = std::clamp(interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)
		+ interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING)
		+ calculate_lerp(), 0.f, 0.2f);

	float deltaTime = fabsf(correct - (interfaces.global_vars->curtime - sim_time));
	float ping = 0.2f;
	ping += vars.misc.fakeping / 1000.f;
	//if (CanDT() && csgo->dt_charged && !CMAntiAim::Get().did_shot)
	//	ping += TICKS_TO_TIME(get_max_tickbase_shiftS2());

	return deltaTime < ping&& deltaTime >= 0.f;
}

std::optional<animation*> CMAnimationFix::get_latest_animation(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it)) {
		if ((*it)->is_valid_extended()) {
			if (TIME_TO_TICKS(fabsf((*it)->sim_time - player->GetSimulationTime())) < 16 + 14)
				return &**it;
		}
	}

	return std::nullopt;
}

std::optional<animation*> CMAnimationFix::get_oldest_animation(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.rbegin(); it != info->second.frames.rend(); it = next(it)) {
		if ((*it)->is_valid_extended()) {
			return &**it;
		}
	}

	return std::nullopt;
}
std::optional<std::pair<animation*, animation*>> CMAnimationFix::get_intermediate_animations(
	IBasePlayer* player, const float range)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if ((*it)->is_valid_extended() && it + 1 != info->second.frames.end() && !((*it) + 1)->is_valid_extended())
			return std::make_pair(&*((*it) + 1), &**it);

	return std::nullopt;
}

std::vector<animation*> CMAnimationFix::get_valid_animations(IBasePlayer* player, const float range)
{
	std::vector<animation*> result;

	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second.frames.empty())
		return result;

	result.reserve(static_cast<int>(std::ceil(range * .2f / interfaces.global_vars->interval_per_tick)));

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if ((*it)->is_valid(range * .2f))
			result.push_back(&**it);

	return result;
}


std::optional<animation*> CMAnimationFix::get_latest_firing_animation(IBasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if ((*it)->is_valid_extended() && (*it)->didshot)
			return &**it;

	return std::nullopt;
}
void CMAnimationFix::UpdateFakeState()
{
	//static bool ShouldInitAnimstate = false;

	if ((!interfaces.engine->IsConnected() && !interfaces.engine->IsInGame()) || !csgo->local) {
		//ShouldInitAnimstate = false;
		return;
	}

	if (!csgo->local->isAlive() || csgo->game_rules->IsFreezeTime()) {
		//ShouldInitAnimstate = false;
		return;
	}

	static CBaseHandle* selfhandle = nullptr;
	static float spawntime = csgo->local->GetSpawnTime();

	auto alloc = FakeAnimstate == nullptr;
	auto change = !alloc && selfhandle != &csgo->local->GetRefEHandle();
	auto reset = !alloc && !change && csgo->local->GetSpawnTime() != spawntime;

	if (change) {
		memset(&FakeAnimstate, 0, sizeof(FakeAnimstate));
		selfhandle = (CBaseHandle*)&csgo->local->GetRefEHandle();
	}
	if (reset) {
		csgo->local->ResetAnimationState(FakeAnimstate);
		spawntime = csgo->local->GetSpawnTime();
	}

	if (alloc || change) {
		FakeAnimstate = reinterpret_cast<CCSGOPlayerAnimState*>(interfaces.memalloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (FakeAnimstate)
			csgo->local->CreateAnimationState(FakeAnimstate);
	}
	static std::array<float, 24> m_poses = csgo->local->m_flPoseParameter();
	CAnimationLayer backup_layers[13];
	std::memcpy(backup_layers, csgo->local->GetAnimOverlays(), sizeof(CAnimationLayer) * 13);

	if (csgo->send_packet)
	{
		std::memcpy(m_poses.data(), csgo->local->m_flPoseParameter().data(), sizeof(m_poses));
		/* FUCKING THIS GETS CALLED ON TICKS NOT ON FRAMES */
		FakeAnimstate->m_last_clientside_anim_framecount = 0;

		csgo->local->UpdateAnimationState(FakeAnimstate, csgo->FakeAngle); // update animstate
		csgo->local->SetAbsAngles(Vector(0, FakeAnimstate->m_fGoalFeetYaw, 0));
		csgo->local->SetupBones(csgo->fakematrix, 128, 0x7FF00, interfaces.global_vars->curtime);// setup matrix
		csgo->local->SetupBones(csgo->fakelag_matrix, 128, 0x7FF00, interfaces.global_vars->curtime);// setup matrix
		for (auto& i : csgo->fakematrix)
		{

			i[0][3] -= csgo->local->GetRenderOrigin().x;
			i[1][3] -= csgo->local->GetRenderOrigin().y;
			i[2][3] -= csgo->local->GetRenderOrigin().z;
		}
		std::memcpy(csgo->local->GetAnimOverlays(), csgo->local->GetAnimOverlays(), sizeof(CAnimationLayer) * 13);
		std::memcpy(csgo->local->m_flPoseParameter().data(), m_poses.data(), sizeof(m_poses));
	}
}



void CMAnimationFix::ApplyLocalPlayer() {
	if (!csgo->cmd)
		return;

	auto animstate = csgo->local->GetPlayerAnimState();
	if (!animstate)
		return;

	auto g_local = csgo->local;
	if (!g_local || !g_local->isAlive())
		return;

	static auto abs = animstate->m_fGoalFeetYaw;
	static std::array<float, 24> m_poses = g_local->m_flPoseParameter();
	static CAnimationLayer layers[13];

	/* allow the game reanimating twice in the same tick ( if failed ) */
	if (animstate->m_last_clientside_anim_framecount == interfaces.global_vars->framecount) {
		animstate->m_last_clientside_anim_framecount -= 1;
	}

	//update anim delta /* later */ 

	/* store animoverlays */
	std::memcpy(layers, g_local->GetAnimOverlays(), sizeof(CAnimationLayer) * g_local->GetNumAnimOverlays());

	/* make our client animate */
	g_local->GetClientSideAnims() = true;

	g_local->GetEFlags() &= ~4096;
	g_local->GetAbsVelocity() = g_local->GetVelocity();

	/* update info based on clientanims */
	g_local->UpdateClientSideAnimation();

	/* stop client animating let the server do the rest */
	g_local->GetClientSideAnims() = false;

	if (csgo->send_packet) {
		std::memcpy(m_poses.data(), g_local->m_flPoseParameter().data(), sizeof(m_poses));
		abs = animstate->m_fGoalFeetYaw;
	}

	/* remove model sway */
	layers[3].m_cycle = 0.0f;
	layers[10].m_weight = 0;
	layers[7].m_weight = 0;
	layers[6].m_weight = 0;
	layers[12].m_weight = 0;
	layers[7].m_cycle = 0;

	if (!(csgo->local->GetFlags() & FL_ONGROUND)) {
		layers[12].m_weight = 0.453;
	}
	if (vars.antiaim.Legs == 2) {
		m_poses[0] = 0.f;
	}

	/* fix absyaw */
	g_local->SetAbsAngles(Vector(0, abs, 0));



	/* restore */
	std::memcpy(g_local->GetAnimOverlays(), layers, sizeof(CAnimationLayer) * g_local->GetNumAnimOverlays());
	std::memcpy(g_local->m_flPoseParameter().data(), m_poses.data(), sizeof(m_poses));
}