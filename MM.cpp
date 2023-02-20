#include "Hooks.h"
#include "MM.h"
#include "Resolver.h"
#include "EnginePrediction.h"
#include "Ragebot.h"



CLegitWeaponConfig Legitbot::GetConfig() {
	if (csgo->weapon->isPistol() && vars.legitbot.weapons[weap_category_legit::Pistol].enable)
		return vars.legitbot.weapons[weap_category_legit::Pistol];
	else if (csgo->weapon->isRifle() && vars.legitbot.weapons[weap_category_legit::Rifle].enable)
		return vars.legitbot.weapons[weap_category_legit::Rifle];
	else if (csgo->weapon->IsHeavyPistol() && vars.legitbot.weapons[weap_category_legit::HeavyPistols].enable)
		return vars.legitbot.weapons[weap_category_legit::HeavyPistols];
	else if (csgo->weapon->IsAWP() && vars.legitbot.weapons[weap_category_legit::TAWP].enable)
		return vars.legitbot.weapons[weap_category_legit::TAWP];
	else if (csgo->weapon->isSniper() && vars.legitbot.weapons[weap_category_legit::Sniper].enable)
		return vars.legitbot.weapons[weap_category_legit::Sniper];
	else if (csgo->weapon->isShotgun() && vars.legitbot.weapons[weap_category_legit::SHOTGUNS].enable)
		return vars.legitbot.weapons[weap_category_legit::SHOTGUNS];
	else
		return vars.legitbot.weapons[weap_category_legit::Def];
}


void Legitbot::Reset() {
	this->AnimationCache.Clear();
	this->Cache.Clear();
	g_AutoWall.reset();
}

void Legitbot::SetAnimations(animation* anims) {
	this->AnimationCache.Entity = anims->player;
	this->AnimationCache.Origin = anims->player->GetOrigin();
	this->AnimationCache.AbsOrigin = anims->player->GetAbsOrigin();
	this->AnimationCache.Mins = anims->player->GetMins();
	this->AnimationCache.Maxs = anims->player->GetMaxs();
	this->AnimationCache.Bones = anims->player->GetBoneCache().Base();

	anims->player->GetOrigin() = anims->origin;
	anims->player->SetAbsOrigin(anims->abs_origin);
	anims->player->GetMins() = anims->obb_mins;
	anims->player->GetMaxs() = anims->obb_maxs;
	anims->player->SetBoneCache(anims->bones);
	this->AnimationCache.Stored = true;
}
void Legitbot::RestoreAnimations() {
	if (!this->AnimationCache.Stored)
		return;
	this->AnimationCache.Entity->GetOrigin() = this->AnimationCache.Origin;
	this->AnimationCache.Entity->SetAbsOrigin(this->AnimationCache.AbsOrigin);
	this->AnimationCache.Entity->GetMins() = this->AnimationCache.Mins;
	this->AnimationCache.Entity->GetMaxs() = this->AnimationCache.Maxs;
	this->AnimationCache.Entity->SetBoneCache(this->AnimationCache.Bones);
	this->AnimationCache.Clear();
}

std::vector<Vector> Legitbot::GetPoints(IBasePlayer* pBaseEntity, int iHitbox, matrix BoneMatrix[128])
{
	std::vector<Vector> vPoints;

	if (!pBaseEntity)
		return vPoints;

	studiohdr_t* pStudioModel = interfaces.models.model_info->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(0);

	if (!set)
		return vPoints;

	mstudiobbox_t* untransformedBox = set->pHitbox(iHitbox);
	if (!untransformedBox)
		return vPoints;

	Vector vecMin = { 0, 0, 0 };
	Math::VectorTransform_Wrapper(untransformedBox->bbmin, BoneMatrix[untransformedBox->bone], vecMin);

	Vector vecMax = { 0, 0, 0 };
	Math::VectorTransform_Wrapper(untransformedBox->bbmax, BoneMatrix[untransformedBox->bone], vecMax);

	float mod = untransformedBox->radius != -1.f ? untransformedBox->radius : 0.f;
	Vector max;
	Vector min;

	static float ps = 0.75f;
	
	

	Math::VectorTransform(untransformedBox->bbmax + mod, BoneMatrix[untransformedBox->bone], max);
	Math::VectorTransform(untransformedBox->bbmin - mod, BoneMatrix[untransformedBox->bone], min);

	auto center = (min + max) * 0.5f;
	if (ps <= 0.05f) {
		vPoints.push_back(center);
		return vPoints;
	}

	auto clamp_shit = [](float val, float min, float max) {
		if (val < min)
			return min;
		if (val > max)
			return max;
		return val;
	};
	Vector curAngles = Math::CalculateAngle(center, csgo->local->GetEyePosition());
	Vector forward;
	Math::AngleVectors(curAngles, forward);
	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);
	if (iHitbox == 0) {
		for (auto i = 0; i < 4; ++i)
			vPoints.push_back(center);
		vPoints[1].x += untransformedBox->radius * clamp_shit(0.f, ps - 0.2f, 0.87f); // near left ear
		vPoints[2].x -= untransformedBox->radius * clamp_shit(0.f, ps - 0.2f, 0.87f); // near right ear
		vPoints[3].z += untransformedBox->radius * ps - 0.05f; // forehead
	}
	else if (iHitbox == (int)CSGOHitboxID::Neck)
		vPoints.push_back(center);
	else if (iHitbox == (int)CSGOHitboxID::RightThigh ||
		iHitbox == (int)CSGOHitboxID::LeftThigh ||
		iHitbox == (int)CSGOHitboxID::RightShin ||
		iHitbox == (int)CSGOHitboxID::LeftShin ||
		iHitbox == (int)CSGOHitboxID::RightFoot ||
		iHitbox == (int)CSGOHitboxID::LeftFoot) {

		if (iHitbox == (int)CSGOHitboxID::RightThigh ||
			iHitbox == (int)CSGOHitboxID::LeftThigh) {
			vPoints.push_back(center);
		}
		else if (iHitbox == (int)CSGOHitboxID::RightShin ||
			iHitbox == (int)CSGOHitboxID::LeftShin) {
			vPoints.push_back(center);
		}
		else if (iHitbox == (int)CSGOHitboxID::RightFoot ||
			iHitbox == (int)CSGOHitboxID::LeftFoot) {
			vPoints.push_back(center);
			vPoints[0].z += 5.f;
		}
	}
	else {
		for (auto i = 0; i < 3; ++i)
			vPoints.push_back(center);
		vPoints[1] += right * (untransformedBox->radius * ps);
		vPoints[2] += left * (untransformedBox->radius * ps);
	}


	return vPoints;
}

std::vector<int> Legitbot::GetTargettedHitboxes()
{
	std::vector<int> hitboxes;




	if (this->Config.Hitscan & 1) {
		hitboxes.push_back((int)CSGOHitboxID::Head);
		hitboxes.push_back((int)CSGOHitboxID::Neck);
	}
	if (this->Config.Hitscan & 2) {
		hitboxes.push_back((int)CSGOHitboxID::UpperChest);
		hitboxes.push_back((int)CSGOHitboxID::Chest);
		hitboxes.push_back((int)CSGOHitboxID::LeftUpperArm);
		hitboxes.push_back((int)CSGOHitboxID::RightUpperArm);
	}
	if (this->Config.Hitscan & 4) {
		hitboxes.push_back((int)CSGOHitboxID::LowerChest);
		hitboxes.push_back((int)CSGOHitboxID::Pelvis);
		hitboxes.push_back((int)CSGOHitboxID::Stomach);
	}

	if (this->Config.Hitscan & 8)
	{
		hitboxes.push_back((int)CSGOHitboxID::LeftThigh);
		hitboxes.push_back((int)CSGOHitboxID::RightThigh);
	}

	

	return hitboxes;
}

bool Legitbot::PointVisible(Vector& point) {
	trace_t trace;
	Ray_t ray;
	ray.Init(csgo->unpred_eyepos, point);

	static CTraceFilter filter;
	filter.pSkip = csgo->local;

	interfaces.trace->TraceRay(ray, MASK_SOLID, &filter, &trace);

	return (trace.fraction == 1.f);
}

float Legitbot::Scan(IBasePlayer* target) {

	ScanCache.Clear();

	auto BestAnimation = g_Animfix->get_latest_animation(target);
	auto BestAnimationValue = BestAnimation.value();

	if (!BestAnimation.has_value() || !BestAnimationValue->player) {
		return 180.f;
	}
	memcpy(this->Cache.TargettedMatrix, BestAnimationValue->bones, sizeof(matrix[128]));

	this->SetAnimations(BestAnimationValue);

	for (auto& Hitbox : this->Cache.TargettedHitboxes) {
		auto Points = this->GetPoints(target, Hitbox, this->Cache.TargettedMatrix);
		if (this->Config.visibleonly) {
			for (auto& Point : Points) {
				if (!PointVisible(Point))
					continue;

				Vector ViewAngle = Math::CalculateAngle(csgo->unpred_eyepos, Point);
				auto FOV = Math::GetFov(this->Cache.ViewAngles, ViewAngle);

				if (FOV < this->ScanCache.BestDistance) {
					this->ScanCache.BestDistance = FOV;
					this->ScanCache.Point = ViewAngle;
				}
			}
		}
		else {
			for (auto& Point : Points) {
				if (!g_AutoWall.Think(Point, target).m_damage < this->Cache.TargettedDamage)
					continue;

				Vector ViewAngle = Math::CalculateAngle(csgo->unpred_eyepos, Point);
				auto FOV = Math::GetFov(this->Cache.ViewAngles, ViewAngle);

				if (FOV < this->ScanCache.BestDistance) {
					this->ScanCache.BestDistance = FOV;
					this->ScanCache.Point = ViewAngle;
				}
			}
		}
	}

	this->RestoreAnimations();

	return this->ScanCache.BestDistance;
}

void Legitbot::Run(CUserCmd* cmd) {

	if (!cmd || !csgo->weapon->IsGun() || csgo->weapon->IsZeus() || csgo->weapon->IsKnife())
		return;

	this->Config = this->GetConfig();

	if (!this->Config.enable)
		return;

	this->Cache.TargettedHitboxes = this->GetTargettedHitboxes();

	interfaces.engine->GetViewAngles(this->Cache.ViewAngles);

	this->Cache.TargettedDamage = this->Config.mindamage;

	if (!(cmd->buttons & IN_ATTACK && cmd->buttons & IN_ATTACK2) || !Ragebot::Get().IsAbleToShoot())
		return;

	int LocalTeam = csgo->local->GetTeam();

	for (auto i = 1; i <= interfaces.global_vars->maxClients; i++)
	{
		auto Player = interfaces.ent_list->GetClientEntity(i);
		if (!Player || Player == csgo->local || Player->IsDormant() || !Player->isAlive() || !Player->GetHealth() <= 0 || Player->GetTeam() == LocalTeam)
			continue;
		
		this->Scan(Player);

		if (this->ScanCache.BestDistance < this->Cache.BestDistance && !this->ScanCache.Point.IsZero())
		{
			this->Cache.BestDistance = this->ScanCache.BestDistance;
			this->Cache.BestPoint = this->ScanCache.Point;
		}
	}

	if (this->Cache.BestPoint.IsZero() && this->Cache.BestDistance >= 180.f)
		return;

	cmd->viewangles = Math::normalize(this->Cache.BestPoint);
	interfaces.engine->SetViewAngles(cmd->viewangles);
}