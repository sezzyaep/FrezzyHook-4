#include "Visuals.h"
#include "Autowall.h"
#include "Resolver.h"
#include "FakelagGraph.h"
#include "AntiAims.h"
#include "Ragebot.h"
#include "render.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx9.h"
#include <chrono>
#include "GUI/element.h"
#define INRANGE(x,a,b)   (x >= a && x <= b)
#define GET_BYTE( x )    (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))


std::vector<Vector> GetHitboxRadiusVecVec(IBasePlayer* pBaseEntity, int iHitbox, matrix BoneMatrix[128])
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

	float ps = 0.75f;
	if (pBaseEntity->GetVelocity().Length() > 300.f && iHitbox > 0)
		ps = 0.f;
	else {
		if (iHitbox <= (int)CSGOHitboxID::Neck)
			ps = 1.0f;
		else if (iHitbox <= (int)CSGOHitboxID::RightThigh)
			ps = 0.75f;
	}

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

void Rotate3DPoint(Vector& Point, Vector Origin, float Angle) {
	Vector2D Point2D = Vector2D(Point.x, Point.y);
	Vector2D Origin2D = Vector2D(Origin.x, Origin.y);
	Drawing::rotate_point(Point2D, Origin2D, false, Angle);
	Point.x = Point2D.x;
	Point.y = Point2D.y;
}

//VISUALS DEFINITIONS

CSingleMolotovProximityWarning::CSingleMolotovProximityWarning(IBasePlayer* BindEntity) {
	this->Entity = BindEntity;
	if (vars.visuals.MollyWarning & 1) 
	this->Update();

}
CSingleMolotovProximityWarning::~CSingleMolotovProximityWarning() {
	this->Shutdown();
}

void CSingleMolotovProximityWarning::Render(bool Timer) {
	if (vars.visuals.MollyWarning & 1) {
	static auto Center = color_t(vars.visuals.MollyColor[2],
		vars.visuals.MollyColor[1], vars.visuals.MollyColor[0], 200).u32();
	static auto Outside = color_t(vars.visuals.MollyColor[2],
		vars.visuals.MollyColor[1], vars.visuals.MollyColor[0], 80).u32();


	this->RenderablePoints.clear();
	Vector WorldToScreenBuffer = Vector(0, 0, 0);
	if (Math::WorldToScreen(this->Origin, WorldToScreenBuffer))
		this->RenderablePoints.push_back(vertex{
				WorldToScreenBuffer.x,
				WorldToScreenBuffer.y,
				0.0f,
				1.0f,
				Center
			});
	for (auto& Point : this->VirtualPoints) {
		if (Math::WorldToScreen(Point, WorldToScreenBuffer))
			this->RenderablePoints.push_back(vertex{
			WorldToScreenBuffer.x,
			WorldToScreenBuffer.y,
			0.0f,
			1.0f,
			Outside
				});
	}

	if (this->RenderablePoints.size() <= 2)
		return;


	this->RenderablePoints.push_back(this->RenderablePoints[1]);

	if (this->RenderablePoints.size() <= 3)
		return;

		auto dev = g_Render->GetDevice();

		dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);
		dev->SetTexture(0, nullptr);
		dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, this->RenderablePoints.size() - 2, this->RenderablePoints.data(), sizeof vertex);
		dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);
	}

	if (Timer) {
		CGameTrace trace;

		Ray_t ray;
		ray.Init(csgo->local->GetEyePosition(), this->Origin + Vector(0, 0, 100));

		static CTraceFilterWorldAndPropsOnly filter;

		interfaces.trace->TraceRay(ray, 0x200400B, &filter, &trace);
		
		if (trace.fraction != 1.f)
			return;

		Vector Screen;
		Math::WorldToScreen(this->Origin + Vector(0,0,100), Screen);


		/*
		g_Render->circle_filled_radial_gradient(g_Render->GetDevice(), Vector2D(P2. x, P2.y), 30, color_t(col1[0],
			col1[1], col1[2], 0), color_t(col1[0],
				col1[1], col1[2], 255));
		g_Render->CircleFilled(P2.x, P2.y, 16, color_t(23,23,23, 255), 35);
		g_Render->PArc(P2.x, P2.y, 18, 1.5f * PI, ((1.5f * PI) - ((1.5f * PI + 1.57) * (this->TimeLeft / 7.03125f))), 3.f, color_t(vars.visuals.MollyColor[0],
			vars.visuals.MollyColor[1], vars.visuals.MollyColor[2], 255));
		g_Render->DrawString(P2.x + 1, P2.y, color_t(vars.visuals.MollyColor[0],
			vars.visuals.MollyColor[1], vars.visuals.MollyColor[2],255), render::centered_x | render::centered_y, fonts::ESPIcons2, "P");
			*/

		g_Render->CircleFilled(Screen.x, Screen.y, 26 - 10,color_t(23, 23, 23, 130), 35);
		g_Render->PArc(Screen.x, Screen.y, 26 - 9, 1.5f * PI, ((1.5f * PI) - ((1.5f * PI + 1.57) * (this->TimeLeft / 7.03125f))), 1.f, color_t(165, 165, 165, 255));
		g_Render->DrawString(Screen.x + 1, Screen.y, color_t(255, 255, 255, 255), render::centered_x | render::centered_y, fonts::ESPIcons2, "P");

	}

}
void CSingleMolotovProximityWarning::Shutdown() {


	this->TimeLeft = 0;
	this->RenderablePoints.clear();
	this->VirtualPoints.clear();
	this->Range = 0.f;
	this->Origin.Zero();
}

void CSingleMolotovProximityWarning::Update() {

	

	this->TimeLeft = (((*(float*)(uintptr_t(this->Entity) + 0x20)) + 7.03125f) - interfaces.global_vars->curtime);



	int m_fireCount = this->Entity->m_fireCount();  //0x13A8

	

	bool* m_bFireIsBurning = this->Entity->m_bFireIsBurning(); //0xE94
	int* m_fireXDelta = this->Entity->m_fireXDelta(); //0x9E4
	int* m_fireYDelta = this->Entity->m_fireYDelta(); //0xB74
	int* m_fireZDelta = this->Entity->m_fireZDelta(); //0xD04
	this->Range = 0.f;
	this->VirtualPoints.clear();
	this->Origin = this->Entity->GetOrigin();


	std::vector<Vector> PreVirtualPoints;
	PreVirtualPoints.push_back(this->Origin);

	Vector Center = Vector(0, 0, 0);
	Vector CurrentFlameOrigin = Vector(0, 0, 0);
	for (int i = 0; i <= m_fireCount; i++) {
		CurrentFlameOrigin = Vector(m_fireXDelta[i], m_fireYDelta[i], m_fireZDelta[i]);
		float Range = CurrentFlameOrigin.Length2DSqr();
		if (Range > this->Range)
			this->Range = Range;

		Center += CurrentFlameOrigin;

		if (CurrentFlameOrigin.IsZero())
			continue;

		PreVirtualPoints.push_back(this->Origin + (CurrentFlameOrigin * 1.3f));
	}

	this->Range = sqrtf(this->Range) + 14.5f;

	if (m_fireCount > 1)
		this->Origin += (Center / m_fireCount);

	PreVirtualPoints.push_back(this->Origin);

	int PointCount = PreVirtualPoints.size();


	float Step = PI * 2.0f / PointCount;
	float BestRange = 0.f;
	Vector BestPoint = Vector(0,0,0);



	for (float CurrentAngle = 0.f; CurrentAngle <= PI * 2.0f; CurrentAngle += Step)
	{

		BestPoint.Zero();
		BestRange = INT_MAX;
		Vector Offset = Vector(sin(CurrentAngle), cos(CurrentAngle), 0.f) * this->Range;
		for (auto& VirtualPoint : PreVirtualPoints) {
			auto CurrentRange = (VirtualPoint - (this->Origin + Offset)).LengthSqr();
			if (CurrentRange < BestRange) {
				BestRange = CurrentRange;
				BestPoint = VirtualPoint;
			}
		}
		this->VirtualPoints.push_back(BestPoint);

	}
}

void SetupMove(Vector& move, Vector& Velocity, float frametime)
{
	static auto sv_gravity = interfaces.cvars->FindVar("sv_gravity");


	move.x = (Velocity.x) * frametime;
	move.y = (Velocity.y) * frametime;



	float gravity = sv_gravity->GetFloat() * 0.4f;

	float newZ = Velocity.z - (gravity * frametime);
	move.z = ((Velocity.z + newZ) / 2.0f) * frametime;

	Velocity.z = newZ;

}
int ClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float    backoff;
	float    change;
	float    angle;
	int        i, blocked;

	blocked = 0;

	angle = normal[2];

	if (angle > 0)
	{
		blocked |= 1;        // floor
	}
	if (!angle)
	{
		blocked |= 2;        // step
	}

	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
		{
			out[i] = 0;
		}
	}

	return blocked;
}
void TraceHull(Vector& src, Vector end, trace_t& tr)
{

	Ray_t ray;
	ray.Init(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));

	static CTraceFilterWorldAndPropsOnly filter;


	interfaces.trace->TraceRay(ray, 0x200400B, &filter, &tr);
}
void ResolveCollision(trace_t& tr, Vector& vecVelocity, float interval)
{

	// Calculate elasticity
	float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
	float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	// Calculate bounce
	Vector vecAbsVelocity;
	ClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	// Stop completely once we move too slow
	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
	if (flSpeedSqr < flMinSpeedSqr)
	{
		//vecAbsVelocity.Zero();
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	// Stop if on ground
	if (tr.plane.normal.z > 0.7f)
	{
		float speed = vecAbsVelocity.LengthSqr();


		if (speed > 96000.f) {

			static auto NormalizeVector = [](Vector vec) {
			
				Vector res = vec;
				res /= (res.Length() + std::numeric_limits< float >::epsilon());
				return res;
				
			};
			float len = vecAbsVelocity.normalized().Dot(tr.plane.normal);
			if (len > 0.5f) 
				vecAbsVelocity *= 1.5f - len;

			
		}


		if (speed < 400.f) {
			vecAbsVelocity.Zero();
		}
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval); //vecAbsVelocity.Mult((1.0f - tr.fraction) * interval);

		// Trace through world
		TraceHull(tr.endpos, tr.endpos + vecAbsVelocity, tr);
	}
	else
	{
		vecVelocity = vecAbsVelocity;
	}
}
void CheckCollision(Vector& src, const Vector& move, trace_t& tr)
{

	Vector vecAbsEnd = src;
	vecAbsEnd += move;


	TraceHull(src, vecAbsEnd, tr);
}


CSingleGrenadeProximityWarning::CSingleGrenadeProximityWarning(IBasePlayer* BindEntity, bool Trail, EGrenadeWarningType Type) {
	m_flDistance = 0.f;
	m_eType = Type;
	m_iLastTick = 0;
	m_flBlend = 1.f;
	PredictGrenade(BindEntity, Trail);
}
CSingleGrenadeProximityWarning::~CSingleGrenadeProximityWarning() {  }
void CSingleGrenadeProximityWarning::Render(float m_flAnimationSpeed, bool m_bValid) {
	if (m_bValid || this->m_eType == EGrenadeWarningType::GW_SMOKE) {
		m_flAlpha += m_flAnimationSpeed;
	}
	else {
		m_flAlpha -= m_flAnimationSpeed;
	}
	m_flAlpha = clamp(m_flAlpha, 0.f, 1.f);
	bool FirstRender = false;
	if (m_flDistance == 0.f) {
		m_flDistance = csgo->local->GetOrigin().DistTo(m_vEndPosition);
	}
	if (m_iLastTick != interfaces.global_vars->tickcount) {
		if (m_iLastTick == 0) {
			FirstRender = true;
		}
		m_iLastTick = interfaces.global_vars->tickcount;
		if (csgo->local && csgo->local->isAlive()) {
			if (m_eType == GW_HE_GRENADE || m_eType == GW_MOLOTOV) 
				m_flDistance = csgo->local->GetOrigin().DistTo(m_vEndPosition);
			
	
			CGameTrace trace;

			Ray_t ray;
			ray.Init(csgo->local->GetEyePosition(), m_vEndPosition);

			static CTraceFilterWorldAndPropsOnly filter;

			interfaces.trace->TraceRay(ray, 0x200400B, &filter, &trace);
			m_bSafe = true;
			m_bVisible = m_flDistance < 1000 || trace.fraction >= 1.0f;
			if (FirstRender && m_bVisible) {
				m_flBlend = 1.f;
			}
			else if (FirstRender) {
				m_flBlend = 0.f;
			}

			

			if (m_bVisible ) {
				m_flBlend += m_flAnimationSpeed * 2.f;
			}
			else {
				m_flBlend -= m_flAnimationSpeed *2.f;
			}

			m_flBlend = clamp(m_flBlend, 0.f, 1.f);
			switch (m_eType) {
			case GW_HE_GRENADE:

				m_bSafe = m_flDistance > 350 || trace.fraction < 0.61f;

				break;
			case GW_MOLOTOV:

				m_bSafe = m_flDistance > 200 || trace.fraction < 0.61f;

				break;
			case GW_FLASHBANG:
				m_bSafe = trace.fraction < 1.f;
				break;

			}
		

			
			m_flMolotovBlend = m_flBlend;
			
		}
		else
			m_bSafe = true;

	}

	if (!(vars.visuals.GrenadeProximityWarning & 1))
		return;

	switch (this->m_eType) {
	case EGrenadeWarningType::GW_DECOY:
		RenderDecoy();
		break;
	case EGrenadeWarningType::GW_SMOKE:
		RenderSmokeGrenade();
		break;
	case EGrenadeWarningType::GW_FLASHBANG:
		RenderFlashBang();
		break;
	case EGrenadeWarningType::GW_HE_GRENADE:
		RenderHeGrenade();
		break;
	case EGrenadeWarningType::GW_MOLOTOV:
		RenderMolotov();
		break;
	}
}

void CSingleGrenadeProximityWarning::RenderHeGrenade() {
	if (this->m_flBlend <= 0)
		return;
	Vector Screen;
	
		
	if (!Math::WorldToScreen(this->m_vEndPosition, Screen))
		return;
	
	g_Render->CircleFilled(Screen.x, Screen.y, 26 - 10, this->m_bSafe ? color_t(23, 23, 23, 130 * m_flAlpha) : color_t(168, 29, 29, 130 * m_flAlpha), 35);
	g_Render->PArc(Screen.x, Screen.y, 26 - 9, 0, 2.f * PI, 1.f, color_t(165, 165, 165, 255 * m_flAlpha));
	g_Render->DrawString(Screen.x + 1, Screen.y, color_t(255, 255, 255, 255 * m_flAlpha), render::centered_x | render::centered_y, fonts::ESPIcons2, "W");

}
void CSingleGrenadeProximityWarning::RenderSmokeGrenade() {
	if (this->m_flBlend <= 0)
		return;
	static auto FilledCircle = [](Vector location, float radius, color_t Col, float alpha) {



		g_Render->Gradient3DCircleOp(g_Render->GetDevice(), location, radius, color_t(Col[0],
			Col[1], Col[2], 0), color_t(Col[0],
				Col[1], Col[2],  120 * alpha));

	};
	FilledCircle(this->m_vEndPosition, 145 * this->m_flAlpha * this->m_flBlend, color_t(255, 255, 255, 255), this->m_flAlpha * this->m_flBlend);
}
void CSingleGrenadeProximityWarning::RenderFlashBang() {
	if (!this->m_bVisible)
		return;
	Vector Screen;
	if (!Math::WorldToScreen(this->m_vEndPosition, Screen))
		return;

	g_Render->circle_filled_radial_gradient(g_Render->GetDevice(), Vector2D(Screen.x, Screen.y), 20, color_t(255,255,255, 0), color_t(255,255,255, 255));
}
void CSingleGrenadeProximityWarning::RenderDecoy() {
	


	
}
void CSingleGrenadeProximityWarning::RenderMolotov() {

	if (vars.visuals.GrenadeProximityWarning & 1) {
		Vector Screen;
		if (this->m_bWillLand) {
			if (!Math::WorldToScreen(this->m_vEndPosition + Vector(0, 0, 100), Screen))
				return;
		
		}
		else {
			if(!Math::WorldToScreen(this->m_vEndPosition, Screen))
				return;
		}
		g_Render->CircleFilled(Screen.x, Screen.y, 26 - 10, this->m_bSafe ? color_t(23, 23, 23, 130 * m_flMolotovBlend * m_flAlpha) : color_t(168, 29, 29, 130 * m_flMolotovBlend * m_flAlpha), 35);
		g_Render->PArc(Screen.x, Screen.y, 26 - 9, 0, 2.f * PI, 1.f, color_t(165, 165, 165, 255 * m_flMolotovBlend * m_flAlpha));
		g_Render->DrawString(Screen.x + 1, Screen.y, color_t(255,255,255, 255 * m_flMolotovBlend * m_flAlpha), render::centered_x | render::centered_y, fonts::ESPIcons2, "P");
		
	}
	if (this->m_flBlend <= 0)
		return;
	if (this->m_bWillLand) {
		static auto FilledCircle = [](Vector location, float radius, color_t Col,float alpha) {

			
		
			g_Render->Gradient3DCircleOp(g_Render->GetDevice(), location, radius, color_t(Col[0],
				Col[1], Col[2], 0), color_t(Col[0],
					Col[1], Col[2], (Col[3] / 255.f) * 150 * alpha));

		};

		FilledCircle(this->m_vEndPosition, 125 * this->m_flAlpha * this->m_flBlend, vars.visuals.GrenadeColor, this->m_flAlpha * this->m_flBlend);
	}
}
void CSingleGrenadeProximityWarning::PredictGrenade(IBasePlayer* Entity, bool Trail) {
	std::vector<std::pair<Vector, float>> m_Trail;
	trace_t tr;
	Vector Origin = Entity->GetAbsOrigin();
	Vector Current = Origin;
	Vector velocity = Entity->GetVelocity();
	Vector Move;
	auto interval = interfaces.global_vars->interval_per_tick;
	this->m_bWillLand = false;
	static auto DetonateCvar = interfaces.cvars->FindVar("molotov_throw_detonate_time");

	float DetonateMolotov = 1.5f;
	if (DetonateCvar) {
		DetonateMolotov = DetonateCvar->GetFloat();
	}
	switch (this->m_eType) {
	case EGrenadeWarningType::GW_DECOY:
	case EGrenadeWarningType::GW_SMOKE:
		for (unsigned int i = 0; i < 2000; ++i)
		{
			SetupMove(Move, velocity, interval);
			CheckCollision(Current, Move, tr);




			if (tr.fraction != 1.0f)
			{
				ResolveCollision(tr, velocity, interval);
			}
			Current = tr.endpos;
			m_Trail.push_back(std::make_pair(Current, TICKS_TO_TIME(i)));

			if (velocity.Length2D() < 0.1f)
			{

				int det_tick_mod = static_cast<int>(0.2f / interval);
				if (!(i % det_tick_mod)) {
					break;
				}
			}


		}
		break;
	case EGrenadeWarningType::GW_FLASHBANG:
	case EGrenadeWarningType::GW_HE_GRENADE:
		for (unsigned int i = 0; i < 2000; ++i)
		{
			SetupMove(Move, velocity, interval);
			CheckCollision(Current, Move, tr);

			if (!(i % static_cast<int>(0.2f / interval))) {
				if (TICKS_TO_TIME(i) > 1.5f) {
					
					break;
				}
			}

			if (tr.fraction != 1.0f)
			{
				ResolveCollision(tr, velocity, interval);
			}
			Current = tr.endpos;
		
				m_Trail.push_back(std::make_pair(Current, TICKS_TO_TIME(i)));

			

		}
		
		break;
	case EGrenadeWarningType::GW_MOLOTOV:
		for (unsigned int i = 0; i < 2000; ++i)
		{
			SetupMove(Move, velocity, interval);
			CheckCollision(Current, Move, tr);


			if (!(i % static_cast<int>(0.2f / interval))) {
				if (TICKS_TO_TIME(i) > DetonateMolotov) {
					velocity.Zero();
					CheckCollision(Current, Vector(0,0,-200), tr);
					if (tr.fraction != 1.0f) {
						Current = tr.endpos;
						this->m_bWillLand = true;
						m_Trail.push_back(std::make_pair(Current, TICKS_TO_TIME(i)));
					}
					break;
				}
			}

			if (tr.fraction != 1.0f)
			{
				if (tr.plane.normal.z > 0.7f) {
					velocity.Zero();
					CheckCollision(Current, Vector(0, 0, -200), tr);
					if (tr.fraction != 1.0f) {
						this->m_bWillLand = true;
						Current = tr.endpos;
						m_Trail.push_back(std::make_pair(Current, TICKS_TO_TIME(i)));
					}
					break;
				}
				ResolveCollision(tr, velocity, interval);
			}
			Current = tr.endpos;
				m_Trail.push_back(std::make_pair(Current, TICKS_TO_TIME(i)));

			

		}
		break;
	}
	if (Trail) {
		static std::string PSprite = "sprites/laserbeam.vmt";//laserbeam
		CacheModel(PSprite.c_str());
	
		if (this->m_eType != EGrenadeWarningType::GW_DECOY) {
			Vector Last = Origin;

			for (auto& P : m_Trail) {
				
				if (P.second <= 0.f)
					continue;
				BeamInfo_t beamInfo;
				beamInfo.m_nType = TE_BEAMPOINTS;
				beamInfo.m_pszModelName = PSprite.c_str();
				beamInfo.m_nModelIndex = -1;
				beamInfo.m_flHaloScale = 0.0f;
				beamInfo.m_flLife = P.second; //duration of tracers
				beamInfo.m_flWidth = 1; //start width
				beamInfo.m_flEndWidth = 1; //end width
				beamInfo.m_flFadeLength = 0.0f;
				beamInfo.m_flAmplitude = 2;
				beamInfo.m_flBrightness = vars.visuals.GrenadeColor[3];
				beamInfo.m_flSpeed = 0.1f;
				beamInfo.m_nStartFrame = 0;
				beamInfo.m_flFrameRate = 0.f;
				beamInfo.m_flRed = vars.visuals.GrenadeColor[0];
				beamInfo.m_flGreen = vars.visuals.GrenadeColor[1];
				beamInfo.m_flBlue = vars.visuals.GrenadeColor[2];
				beamInfo.m_nSegments = 2;
				beamInfo.m_bRenderable = true;
				beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
				beamInfo.m_vecStart = Last;
				beamInfo.m_vecEnd = P.first;

				Beam_t* beam = interfaces.beams->CreateBeamPoints(beamInfo);
				if (beam) {
					interfaces.beams->DrawBeam(beam);
				}
				Last = P.first;
			}

		}
	}
	this->m_vEndPosition = Current;

	
}

void VSTraceLine(Vector start, Vector end, unsigned int mask, IBasePlayer* ignore, trace_t* trace)
{
	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = ignore;

	interfaces.trace->TraceRay(ray, mask, &filter, trace);
}


bool GetBox(IBasePlayer* entity, int& x, int& y, int& w, int& h, Vector origin)
{
	if (entity->GetClientClass() && entity->GetClientClass()->m_ClassID == ClassId->CCSPlayer) {
		auto min = entity->GetCollideable()->OBBMins();
		auto max = entity->GetCollideable()->OBBMaxs();

		Vector dir, vF, vR, vU;

		interfaces.engine->GetViewAngles(dir);
		interfaces.engine->GetNetChannelInfo()->GetAddress();
		dir.x = 0;
		dir.z = 0;
		//dir.Normalize();
		//printf("%.1f\n", dir.y);
		Math::AngleVectors(dir, &vF, &vR, &vU);

		auto zh = vU * max.z + vF * max.y + vR * min.x; // = Front left front
		auto e = vU * max.z + vF * max.y + vR * max.x; //  = Front right front
		auto d = vU * max.z + vF * min.y + vR * min.x; //  = Front left back
		auto c = vU * max.z + vF * min.y + vR * max.x; //  = Front right back

		auto g = vU * min.z + vF * max.y + vR * min.x; //  = Bottom left front
		auto f = vU * min.z + vF * max.y + vR * max.x; //  = Bottom right front
		auto a = vU * min.z + vF * min.y + vR * min.x; //  = Bottom left back
		auto b = vU * min.z + vF * min.y + vR * max.x; //  = Bottom right back*-

		Vector pointList[] = {
			a,
			b,
			c,
			d,
			e,
			f,
			g,
			zh,
		};

		Vector transformed[ARRAYSIZE(pointList)];

		for (int i = 0; i < ARRAYSIZE(pointList); i++)
		{
			pointList[i] += origin;

			if (!Math::WorldToScreen2(pointList[i], transformed[i]))
				return false;
		}

		float left = FLT_MAX;
		float top = -FLT_MAX;
		float right = -FLT_MAX;
		float bottom = FLT_MAX;
		for (int i = 0; i < ARRAYSIZE(pointList); i++) {
			if (left > transformed[i].x)
				left = transformed[i].x;
			if (top < transformed[i].y)
				top = transformed[i].y;
			if (right < transformed[i].x)
				right = transformed[i].x;
			if (bottom > transformed[i].y)
				bottom = transformed[i].y;
		}

		x = left;
		y = bottom - 1;
		w = right - left;
		h = top - bottom + 4;

		return true;
	}
	else
	{
		Vector vOrigin, min, max, flb, brt, blb, frt, frb, brb, blt, flt;
		//float left, top, right, bottom;

		auto collideable = entity->GetCollideable();

		if (!collideable)
			return false;

		min = collideable->OBBMins();
		max = collideable->OBBMaxs();

		auto& trans = entity->GetrgflCoordinateFrame();

		Vector points[] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector pointsTransformed[8];
		for (int i = 0; i < 8; i++) {
			Math::VectorTransform(points[i], trans, pointsTransformed[i]);
		}

		Vector pos = entity->GetRenderOrigin();

		if (!Math::WorldToScreen(pointsTransformed[3], flb) || !Math::WorldToScreen(pointsTransformed[5], brt)
			|| !Math::WorldToScreen(pointsTransformed[0], blb) || !Math::WorldToScreen(pointsTransformed[4], frt)
			|| !Math::WorldToScreen(pointsTransformed[2], frb) || !Math::WorldToScreen(pointsTransformed[1], brb)
			|| !Math::WorldToScreen(pointsTransformed[6], blt) || !Math::WorldToScreen(pointsTransformed[7], flt))
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
		//+1 for each cuz of borders at the original box
		float left = flb.x;        // left
		float top = flb.y;        // top
		float right = flb.x;    // right
		float bottom = flb.y;    // bottom

		for (int i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (bottom < arr[i].y)
				bottom = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (top > arr[i].y)
				top = arr[i].y;
		}

		x = (int)left;
		y = (int)top;
		w = (int)(right - left);
		h = (int)(bottom - top);

		return true;
	}
	return false;
}

const char* GetIconChar(IBaseCombatWeapon* weapon) {
	bool defaultt = false;
	switch (weapon->GetItemDefinitionIndex()) {
	default:
		defaultt = true;
		break;
	case WEAPON_AK47:
		return "2";
		break;
	case WEAPON_AUG:
		return "b";
		break;
	case weapon_revolver:
		return "p";
		break;
	case WEAPON_KNIFE_BAYONET:
		return "h";
	case WEAPON_KNIFE_M9_BAYONET:
		return "m";
	case WEAPON_KNIFE_FLIP:
		return "f";
	case WEAPON_KNIFE_GUT:
		return "g";
	case WEAPON_KNIFE_KARAMBIT:
		return "l";
	case WEAPON_KNIFE_FALCHION:
		return "f";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "h";
	case WEAPON_KNIFE_BUTTERFLY:
		return "1";
	case WEAPON_KNIFE_WIDOWMAKER:
		return "r";
	case WEAPON_KNIFE_URSUS:
		return "g";
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		return "g";
	case WEAPON_KNIFE_STILETTO:
		return "g";
	case WEAPON_KNIFE_SKELETON:
		return "g";
	case WEAPON_KNIFE_CSS:
		return "z";
	case WEAPON_AWP:
		return "e";
		break;
	case WEAPON_BIZON:
		return "k";
		break;
	case WEAPON_C4:
		return "n";
		break;
	case WEAPON_CZ75A:
		return "q";
		break;
	case WEAPON_DEAGLE:
		return "s";
		break;
	case WEAPON_DECOY:
		return "u";
		break;
	case WEAPON_ELITE:
		return "v";
		break;
	case WEAPON_FAMAS:
		return "B";
		break;
	case WEAPON_FIVESEVEN:
		return "F";
		break;
	case WEAPON_FLASHBANG:
		return "I";
		break;
	case WEAPON_G3SG1:
		return "L";
		break;
	case WEAPON_GALILAR:
		return "O";
		break;
	case WEAPON_GLOCK:
		return "R";
		break;
	case WEAPON_INCGRENADE:
		return "X";
		break;
	case WEAPON_MOLOTOV:
		return "P";
		break;
	case WEAPON_SSG08:
		return "G";
		break;
	case WEAPON_HEGRENADE:
		return "W";
		break;
	case WEAPON_M249:
		return "E";
		break;
	case WEAPON_M4A1:
		return "A";
		break;
	case WEAPON_MAC10:
		return "J";
		break;
	case WEAPON_MP5SD:
		return "?";
		break;
	case WEAPON_MAG7:
		return "K";
		break;
	case WEAPON_MP7:
		return "Q";
		break;
	case WEAPON_MP9:
		return "c";
		break;
	case WEAPON_NOVA:
		return "i";
		break;
	case WEAPON_NEGEV:
		return "d";
		break;
	case WEAPON_P250:
		return "U";
		break;
	case WEAPON_P90:
		return "k";
		break;
	case WEAPON_SAWEDOFF:
		return "t";
		break;
	case WEAPON_SCAR20:
		return "x";
		break;
	case WEAPON_SMOKEGRENADE:
		return "C";
		break;
	case WEAPON_SG553:
		return "w";
		break;
	case WEAPON_TEC9:
		return "M";
		break;
	case WEAPON_HKP2000:
		return "o";
		break;
	case WEAPON_USP_SILENCER:
		return "S";
		break;
	case WEAPON_UMP45:
		return "N";
		break;
	case WEAPON_XM1014:
		return "T";
		break;
	case WEAPON_TASER:
		return "H";
		break;
	case WEAPON_M4A1_SILENCER:
		return "D";
	case WEAPON_KNIFE:
		return "3";
	}
	if (defaultt && weapon->IsKnife()) {
		return "3";
	}
	else
	{
		return "?";
	}
}

void CVisuals::DrawAngleLine(Vector origin, float angle, color_t color)
{
	Vector src, dst, sc1, sc2, forward;

	src = origin;
	Math::AngleVectors(Vector(0, angle, 0), &forward);
	if (Math::WorldToScreen(src, sc1) && Math::WorldToScreen(src + (forward * 40), sc2))
	{
		Drawing::DrawLine(sc1.x, sc1.y, sc2.x, sc2.y, color);
		//g_Render->DrawLine(sc1.x, sc1.y, sc2.x, sc2.y, color, 1.f);
	}
}

int ClampInt(int Value, int Min, int Max)
{
	int Return = Value;
	if (Value < Min)
		Return = Min;
	if (Value > Max)
		Return = Max;
	return(Return);
}

float ClampFloat(float Value, float Min, float Max)
{
	float Return = Value;
	if (Value < Min)
		Return = Min;
	if (Value > Max)
		Return = Max;
	return(Return);
}

double ClampDouble(double Value, int Min, int Max)
{
	double Return = Value;
	if (Value < Min)
		Return = Min;
	if (Value > Max)
		Return = Max;
	return(Return);
}

int get_max_tickbase_shiftS()
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

static uintptr_t FindSignature(const char* szModule, const char* szSignature)
{
	const char* pat = szSignature;
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(szModule);
	MODULEINFO miModInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GET_BYTE(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;
		}
		else
		{
			pat = szSignature;
			firstMatch = 0;
		}
	}
	return NULL;
}

int BiggestInArray(int Input[], int legth)
{
	int Temp = 0;
	for (int i = 0; i < legth; i++)
	{
		if (Input[i] > Temp)
		{
			Temp = Input[i];
		}
	}
	return(Temp);
}

void LoadNamedSky(const char* sky_name)
{
	static auto fnLoadNamedSkys = (void(__fastcall*)(const char*))FindSignature("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45");
	fnLoadNamedSkys(sky_name);
	csgo->SkyCheckk = false;
}
CWeaponConfig CurWepSet() {
	if (csgo->weapon->IsAuto() && vars.ragebot.weapon[weap_type::scar].enable)
		return vars.ragebot.weapon[weap_type::scar];
	else if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_SSG08 && vars.ragebot.weapon[weap_type::scout].enable)
		return vars.ragebot.weapon[weap_type::scout];
	else if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_AWP && vars.ragebot.weapon[weap_type::_awp].enable)
		return vars.ragebot.weapon[weap_type::_awp];
	else if (csgo->weapon->isRifle() && vars.ragebot.weapon[weap_type::rifles].enable)
		return vars.ragebot.weapon[weap_type::rifles];
	else if (csgo->weapon->IsHeavyPistol() && vars.ragebot.weapon[weap_type::heavy_pistols].enable)
		return vars.ragebot.weapon[weap_type::heavy_pistols];
	else if (csgo->weapon->isPistol() && vars.ragebot.weapon[weap_type::pistols].enable)
		return vars.ragebot.weapon[weap_type::pistols];
	else
		return vars.ragebot.weapon[weap_type::def];
}

void CVisuals::DrawLocalShit(IDirect3DDevice9* pDevice)
{
	if (!csgo->NoUser) {
		static bool LastPing = csgo->ConnectedToInternet;
		if (csgo->ConnectedToInternet != LastPing) {
			LastPing = csgo->ConnectedToInternet;
			PNotify("Cloud", LastPing ? "Connected to Cloud" : "Disconnected from Cloud", 4);
			PNotify("Frezzyterror", "Fixed for the last update amk", 20);
		}
	}
	int cnotifyw = csgo->w - 14;
	for (int i = cnotify.size() - 1; i >= 0; i--)
	{

		if (cnotify[i].YModF <= -40) {
			cnotify[i].YModF = 100 + i * 72;
		}
		else
		{
			cnotify[i].YModF += ((100 + i * 72) - cnotify[i].YModF) * 0.06f;
		}

		if (interfaces.global_vars->realtime >= cnotify[i].TimeToEnd) {
			cnotify[i].XmodF = clamp(cnotify[i].XmodF + clamp((abs(0 - cnotify[i].XmodF) * 0.06f), 0.5f, 500.f), 0.f, 400.f);
		}
		else
		{
			cnotify[i].XmodF = clamp(cnotify[i].XmodF - clamp((abs(0 - cnotify[i].XmodF) * 0.06f), 0.5f, 500.f), 0.f, 400.f);
		}
		
		if (cnotify[i].useicon) {
			g_Render->FilledRect(cnotifyw + cnotify[i].XmodF - cnotify[i].Width - 45, cnotify[i].YModF, cnotify[i].Width + 45, 55, vars.movable.colormode == 2 ? color_t(4, 4, 4, cnotify[i].alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 22, cnotify[i].alpha) : color_t(0, 0, 34, cnotify[i].alpha), 7.f);

			g_Render->DrawString(cnotifyw + cnotify[i].XmodF - cnotify[i].Width + 10, cnotify[i].YModF + 4 + 3, color_t(255, 255, 255, cnotify[i].alpha), render::none, fonts::NotifyBold, cnotify[i].Owner.c_str());
			g_Render->DrawString(cnotifyw + cnotify[i].XmodF - cnotify[i].Width + 10, cnotify[i].YModF + 4 + 25 - 1, color_t(255, 255, 255, cnotify[i].alpha), render::none, fonts::EventLog, cnotify[i].Text.c_str());
			g_Render->DrawString(cnotifyw + cnotify[i].XmodF - cnotify[i].Width - 35 / 2, cnotify[i].YModF + 55 / 2, color_t(255, 255, 255, cnotify[i].alpha), render::centered_x | render::centered_y, cnotify[i].icon > 4 ? fonts::NotifyMore : fonts::NewIconsNotify2, cnotify[i].icon == 1 ? "B" : cnotify[i].icon == 2 ? "C" : cnotify[i].icon == 3 ? "D" : cnotify[i].icon == 5 ? "A" : cnotify[i].icon == 6 ? "B" : "E");
		}
		else {
			g_Render->FilledRect(cnotifyw + cnotify[i].XmodF - cnotify[i].Width, cnotify[i].YModF, cnotify[i].Width, 55, vars.movable.colormode == 2 ? color_t(4, 4, 4, cnotify[i].alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 22, cnotify[i].alpha) : color_t(0, 0, 34, cnotify[i].alpha), 7.f);
			
				g_Render->DrawString(cnotifyw + cnotify[i].XmodF - cnotify[i].Width + 10, cnotify[i].YModF + 4 + 3, color_t(255, 255, 255, cnotify[i].alpha), render::none, fonts::NotifyBold, cnotify[i].Owner.c_str());
				g_Render->DrawString(cnotifyw + cnotify[i].XmodF - cnotify[i].Width + 10, cnotify[i].YModF + 4 + 25 - 1, color_t(255, 255, 255, cnotify[i].alpha), render::none, fonts::EventLog, cnotify[i].Text.c_str());
			
		}



		if (cnotify[i].XmodF >= 180) {
			cnotify.erase(cnotify.begin() + i);
		}
	}
	if (vars.visuals.eventlog) {

		for (int i = cmessages.size() - 1; i >= 0; i--)
		{
			if (interfaces.global_vars->realtime >= cmessages[i].TimeToEnd) {

				cmessages[i].Xmodf = clamp(cmessages[i].Xmodf + clamp((abs(0 - cmessages[i].Xmodf) * 0.06f), 0.5f, 500.f), 0.f, 400.f);
				cmessages[i].Alpha -= 3.f;
				if (cmessages[i].Alpha < 0) {
					cmessages[i].Alpha = 0;
				}

			}
			else
			{
				if (cmessages[i].Alpha < 255) {
					cmessages[i].Alpha += 17.f;
				}
				else {
					cmessages[i].Alpha = 255;
				}
				cmessages[i].Xmodf = clamp(cmessages[i].Xmodf - clamp((abs(0 - cmessages[i].Xmodf) * 0.06f), 0.5f, 500.f), 0.f, 400.f);
			}

			if (cmessages[i].YModF <= -40) {
				cmessages[i].YModF = i * 18 + 5;
			}
			else
			{
				cmessages[i].YModF += ((i * 18 + 5) - cmessages[i].YModF) * 0.06f;
			}

			g_Render->DrawString(5 - cmessages[i].Xmodf, cmessages[i].YModF, color_t(135, 155, 255, cmessages[i].Alpha), render::none, fonts::EventLog, "[ FREZZY ]");

			g_Render->DrawString(76 - cmessages[i].Xmodf, cmessages[i].YModF, color_t(255, 255, 255,
				(vars.visuals.eventlog_color[3] / 255) * cmessages[i].Alpha), render::none, fonts::EventLog, cmessages[i].Message.c_str());

			if (cmessages[i].Alpha <= 0 || cmessages[i].Xmodf >= 300)
				cmessages.erase(cmessages.begin() + i);
		}

	}

	if (csgo->local->isAlive() && csgo->weapon)
	{
		
		if (vars.antiaim.Legs == 2) {
			auto p = (rand() % 16);
			if (p == 0) {
				csgo->need_recharge_rn = false;
			}
			else if (p <= 4) {
				csgo->need_recharge_rn = true;
			}
			else if (p >= 5) {
				csgo->need_recharge_rn = false;	
			}

		}
	//	csgo->local->DrawServerHitboxes();
		static bool LastAntiExploit = false;
		if (vars.ragebot.FullTP != LastAntiExploit) {
			LastAntiExploit = vars.ragebot.FullTP;
			int CurrentTeam = csgo->local->GetTeam();
			interfaces.engine->ClientCmd_Unrestricted("jointeam 1", 0);
			if (LastAntiExploit) {
				interfaces.engine->ClientCmd_Unrestricted("cl_lagcompensation 0", 0);
			}
			else
			{
				interfaces.engine->ClientCmd_Unrestricted("cl_lagcompensation 1", 0);
			}
			std::string Final = "jointeam " + std::to_string(CurrentTeam);
			interfaces.engine->ClientCmd_Unrestricted(Final.c_str(), 0);
				static auto CLLAG = interfaces.cvars->FindVar("cl_lagcompensation");
				CLLAG->SetValue((LastAntiExploit ? 0 : 1));
			
		}
		if (vars.misc.AutoPeek->active) {
			if (csgo->AutoPeek.Position.IsZero()) {
				csgo->AutoPeek.Position = csgo->local->GetAbsOrigin();
			}
			if (!csgo->AutoPeek.Position.IsZero()) {
				g_Render->Gradient3DCircleOp(pDevice, csgo->AutoPeek.Position, 23.5f, color_t(vars.misc.AutoPeekCol[0], vars.misc.AutoPeekCol[1], vars.misc.AutoPeekCol[2], 0), color_t(vars.misc.AutoPeekCol[0], vars.misc.AutoPeekCol[1], vars.misc.AutoPeekCol[2], (vars.misc.AutoPeekCol[3] / 255) * 200));
			}
		}
		else if (!csgo->AutoPeek.Position.IsZero()) {
			csgo->AutoPeek.Position.Zero();
		}

		if (vars.visuals.BombT & 16) {
			csgo->PDBomb.Planted = false;
	
		}
		if (vars.visuals.BombT & 32) {
			csgo->PDBomb.IsDefusing = false;
		}
		if (vars.visuals.localmemes & 1) {
			static auto DLight = interfaces.effects->CL_AllocDlight(csgo->local->EntIndex());
			if (DLight) {
				DLight->origin = csgo->local->GetOrigin();
				DLight->flags = 0x2;
				DLight->style = 5;
				DLight->key = csgo->local->EntIndex();
				DLight->die = interfaces.global_vars->curtime + 60.f;
				ColorRGBExp32 color;
				color.r = vars.visuals.localmemecolor[0];
				color.g = vars.visuals.localmemecolor[1];
				color.b = vars.visuals.localmemecolor[2];
				color.exponent = (vars.visuals.localmemecolor[3] / 255) * 5;
				DLight->radius = 275;
				DLight->decay = 300;
				DLight->m_Direction = csgo->local->GetOrigin();
				DLight->color = color;
			}
		}
		if (vars.visuals.localmemes & 2) {
			static dlight_p DLights[65];
			for (int i = 0; i < interfaces.engine->GetMaxClients(); i++) {
				auto cplayer = interfaces.ent_list->GetClientEntity(i);
				if (!cplayer->isAlive() || cplayer->IsDormant() || cplayer == csgo->local || cplayer->GetTeam() == csgo->local->GetTeam())
					continue;

				if (!DLights[i].light)
					DLights[i].light = interfaces.effects->CL_AllocDlight(i);

				DLights[i].light->origin = cplayer->GetOrigin();
				DLights[i].light->flags = 0x2;
				DLights[i].light->style = 5;
				DLights[i].light->key = cplayer->EntIndex();
				DLights[i].light->die = interfaces.global_vars->curtime + 60.f;
				ColorRGBExp32 color;
				color.r = vars.visuals.localmemecolor[0];
				color.g = vars.visuals.localmemecolor[1];
				color.b = vars.visuals.localmemecolor[2];
				color.exponent = (vars.visuals.localmemecolor[3] / 255) * 5;
				DLights[i].light->radius = 275;
				DLights[i].light->decay = 300;
				DLights[i].light->m_Direction = cplayer->GetOrigin();
				DLights[i].light->color = color;
			}
		}
		static auto matlightr = interfaces.cvars->FindVar("mat_ambient_light_r");
		static auto matlightg = interfaces.cvars->FindVar("mat_ambient_light_g");
		static auto matlightb = interfaces.cvars->FindVar("mat_ambient_light_b");
		static bool ambientreset = false;
		static auto modelam = interfaces.cvars->FindVar("r_modelAmbientMin");
		static int oldmodellight = 0;
		if (vars.visuals.modellight != oldmodellight) {
			modelam->SetValue(vars.visuals.modellight == 0 ? 0.f : 1.0f);
			oldmodellight = vars.visuals.modellight;
		}
		if (vars.visuals.ambientlight > 0) {
			if (!ambientreset)
				ambientreset = true;
			if (vars.visuals.ambientlight == 1) {
				if (matlightr->GetFloat() != 0.129f) {
					matlightr->SetValue(0.129f);
				}
				if (matlightg->GetFloat() != 0.129f) {
					matlightg->SetValue(0.129f);
				}
				if (matlightb->GetFloat() != 0.129f) {
					matlightb->SetValue(0.129f);
				}
			}
			else {
				if (matlightr->GetFloat() != vars.visuals.ambient_color[0] / 255.f) {
					matlightr->SetValue(vars.visuals.ambient_color[0] / 255.f);
				}

				if (matlightg->GetFloat() != vars.visuals.ambient_color[1] / 255.f) {
					matlightg->SetValue(vars.visuals.ambient_color[1] / 255.f);
				}
				if (matlightb->GetFloat() != vars.visuals.ambient_color[2] / 255.f) {
					matlightb->SetValue(vars.visuals.ambient_color[2] / 255.f);
				}
			}
			
		}
		else if (ambientreset) {
			matlightr->SetValue(0.f);


			matlightg->SetValue(0.f);


			matlightb->SetValue(0.f);
		}
		if (vars.visuals.shot_multipoint) {

			for (int i = 0; i < interfaces.engine->GetMaxClients(); i++) {
				auto cplayer = interfaces.ent_list->GetClientEntity(i);
				if (!cplayer || cplayer->IsDormant() || !cplayer->isAlive() || (cplayer->GetTeam() == csgo->local->GetTeam()))
					continue;

			
				auto hitboxes = Ragebot::Get().GetHitboxesToScan(cplayer);

				auto af = g_Animfix->get_latest_animation(cplayer);
				if (af.has_value()) {
					for (auto i : hitboxes) {
						Vector world;
						Vector Center;
						auto point = Ragebot::Get().GetPoint(cplayer, i, af.value()->bones);
						if (!Math::WorldToScreen(point, Center)) {
							continue;
						}
						float MostDistance = -5;
						if (vars.visuals.galaxy) {
							for (auto& m : Ragebot::Get().GetMultipoints(cplayer, i, af.value()->bones)) {
								if (Math::WorldToScreen(m, world)) {
									float dist = (Center - world).Length2DSqr();
									if (dist > MostDistance) {
										MostDistance = dist;
									}
								}
							}
						}
						else {
							for (auto& m : GetHitboxRadiusVecVec(cplayer, i, af.value()->bones)) {
								if (Math::WorldToScreen(m, world)) {
									float dist = (Center - world).Length2DSqr();
									if (dist > MostDistance) {
										MostDistance = dist;
									}
								}
							}
						}
						if (MostDistance <= 0)
							continue;

						MostDistance = sqrtf(MostDistance);
						MostDistance *= 1.05f;

						
						
							g_Render->circle_filled_radial_gradient(pDevice, Vector2D(Center.x, Center.y), MostDistance, color_t(vars.visuals.aimbot_color[2],
								vars.visuals.aimbot_color[1],
								vars.visuals.aimbot_color[0], 0), color_t(vars.visuals.aimbot_color[2],
									vars.visuals.aimbot_color[1],
									vars.visuals.aimbot_color[0], 255));
						

					}
				}

			}
		}
		bool NadeWarning = vars.visuals.GrenadeProximityWarning & 1;
		bool trail = vars.visuals.GrenadeProximityWarning & 2;


		bool MolotovSpread = vars.visuals.MollyWarning & 1;
		bool MolotovTimer = vars.visuals.MollyWarning & 2;

		float animspeed = (interfaces.global_vars->interval_per_tick) * 2.5f;
		static float PAlpha;
			for (auto i = 64; i < interfaces.ent_list->GetHighestEntityIndex(); i++)
			{
				IBasePlayer* entity = interfaces.ent_list->GetClientEntity(i);
				if (!entity || entity->IsPlayer() || entity->IsDormant())
					continue;
				auto class_id = entity->GetClientClass()->m_ClassID;

				if (class_id == (int)9) {
					if (const auto model = entity->GetModel(); model && strstr(model->name, "flashbang")) {
						auto handle = entity->GetRefEHandle();
						auto it = GrenadeProximityWarnings.find(handle);
						auto vel = entity->GetVelocity();
						if (vel.x == 0 && vel.y == 0)
							continue;
						if (it == GrenadeProximityWarnings.end()) {
							auto thrower = entity->GetThrower();
							if (thrower) {
								if (!thrower->IsPlayer())
									continue;

								if (thrower->GetTeam() == csgo->local->GetTeam() && thrower != csgo->local)
									continue;
							}
							GrenadeProximityWarnings.emplace(handle, CSingleGrenadeProximityWarning(entity, trail,GW_FLASHBANG));
						}
						continue;
					}
					auto handle = entity->GetRefEHandle();
					auto it = GrenadeProximityWarnings.find(handle);
					auto vel = entity->GetVelocity();
					if (vel.x == 0 && vel.y == 0)
						continue;
					if (it == GrenadeProximityWarnings.end()) {
						auto thrower = entity->GetThrower();
						if (thrower) {
							if (!thrower->IsPlayer())
								continue;

							if (thrower->GetTeam() == csgo->local->GetTeam() && thrower != csgo->local)
								continue;
						}
						GrenadeProximityWarnings.emplace(handle, CSingleGrenadeProximityWarning(entity, trail, GW_HE_GRENADE));
					}
				}
				else if (class_id == (int)157) {

					if (entity->m_bDidSmokeEffect()) {
						static auto FilledCircle = [](Vector location, float radius, color_t Col, float alpha) {



							g_Render->Gradient3DCircleOp(g_Render->GetDevice(), location, radius, color_t(Col[0],
								Col[1], Col[2], 0), color_t(Col[0],
									Col[1], Col[2], 120 * alpha));

						};

						FilledCircle(entity->GetOrigin(), 145, color_t(255, 255, 255, 255), 1);
						continue;
					}

					auto handle = entity->GetRefEHandle();
					auto it = GrenadeProximityWarnings.find(handle);

					auto vel = entity->GetVelocity();
					if (vel.x == 0 && vel.y == 0)
						continue;
					if (it == GrenadeProximityWarnings.end()) {

						GrenadeProximityWarnings.emplace(handle, CSingleGrenadeProximityWarning(entity, trail, GW_SMOKE));
					}
					continue;
				}
				else if (class_id == (int)48) {

					auto handle = entity->GetRefEHandle();
					auto it = GrenadeProximityWarnings.find(handle);

					auto vel = entity->GetVelocity();
					if (vel.x == 0 && vel.y == 0)
						continue;
					if (it == GrenadeProximityWarnings.end()) {

						GrenadeProximityWarnings.emplace(handle, CSingleGrenadeProximityWarning(entity, trail, GW_DECOY));
					}
					continue;
				}
				/*
				else if (class_id == (int)114) {
					auto handle = entity->GetRefEHandle();
					auto it = GrenadeProximityWarnings.find(handle);

					auto vel = entity->GetVelocity();
					if (vel.x == 0 && vel.y == 0)
						continue;
					if (it == GrenadeProximityWarnings.end()) {
						auto thrower = entity->GetThrower();
						if (thrower) {
							if (!thrower->IsPlayer())
								continue;

							if (thrower->GetTeam() == csgo->local->GetTeam() && thrower != csgo->local)
								continue;
						}
						GrenadeProximityWarnings.emplace(handle, CSingleGrenadeProximityWarning(entity, trail, GW_MOLOTOV));
					}
					continue;
				}
				*/
				/*
				if (MolotovSpread || MolotovTimer) {
					if (class_id == ClassId->CInferno) {
						auto CurrentWarning = CSingleMolotovProximityWarning(entity);
						CurrentWarning.Render(MolotovTimer);
						continue;
					}
				}
				*/
				

				if (vars.visuals.BombT & 1 || vars.visuals.BombT & 2 || vars.visuals.BombT & 16) {
					if (entity->GetClientClass()->m_ClassID == ClassId->CPlantedC4) {
						if (entity->GetBombTicking()) {
							static float PAlphaD;
							static float DRadius;
							auto bombtime = entity->GetC4Blow() - interfaces.global_vars->curtime;
							if (bombtime > 0) {
								if (bombtime <= 0.2f) {
									PAlphaD = clamp(PAlphaD - 5.7f, 0.f, 255.f);
								}
								else {
									PAlphaD = clamp(PAlphaD + 5.7f, 0.f, 255.f);
								}
								static float MaxTime = entity->GetTimerLength();
								Vector dorigin = entity->GetAbsOrigin();
								if (vars.visuals.BombT & 16) {
									csgo->PDBomb.TimeLeft = bombtime;
									csgo->PDBomb.Planted = true;
									csgo->PDBomb.MaxTime = MaxTime;
									
									
								}
								if (vars.visuals.BombT & 32) {
									csgo->PDBomb.IsDefusing = entity->GetDefuser() != nullptr;
									if (csgo->PDBomb.IsDefusing) {
										csgo->PDBomb.DefuseTimeLeft = entity->GetDefuseCooldown() - interfaces.global_vars->curtime;
									}
								}
								trace_t Floor;
								VSTraceLine(dorigin + Vector(0,0,7), dorigin - Vector(0, 0, 7),MASK_SOLID,csgo->local,&Floor);
								Vector origin;
								if (PAlpha > 0) {
									if (vars.visuals.BombT & 2) {
										trace_t visible;
										VSTraceLine(csgo->local->GetEyePosition(), dorigin, MASK_SOLID, csgo->local, &visible);
										if (visible.fraction == 1.0f) {
											DRadius = clamp(DRadius + 3.0f, 0.0f, 50.f);
											
										}
										else {
											DRadius = clamp(DRadius - 3.0f, 0.0f, 50.f);
										}
										if (DRadius != 0.f) {
											g_Render->Gradient3DCircleOp(pDevice, Floor.endpos, (PAlphaD / 255)* DRadius, color_t(vars.visuals.BombColor[0],
												vars.visuals.BombColor[1], vars.visuals.BombColor[2], 0), color_t(vars.visuals.BombColor[0],
													vars.visuals.BombColor[1], vars.visuals.BombColor[2], (PAlphaD / 255)* (vars.visuals.BombColor[3] / 255) * 170));
										}
									}

										if (vars.visuals.BombT & 1)
										{
											Math::WorldToScreen(dorigin + Vector(0, 0, 60), origin);
											if (origin.x > 0 && origin.x < csgo->w - 0 && origin.y > 0 && origin.y < csgo->h - 0) {
												if (vars.visuals.BombT & 8) {
													color_t col1 = color_t(vars.visuals.BombColor[2],
														vars.visuals.BombColor[1], vars.visuals.BombColor[1], PAlphaD);
													g_Render->circle_filled_radial_gradient(pDevice, Vector2D(origin.x, origin.y), 39, color_t(col1[0],
														col1[1], col1[2], 0), color_t(col1[0],
															col1[1], col1[2], PAlphaD));
													g_Render->CircleFilled(origin.x, origin.y, 26 - 7, color_t(13, 13, 13, PAlphaD), 35);
													g_Render->PArc(origin.x, origin.y, 26 - 5, 1.5f * PI, ((1.5f * PI) - ((1.5f * PI + 1.57) * (bombtime / MaxTime))), 4.f,color_t(vars.visuals.BombColor[0],
														vars.visuals.BombColor[1], vars.visuals.BombColor[2], PAlphaD));
													g_Render->DrawString(origin.x, origin.y, color_t(255,255,255, PAlphaD), render::centered_x | render::centered_y, fonts::ESPIcons2, "n");

												}
												else
												{
													g_Render->PArc(origin.x, origin.y, 26.f - 6.f, 0.f, 2.f * PI, 9.f, color_t(13, 13, 13, PAlphaD));
													g_Render->PArc(origin.x, origin.y, 26.f - 6.f, 1.5f * PI, ((1.5f * PI) - ((1.5f * PI + 1.57) * (bombtime / MaxTime))), 6.f, color_t(vars.visuals.BombColor[0],
														vars.visuals.BombColor[1], vars.visuals.BombColor[2], PAlphaD));
													g_Render->DrawString(origin.x, origin.y, color_t(255, 255, 255, PAlphaD), render::centered_x | render::centered_y, fonts::AADebug, (to_string(((int)bombtime)) + " s").c_str());
												}
											}
											else if(vars.visuals.BombT & 4) //not on screen
											{
												Vector dir;
												interfaces.engine->GetViewAngles(dir);

												float view_angle = dir.y;

												if (view_angle < 0.f)
												{
													view_angle += 360.f;
												}
												view_angle = DEG2RAD(view_angle);

												auto entity_angle = Math::CalculateAngle(csgo->local->GetOrigin(), entity->GetAbsOrigin());
												entity_angle.Normalized();

												if (entity_angle.y < 0.f) {
													entity_angle.y += 360.f;
												}
												entity_angle.y = DEG2RAD(entity_angle.y);
												entity_angle.y -= view_angle;
												Vector2D origin2;
												origin2 = Vector2D(csgo->w / 2 - ((csgo->w / 1920) * 340), csgo->h / 2 - ((csgo->w / 1080) * 340));
												Drawing::rotate_point(origin2, Vector2D(csgo->w / 2, csgo->h / 2), false, entity_angle.y);
												if (vars.visuals.BombT & 8) {
													color_t col1 = color_t(vars.visuals.BombColor[2],
														vars.visuals.BombColor[1], vars.visuals.BombColor[1], PAlphaD);
													g_Render->circle_filled_radial_gradient(pDevice, Vector2D(origin2.x, origin2.y), 39, color_t(col1[0],
														col1[1], col1[2], 0), color_t(col1[0],
															col1[1], col1[2], PAlphaD));
													g_Render->CircleFilled(origin2.x, origin2.y, 26 - 7, color_t(13, 13, 13, PAlphaD), 35);
													g_Render->PArc(origin2.x, origin2.y, 26 - 5, 1.5f * PI, ((1.5f * PI) - ((1.5f * PI + 1.57) * (bombtime / MaxTime))), 4.f, color_t(vars.visuals.BombColor[0],
														vars.visuals.BombColor[1], vars.visuals.BombColor[2], PAlphaD));
													g_Render->DrawString(origin2.x, origin2.y, color_t(255, 255, 255, PAlphaD), render::centered_x | render::centered_y, fonts::ESPIcons2, "n");

												}
												else
												{
													g_Render->PArc(origin2.x, origin2.y, 26.f - 6.5f, 0.f, 2.f * PI, 9.f, color_t(13, 13, 13, PAlphaD));
													g_Render->PArc(origin2.x, origin2.y, 26.f - 6.5f, 1.5f * PI, ((1.5f * PI) - ((1.5f * PI + 1.57) * (bombtime / MaxTime))), 6.f, color_t(vars.visuals.BombColor[0],
														vars.visuals.BombColor[1], vars.visuals.BombColor[2], PAlphaD));
													g_Render->DrawString(origin2.x, origin2.y, color_t(255, 255, 255, PAlphaD), render::centered_x | render::centered_y, fonts::AADebug, (to_string(((int)bombtime)) + " s").c_str());
												}
											}
										}
								}

							}
						}

					}
				}

			}
		
			
		
			if (!GrenadeProximityWarnings.empty()) {
				float ctime = interfaces.global_vars->curtime;
				for (auto it = GrenadeProximityWarnings.begin(); it != GrenadeProximityWarnings.end();)
				{
					if (!csgo->local || !csgo->local->isAlive()) {
						it = GrenadeProximityWarnings.erase(it);
						continue;
					}
					auto Entity = interfaces.ent_list->GetClientEntityFromHandle(it->first);
					if (Entity) {
						bool IsSmoke = it->second.m_eType == GW_SMOKE;
						if (Entity->IsDormant()) {
							it->second.Render(animspeed, true);
							++it;
							continue;
						}

						if (IsSmoke) {
							if (Entity->m_bDidSmokeEffect()) {
								it = GrenadeProximityWarnings.erase(it);
								continue;
							}
						}

						auto vel = Entity->GetVelocity();
						if (vel.x == 0 && vel.y == 0 && !IsSmoke) {
							if (it->second.m_flAlpha <= 0) {
								it = GrenadeProximityWarnings.erase(it);
								continue;
							}
							else {
								it->second.Render(animspeed, false);
								++it;
								continue;
							}
						}
						else {
							it->second.Render(animspeed, true);
							++it;
							continue;
						}

					}
					else if (it->second.m_flAlpha <= 0) {
						it = GrenadeProximityWarnings.erase(it);
						continue;
					}
					else {
						it->second.Render(animspeed, false);
						++it;
						continue;
					}
					++it;
					continue;
				}
			}

		int dx = csgo->w, dy = csgo->h;

		if (vars.visuals.ScopeTHing > 0)
		{
			if (csgo->weapon->isSniper() && csgo->local->IsScoped())
			{
				if (vars.visuals.ScopeTHing == 1) {
					g_Render->DrawLine(dx / 2, 0, dx / 2, dy, color_t(0, 0, 0, 150), vars.visuals.scope_thickness);
					g_Render->DrawLine(0, dy / 2, dx, dy / 2, color_t(0, 0, 0, 150), vars.visuals.scope_thickness);
				}
				else if (vars.visuals.ScopeTHing == 2) {
					g_Render->DrawLine(dx / 2 - 140, dy / 2, dx / 2 - 25, dy / 2, vars.visuals.ScopeColor, vars.visuals.scope_thickness);
					g_Render->DrawLine(dx / 2 + 25, dy / 2, dx / 2 + 140, dy / 2, vars.visuals.ScopeColor, vars.visuals.scope_thickness);

					g_Render->DrawLine(dx / 2, dy / 2 - 140, dx / 2, dy / 2 - 25, vars.visuals.ScopeColor, vars.visuals.scope_thickness);

					g_Render->DrawLine(dx / 2, dy / 2 + 25, dx / 2, dy / 2 + 140, vars.visuals.ScopeColor, vars.visuals.scope_thickness);
				}
			}
		}

		dx /= 2;
		dy /= 2;

		if (vars.visuals.innacuracyoverlay)
		{

			g_Render->circle_filled_radial_gradient(pDevice, Vector2D(dx, dy), csgo->weaponspread,

				color_t(
					vars.visuals.innacuracyoverlay_color[2],
					vars.visuals.innacuracyoverlay_color[1],
					vars.visuals.innacuracyoverlay_color[0],
					vars.visuals.innacuracyoverlay_color[3]), color_t(
						vars.visuals.innacuracyoverlay_color[2],
						vars.visuals.innacuracyoverlay_color[1],
						vars.visuals.innacuracyoverlay_color[0],
						0));

		}

		static auto percent_col = [](int per) -> color_t {
			int red = per < 50 ? 255 : floorf(255 - (per * 2 - 100) * 255.f / 100.f);
			int green = per > 50 ? 255 : floorf((per * 2) * 255.f / 100.f);

			return color_t(red, green, 0);
		};

		int add = 0;

		auto anims = csgo->local->GetPlayerAnimState();

		if (vars.antiaim.enable && !csgo->ForceOffAA)
		{
			/*if (csgo->should_stop)
			{
				auto clr = percent_col(csgo->delta);
				g_Render->DrawString(10, dx - (50 + add), color_t(clr.get_alpha() - 129, clr.get_green() - 65, 10),
					render::outline, fonts::lby_indicator, "STOP"
				);
				//g_Render->DrawString(fonts::lby_indicator, 10, dx - (50 + add), false, false, true, color_t(clr.a() - 129, clr.g() - 65, 10), "FAKE");
				add += 30;
			}*/
			if (vars.visuals.indicators & 1 && vars.antiaim.enable)
			{
				if (vars.movable.leagcy)
				{
					int WW = csgo->w, WH_ = csgo->h;
					int WX = 65;
					int WH = 22;
					int WOffset = 8;
					int TOffset = 2;
					int EOffset = 10;
					int DM = 0;
					int FO = 0;



					string FL = "FL: " + to_string(vars.antiaim.sharedfakelag);
					int WW2 = csgo->w, WH_2 = csgo->h;
					int WX2 = 40;
					int WH2 = 22;
					int WOffset2 = 8;
					int TOffset2 = 2;
					int EOffset2 = 10;
					int DM2 = 0;
					if (vars.antiaim.sharedfakelag > 9)
					{
						DM2 += 5;
					}
					g_Render->FilledRect(WW2 - WX2 - WOffset2 - DM2, WOffset2 + WH2 + EOffset2, WX2 + DM2, WH2, color_t(0, 0, 0, 120));
					g_Render->DrawString(WW2 - WX2 - WOffset2 + 6 - DM2, WOffset2 + TOffset2 + WH2 + EOffset2, color_t(255, 255, 255), 0, fonts::WTLG, FL.c_str());
					FO += 40 + EOffset + DM2 - 2;


					if (vars.movable.shareddesync > 10)
					{
						DM += 5;
					}
					g_Render->FilledRect(WW - WX - WOffset - DM - FO, WOffset + WH + EOffset, WX + DM, WH, color_t(0, 0, 0, 120));
					g_Render->FilledRect(WW - WX - WOffset - 2 - DM - FO, WOffset + WH + EOffset, 2, WH, color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], 255));

					string AA = "FAKE (" + to_string(vars.movable.shareddesync) + "*)";

					g_Render->DrawString(WW - WX - WOffset + 6 - DM - FO, WOffset + TOffset + WH + EOffset, color_t(255, 255, 255), 0, fonts::WTLG, AA.c_str());
				}
				else

				{

				}
			}

		}

			int ix = csgo->h - (csgo->h / 3) - 55; //height / 2

			if ( csgo->PDBomb.Planted) {
				if (vars.visuals.BombT & 16) {
					
					g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
					g_Render->filled_rect_gradient(45, ix + add - 2, 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
					g_Render->DrawString(29, ix + add, (csgo->PDBomb.TimeLeft > (csgo->PDBomb.MaxTime / 2)) ? color_t(255 - (((csgo->PDBomb.TimeLeft - (csgo->PDBomb.MaxTime / 2)) / (csgo->PDBomb.MaxTime / 2)) * 255.f), 255.f, 0) : color_t(255.f, (csgo->PDBomb.TimeLeft / (csgo->PDBomb.MaxTime / 2)) * 255.f, 0),

						render::outline, fonts::lby_indicator, (std::to_string(((double)csgo->PDBomb.TimeLeft)) + " s").c_str()
					);
					add += 45;
				}
				if (vars.visuals.BombT & 32) {
					g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
					g_Render->filled_rect_gradient(45, ix + add - 2, 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
					g_Render->DrawString(29, ix + add, csgo->PDBomb.TimeLeft > 8.0f ? color_t(170, 170, 255) : csgo->PDBomb.TimeLeft > 4.f ? color_t(99, 255, 112) : color_t(255, 0, 0),

						render::outline, fonts::lby_indicator, csgo->PDBomb.TimeLeft > 10.f ? "NO KIT" : csgo->PDBomb.TimeLeft > 4.f ? "NEED KIT" : "NO TIME"
					);
					add += 45;

					if (csgo->PDBomb.IsDefusing) {
						g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
						g_Render->filled_rect_gradient(45, ix + add - 2, 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
						g_Render->DrawString(29, ix + add, csgo->PDBomb.DefuseTimeLeft > csgo->PDBomb.TimeLeft ? color_t(255, 0, 0) : color_t(99, 255, 112),

							render::outline, fonts::lby_indicator, (std::to_string(((double)csgo->PDBomb.DefuseTimeLeft)) + " s").c_str()
						);
						add += 45;
					}

				}
				
			}

			if (vars.visuals.indicators & 16 && vars.ragebot.double_tap->active)
			{

				//g_Render->DrawString(fonts::lby_indicator, 10, dx - (50 + add), false, false, true, color_t(41, 255, 41), "FD");
				g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
				g_Render->filled_rect_gradient(45, ix + add - 2, 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
				g_Render->DrawString(29, ix + add, (csgo->skip_ticks <= 0) ? color_t(255, 255, 255) : color_t(237, 43, 43),

					render::outline, fonts::lby_indicator, "DT"
				);
				add += 45;
			}

			static int LCC = 0;

			if (vars.visuals.indicators & 2 && vars.antiaim.sharedfakelag > 0)
			{

				if (!(csgo->local->GetFlags() & FL_ONGROUND))
				{
					LCC = 0;
				}
				else if (LCC <= 21)
				{
					LCC++;
				}
				if (LCC < 20)
				{
					color_t clr = csgo->canBreakLC || (csgo->NoTeleportRecharge > 0) ? color_t(181, 250, 32) : color_t(237, 43, 43);
					g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
					g_Render->filled_rect_gradient(45, ix + add - 2, 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
					g_Render->DrawString(29, ix + add, clr,
						render::outline, fonts::lby_indicator, "LC"
					);
					add += 45;
				}
			}

			if (vars.visuals.indicators & 32)
			{

				//g_Render->DrawString(fonts::lby_indicator, 10, dx - (50 + add), false, false, true, color_t(41, 255, 41), "FD");
				g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
				g_Render->filled_rect_gradient(45, ix + add - 2, 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
				g_Render->DrawString(29, ix + add, color_t(181, 250, 32),

					render::outline, fonts::lby_indicator, "DA"
				);
				add += 45;
			}
			if (vars.visuals.indicators & 64 && vars.antiaim.enable && csgo->local->isAlive()) {
				static float DAlpha = 0;
				if (DAlpha > 0) {
					static Vector MPos;
					static Vector SMPos;
					static Vector SMPos2;
					if (interfaces.input->m_fCameraInThirdPerson && csgo->local->GetRenderOrigin().IsValid()) {
						matrix bonemat[128];
						memcpy(bonemat, csgo->local->GetBoneCache().Base(), sizeof(matrix[128]));
						auto ChestPos = Ragebot::Get().GetPoint(csgo->local, (int)CSGOHitboxID::LowerChest, bonemat);
						if (ChestPos.IsZero() || !bonemat)
							Math::WorldToScreen(csgo->local->GetRenderOrigin() + (csgo->local->GetDuckAmount() <= 0.8f ? Vector(0, 0, 42) : Vector(0, 0, 33)), MPos);
						else
							Math::WorldToScreen(ChestPos, MPos);
					}
					if (SMPos.IsZero()) {
						SMPos = MPos;
						SMPos2 = MPos + Vector(90, -90, 0);
					}
					else
					{
						Vector Smoothed = MPos - SMPos;
						SMPos += Smoothed * 0.050f; //smooth modifier
						if (((vars.visuals.remove & 8 && csgo->weapon->GetZoomLevel() == 2) || (!(vars.visuals.remove & 8) && csgo->local->IsScoped()))) {
							SMPos2 += ((MPos + Vector((csgo->local->GetVelocity().Length() / 250) * 50 + 390, ((csgo->local->GetVelocity().Length() / 250) * 30) + -120, 0)) - SMPos2) * 0.033f;
						}
						else
						{
							SMPos2 += ((MPos + Vector((csgo->local->GetVelocity().Length() / 250) * 40 + 80, ((csgo->local->GetVelocity().Length() / 250) * 30) + -120, 0)) - SMPos2) * 0.033f;
						}
					}

					//smoothing from MPos to SMPos

					//g_Render->CircleFilled(SMPos.x + 18, SMPos.y, 20, color_t(255, 255, 255, DAlpha), 30);
					g_Render->DrawLine(SMPos.x + 18, SMPos.y, SMPos2.x, SMPos2.y, color_t(170, 170, 170, (DAlpha / 255) * 180), 1.f);
					g_Render->circle_filled_radial_gradient(pDevice, Vector2D(SMPos.x + 18, SMPos.y), 7, color_t(170, 170, 170, 0), color_t(170, 170, 170, (DAlpha / 255) * 100));
					g_Render->FilledRect(SMPos2.x, SMPos2.y - 60, 165, 70, color_t(0, 0, 0, (DAlpha / 255) * 120));
					g_Render->FilledRect(SMPos2.x, SMPos2.y - 62, 165, 2, color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], (DAlpha / 255) * 255));
					g_Render->DrawString(SMPos2.x + 6, SMPos2.y - 60 + 6, color_t(255, 255, 255, DAlpha), render::none, fonts::AADebug, "ANTI-AIMBOT DEBUG");
					string FakeS = to_string(vars.movable.shareddesync);
					float FakeF = vars.movable.shareddesync;
					if (FakeF > 60) {
						FakeF = 60;
					}
					color_t Col1 = color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], (DAlpha / 255) * ((FakeF / 60) * 255));
					color_t Col2 = color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], DAlpha);

					g_Render->filled_rect_gradient(SMPos2.x + 6, SMPos2.y - 60 + 22 + 10, 2, 8, Col1, Col1, Col2, Col2);
					g_Render->filled_rect_gradient(SMPos2.x + 6, SMPos2.y - 60 + 22 + 8 + 10, 2, 8, Col2, Col2, Col1, Col1);
					string FakeS2 = "FAKE (" + FakeS + "*)";
					g_Render->DrawString(SMPos2.x + 17, SMPos2.y - 60 + 22 + 2 + 10, color_t(255, 255, 255, DAlpha), render::none, fonts::AADebug, FakeS2.c_str());
					//g_Render->DrawString(SMPos2.x + 17, SMPos2.y - 70 + 22 + 2, color_t(255, 255, 255, DAlpha), render::none, fonts::AADebug, FakeS2.c_str());
					//159

					float SDD = 79;
					if (FakeF < 15) {
						FakeF = 15;
					}
					if ((vars.antiaim.inverter->active && !vars.ragebot.autoinvert) || (vars.ragebot.autoinvert && csgo->SwitchAA)) {
						SDD = 79 - ((FakeF / 60) * 79);
					}
					float Other = (FakeF / 60) * 79;
					g_Render->FilledRect(SMPos2.x + 6 + SDD, SMPos2.y - 60 + 57, Other, 5, color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], DAlpha));

				}
				if (interfaces.input->m_fCameraInThirdPerson) {
					DAlpha += 10.f;
				}
				else
				{
					DAlpha -= 10.f;
				}
				if (DAlpha < 0) {
					DAlpha = 0;
				}
				if (DAlpha > 255) {
					DAlpha = 255;
				}
			}
			if (vars.visuals.indicators & 4 && vars.antiaim.fakeduck->active && vars.antiaim.enable)
			{

				//g_Render->DrawString(fonts::lby_indicator, 10, dx - (50 + add), false, false, true, color_t(41, 255, 41), "FD");
				g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
				g_Render->filled_rect_gradient(45, ix + add - 2, 2 * 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
				g_Render->DrawString(29, ix + add, color_t(255, 255, 255),

					render::outline, fonts::lby_indicator, "DUCK"
				);
				add += 45;
			}

			if (vars.visuals.indicators & 8 && vars.ragebot.override_dmg->active)
			{

				//g_Render->DrawString(fonts::lby_indicator, 10, dx - (50 + add), false, false, true, color_t(41, 255, 41), "FD");
				g_Render->filled_rect_gradient(15, ix + add - 2, 30, 32, color_t(0, 0, 0, 0), color_t(0, 0, 0, 60), color_t(0, 0, 0, 60), color_t(0, 0, 0, 0));
				g_Render->filled_rect_gradient(45, ix + add - 2, 9 * 15, 32, color_t(0, 0, 0, 60), color_t(0, 0, 0, 0), color_t(0, 0, 0, 0), color_t(0, 0, 0, 60));
				string c = "Damage: " + to_string(CurWepSet().mindamage_override);
				g_Render->DrawString(29, ix + add, color_t(255, 255, 255), render::outline, fonts::lby_indicator, c.c_str());
				add += 45;
			}

	}
	else
	this->GrenadeProximityWarnings.clear();

	

	if (vars.visuals.indicators & 128) {
		static bool IsHoldingM12;
		static Vector2D MouseModif2;
		static float DTAlpha;

		if (vars.ragebot.double_tap->active || vars.menu.open) {
			DTAlpha = clamp(DTAlpha + 16.f, 0.f, 255.f);
		}
		else {
			DTAlpha = clamp(DTAlpha - 16.f, 0.f, 255.f);
		}

		auto clr = csgo->skip_ticks > 0 ? color_t(56, 73, 255, 0) : color_t(255, 99, 99, 0);
		auto clr2 = csgo->skip_ticks > 0 ? color_t(56, 73, 255, DTAlpha) : color_t(255, 99, 99, DTAlpha);
		g_Render->circle_filled_radial_gradient(pDevice, Vector2D(vars.visuals.doubletapindicator.X + 45 / 2, vars.visuals.doubletapindicator.Y + 45 / 2), 46, clr, clr2
		);
		g_Render->CircleFilled(vars.visuals.doubletapindicator.X + 45 / 2, vars.visuals.doubletapindicator.Y + 45 / 2, 35, color_t(0, 9, 67, DTAlpha), 40);
		g_Render->_drawList->AddImage((void*)csgo->Logo, ImVec2(vars.visuals.doubletapindicator.X, vars.visuals.doubletapindicator.Y), ImVec2(vars.visuals.doubletapindicator.X + 45, vars.visuals.doubletapindicator.Y + 45), ImVec2(0, 0), ImVec2(1, 1), color_t(255, 255, 255, DTAlpha).u32());
		int MaxDT = get_max_tickbase_shiftS();
		static float CurT2 = 0.0f;
		if (csgo->skip_ticks > 0) {
			float CurT = 4.71 / MaxDT;
			g_Render->PArc(vars.visuals.doubletapindicator.X + 45 / 2, vars.visuals.doubletapindicator.Y + 45 / 2, 28, -1.57, CurT2, 2.5f, color_t(170, 170, 255, DTAlpha));
			CurT2 += CurT;
		}
		else {
			CurT2 = 0;
		}
		if (DTAlpha > 0.f && vars.menu.open && ImGui::GetIO().MouseDownDuration[0] > 0.f)
		{
			if (g_mouse.x > vars.visuals.doubletapindicator.X && g_mouse.x < vars.visuals.doubletapindicator.X + 100 && g_mouse.y > vars.visuals.doubletapindicator.Y && g_mouse.y < vars.visuals.doubletapindicator.Y + 100)
			{
				if (!IsHoldingM12)
				{
					MouseModif2.x = g_mouse.x - vars.visuals.doubletapindicator.X;
					MouseModif2.y = g_mouse.y - vars.visuals.doubletapindicator.Y;
					IsHoldingM12 = true;
				}
				vars.visuals.doubletapindicator.X = clamp(g_mouse.x - MouseModif2.x, 0.f, 1920 - 100.f);
				vars.visuals.doubletapindicator.Y = clamp(g_mouse.y - MouseModif2.y, 0.f, 1080 - 100.f);
			}
		}
		else
		{
			IsHoldingM12 = false;
		}
	}

	if (vars.antiaim.aa_override.enable) {
		int X = (csgo->w) / 2;
		int Y = (csgo->h) / 2;
		if (vars.antiaim.direction == 1)
		{
			g_Render->DrawString(X - 60, Y, color_t(vars.antiaim.manualindicator[0], vars.antiaim.manualindicator[1], vars.antiaim.manualindicator[2], vars.antiaim.manualindicator[3]), render::centered_x, fonts::MoreIcons, "C");
		}
		if (vars.antiaim.direction == 2)
		{
			g_Render->DrawString(X + 60, Y, color_t(vars.antiaim.manualindicator[0], vars.antiaim.manualindicator[1], vars.antiaim.manualindicator[2], vars.antiaim.manualindicator[3]), render::centered_x, fonts::MoreIcons, "D");
		}
		if (vars.antiaim.direction == 3)
		{
			g_Render->DrawString(X, Y + 60, color_t(vars.antiaim.manualindicator[0], vars.antiaim.manualindicator[1], vars.antiaim.manualindicator[2], vars.antiaim.manualindicator[3]), render::centered_x, fonts::MoreIcons, "E");
		}
	}

	if (vars.antiaim.indicatorr)
	{
		if (interfaces.engine->IsConnected() && interfaces.engine->IsInGame())
		{
			static float finish;
			static bool reset = false;
			static bool drawww = true;
			static int dev;
			int radius = ClampDouble((0.15) * csgo->local->GetVelocity().Length(), 0, (0.15 * 95));
			bool color = false;

			int desync = 3.f - (0.06 * vars.antiaim.DesyncAMM);
			int fakelag = 0.4f - (vars.antiaim.sharedfakelag / 5);
			int yaw = 5.f - (0.06 * vars.antiaim.sharedyaw);
			int W = csgo->w, H = csgo->h;

			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 53 - 5 - radius - vars.visuals.disclay + 6, color ? color_t(255, 48, 59, 255) : color_t(97, 208, 255), 230);
			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 50 - 5 - radius - vars.visuals.disclay + 6, color_t(8, 8, 8, 255), 230);

			//desync
			g_Render->Arc(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 40 + 3 - 5 - radius - vars.visuals.disclay + 6, 3.f, desync, color_t(163, 255, 230));
			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 37 + 3 - 5 - radius - vars.visuals.disclay + 6, color_t(8, 8, 8, 255), 230);

			//fakelag
			g_Render->Arc(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 40 + 3 - 10 - radius - vars.visuals.disclay + 6, -0.4f, -fakelag, color_t(255, 163, 197));
			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 37 + 3 - 10 - radius - vars.visuals.disclay + 6, color_t(8, 8, 8, 255), 230);

			//yaw
			g_Render->Arc(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 40 + 3 - 15 - radius - vars.visuals.disclay + 6, -5.f, -yaw, color_t(255, 241, 163));
			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset + 160, H / 2 + 150 + 50, 37 + 3 - 15 - radius - vars.visuals.disclay + 6, color_t(8, 8, 8, 255), 230);
			//g_Render->CircleFilled((W / 3 + 240) / 3, (H / 2 + 30) / 3, 5, color_t(8, 8, 8, 255), 230);




			g_Render->DrawString(W / 4 + vars.visuals.indicatoroffset + 161, H / 2 + 150 - 11 + 50, color_t(255, 255, 255), render::centered_x, fonts::Logo, "Q");

			//g_Render->DrawString(W / 4 + vars.visuals.indicatoroffset + 1, H / 2 + 30 - 10 - 13, color_t(255, 255, 255), render::centered_x, fonts::Iconss, "T");
		}
	}

	if (vars.visuals.DTindi)
	{
		static float finish;
		static bool reset = false;
		static bool drawww = true;
		static int dev;
		static int radius = 7;
		bool color = false;
		if (CanDT() && CMAntiAim::Get().did_shot)
			reset = true;
		if (csgo->dt_charged && CanDT() && !CMAntiAim::Get().did_shot) {
			finish = -3.f;
			drawww = true;
		}
		else
		{
			if (reset)
			{
				finish = 2.f;
				reset = false;
			}
			if (csgo->skip_ticks >= (30 * 1.5))
			{
				dev = 1;
			}
			if (csgo->skip_ticks >= (23 * 1.5))
				dev = 2;
			if (csgo->skip_ticks <= (16 * 1.5))
				dev = 3;
			if (dev == 1)
			{
				finish -= 0.085f;
			}
			if (dev == 2)
			{
				finish -= 0.095f;
			}
			if (dev == 3)
			{
				finish -= 0.16f;
			}
			drawww = true;
			reset = false;
		}
		if (finish > -3.f)
			color = true;
		if (finish < -3.f) //reset
			dev = 0;
		if (!vars.ragebot.double_tap->active)
			drawww = false;
		float Final = 3.f;
		if (finish > 2.f)
			Final = 2.f;
		else
			Final = finish;
		int W = csgo->w, H = csgo->h;
		if (drawww)
		{
			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset, H / 2 + 30 + 50, 53 - radius - vars.visuals.disclay, color ? color_t(255, 48, 59, 255) : color_t(97, 208, 255), 230);
			g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset, H / 2 + 30 + 50, 50 - radius - vars.visuals.disclay, color_t(8, 8, 8, 255), 230);
			if (color)
			{
				if (!csgo->dt_charged)
				{
					g_Render->Arc(W / 4 + vars.visuals.indicatoroffset, H / 2 + 30 + 50, 40 + 3 - radius - vars.visuals.disclay, 3.f, Final, color_t(235, 249, 255, 255));
					g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset, H / 2 + 30 + 50, 37 + 3 - radius - vars.visuals.disclay, color_t(8, 8, 8, 255), 230);
				}
				else
				{
					//g_Render->Arc(W / 3 + 240, H / 2 + 30, 40 - 15, 3.f, -3.f, color_t(235, 249, 255, 255));
					g_Render->CircleFilled(W / 4 + vars.visuals.indicatoroffset, H / 2 + 30 + 50, 37 + 3 - radius - vars.visuals.disclay, color_t(8, 8, 8, 255), 230);
				}
				//g_Render->CircleFilled((W / 3 + 240) / 3, (H / 2 + 30) / 3, 5, color_t(8, 8, 8, 255), 230);
				if (radius < 15 && color)
					radius += 2;


			}
			if (radius > 7 && !color && csgo->dt_charged)
				radius -= 0.5;
			if (vars.ragebot.BDT)
			{
				g_Render->DrawString(W / 4 + vars.visuals.indicatoroffset + 1, H / 2 + 30 - 11 + 50, color_t(255, 255, 255), render::centered_x, fonts::MoreIcons, "G");
			}
			else
			{
				g_Render->DrawString(W / 4 + vars.visuals.indicatoroffset + 1, H / 2 + 30 - 14 + 50, color_t(255, 255, 255), render::centered_x, fonts::Econs, "A");
			}
			//g_Render->DrawString(W / 4 + vars.visuals.indicatoroffset + 1, H / 2 + 30 - 10 - 13, color_t(255, 255, 255), render::centered_x, fonts::Iconss, "T");
		}
	}



	if (vars.menu.keybinds)
	{
		int w = csgo->w, h = csgo->h;
		int dx = vars.menu.keybindX;

		int dy = vars.menu.keybindY;
		auto KTXM = 0;
		int keybindY = dy + 29;
		int KLX = dx + 7;
		int KTX = dx + 123 + KTXM;
		static bool keybindd = false;
		static float AllAlpha = 0.0f;
		if (vars.menu.open)
		{
			AllAlpha = std::clamp(AllAlpha + 26.0f, 0.f, 255.f);
		}
		//g_Render->filled_rect_gradient(dx - 292 + 100, 17, 129 - 100, 1, color_t(180, 80, 295), color_t(195, 207, 78), color_t(153, 296, 21), color_t(290, 50, 146));




		bool showkeybinds = false;
		static float inverteralpha, forcebaim, dmg, doubletap, fd, sw, thirdperson, fs, ad, add, ons, ym, yb, pt, fs2, fl, sp, ey, maa, onaa, opaa, haa,qpa,hs;
		int Size2 = 0;

	

		if (vars.antiaim.manual.edgeyaw->active || ey > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.antiaim.manual.edgeyaw->type;

			auto A = vars.antiaim.manual.edgeyaw->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, ey * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Wall anti-aim");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, ey * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				ey = std::clamp(ey + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				ey = std::clamp(ey - animation_speed * 300.f, 0.f, 255.f);

			}
		}
	

		if (vars.keybind.safepoint->active || sp > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.safepoint->type;

			auto A = vars.keybind.safepoint->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, sp * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Safe point");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, sp * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				sp = std::clamp(sp + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				sp = std::clamp(sp - animation_speed * 300.f, 0.f, 255.f);

			}
		}



		if ((vars.movable.OnPeekAA->active && vars.movable.OnPeekAA->type != 0) || opaa > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.movable.OnPeekAA->type;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, opaa * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "On peek anti-aim");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, opaa * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (vars.movable.OnPeekAA->active && vars.movable.OnPeekAA->type != 0)
			{
				opaa = std::clamp(opaa + animation_speed * 300.f, 0.f, 255.f);
			}
			else
			{
				opaa = std::clamp(opaa - animation_speed * 300.f, 0.f, 255.f);
			}
		}

		if ((vars.movable.OnshotAAA->active && vars.movable.OnshotAAA->type != 0) || onaa > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.movable.OnshotAAA->type;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, onaa * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "On shot anti-aim");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, onaa * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (vars.movable.OnshotAAA->active && vars.movable.OnshotAAA->type != 0)
			{
				onaa = std::clamp(onaa + animation_speed * 300.f, 0.f, 255.f);
			}
			else
			{
				onaa = std::clamp(onaa - animation_speed * 300.f, 0.f, 255.f);
			}
		}


		if (vars.antiaim.inverter->active || inverteralpha > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.antiaim.inverter->type;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, inverteralpha * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Invert desync");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, inverteralpha * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (vars.antiaim.inverter->active)
			{
				inverteralpha = std::clamp(inverteralpha + animation_speed * 300.f, 0.f, 255.f);
			}
			else
			{
				inverteralpha = std::clamp(inverteralpha - animation_speed * 300.f, 0.f, 255.f);
			}
		}

		if (vars.ragebot.force_body->active || forcebaim > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.ragebot.force_body->type;

			auto A = vars.ragebot.force_body->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, forcebaim * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Force baim");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, forcebaim * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				forcebaim = std::clamp(forcebaim + animation_speed * 300.f, 0.f, 255.f);
			}
			else
			{
				forcebaim = std::clamp(forcebaim - animation_speed * 300.f, 0.f, 255.f);
			}
		}


		if (vars.ragebot.override_dmg->active || dmg > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.ragebot.override_dmg->type;

			auto A = vars.ragebot.override_dmg->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, dmg * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Damage override");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, dmg * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				dmg = std::clamp(dmg + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				dmg = std::clamp(dmg - animation_speed * 300.f, 0.f, 255.f);

			}
		}
		if (vars.keybind.onshot->type == 0)
		{
			ons = 0;
		}
		if ((vars.keybind.onshot->active && vars.ragebot.backshoot_bt && vars.keybind.onshot->type > 0) || ons > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.onshot->type;

			auto A = vars.keybind.onshot->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, ons * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Prefer onshot");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, ons * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				ons = std::clamp(ons + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				ons = std::clamp(ons - animation_speed * 300.f, 0.f, 255.f);

			}
		}
		if (vars.keybind.yawmodifier->type == 0)
		{
			ym = 0;
		}
		if ((vars.checkbox.yawmodifier && vars.keybind.yawmodifier->type > 0) || ym > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.yawmodifier->type;
			float EA = ym;
			auto A = vars.checkbox.yawmodifier;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Yaw modifier");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				ym = std::clamp(EA + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				ym = std::clamp(EA - animation_speed * 300.f, 0.f, 255.f);

			}
		}
		if (vars.keybind.fakelag->type == 0)
		{
			fl = 0;
		}
		if ((vars.checkbox.fakelag && vars.keybind.fakelag->type > 0) || fl > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.fakelag->type;
			float EA = fl;
			auto A = vars.checkbox.fakelag;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Fakelag");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				fl = std::clamp(EA + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				fl = std::clamp(EA - animation_speed * 300.f, 0.f, 255.f);

			}
		}
		if (vars.keybind.pitch->type == 0)
		{
			pt = 0;
		}
		if ((vars.checkbox.pitch && vars.keybind.pitch->type > 0) || pt > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.pitch->type;
			float EA = pt;
			auto A = vars.checkbox.pitch;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Pitch");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				pt = std::clamp(EA + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				pt = std::clamp(EA - animation_speed * 300.f, 0.f, 255.f);

			}
		}
		if (vars.keybind.freestand->type == 0)
		{
			fs = 0;
		}
		if ((vars.checkbox.freestand && vars.keybind.freestand->type > 0) || fs2 > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.freestand->type;
			float EA = fs2;
			auto A = vars.checkbox.freestand;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Freestand");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				fs2 = std::clamp(EA + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				fs2 = std::clamp(EA - animation_speed * 300.f, 0.f, 255.f);

			}
		}
		if (vars.keybind.yawbase->type == 0)
		{
			yb = 0;
		}
		if ((vars.checkbox.yawbase && vars.keybind.yawbase->type > 0) || yb > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.yawbase->type;
			float EA = yb;
			auto A = vars.checkbox.yawbase;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Yaw base");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, EA * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				yb = std::clamp(EA + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				yb = std::clamp(EA - animation_speed * 300.f, 0.f, 255.f);

			}
		}

		if ((vars.keybind.autodir->active && vars.movable.autodir && vars.keybind.autodir->type > 0) || add > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.keybind.autodir->type;

			auto A = vars.movable.autodir;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, add * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Edge anti-aim");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, add * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				add = std::clamp(add + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				add = std::clamp(add - animation_speed * 300.f, 0.f, 255.f);

			}
		}

		if (vars.ragebot.double_tap->active || doubletap > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.ragebot.double_tap->type;

			auto A = vars.ragebot.double_tap->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, doubletap * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Double tap");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, doubletap * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				doubletap = std::clamp(doubletap + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				doubletap = std::clamp(doubletap - animation_speed * 300.f, 0.f, 255.f);

			}
		}

		if (hs > 0 || (vars.ragebot.hideshots->type > 0 && vars.ragebot.hideshots->active)) {
			string text;
			int Modiff = 0;
			auto t = vars.ragebot.hideshots->type;

			auto A = vars.ragebot.hideshots->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, hs * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Hide shots");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, hs * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				hs = std::clamp(hs + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				hs = std::clamp(hs - animation_speed * 300.f, 0.f, 255.f);

			}
		}


		if (qpa > 0 || (vars.misc.AutoPeek->type > 0 && vars.misc.AutoPeek->active)) {
			string text;
			int Modiff = 0;
			auto t = vars.misc.AutoPeek->type;

			auto A = vars.misc.AutoPeek->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, qpa* (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Quick peek assist");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, qpa * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				qpa = std::clamp(qpa + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				qpa = std::clamp(qpa - animation_speed * 300.f, 0.f, 255.f);

			}
		}

		if ((vars.movable.HideAA->active && vars.movable.HideAA->type != 0) || haa > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.movable.HideAA->type;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, haa * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Hide anti-aim");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, haa* (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (vars.movable.HideAA->active && vars.movable.HideAA->type != 0)
			{
				haa = std::clamp(haa + animation_speed * 300.f, 0.f, 255.f);
			}
			else
			{
				haa = std::clamp(haa - animation_speed * 300.f, 0.f, 255.f);
			}
		}

		if (vars.antiaim.fakeduck->active || fd > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.antiaim.fakeduck->type;

			auto A = vars.antiaim.fakeduck->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, fd * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Duck peek assist");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, fd * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				fd = std::clamp(fd + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				fd = std::clamp(fd - animation_speed * 300.f, 0.f, 255.f);

			}
		}

		if (thirdperson > 0 || (vars.misc.thirdperson && vars.misc.thirdperson_bind->active && vars.misc.thirdperson_bind->type > 0)) {
			string text;
			int Modiff = 0;
			auto t = vars.misc.thirdperson_bind->type;

			auto A = vars.misc.thirdperson && vars.misc.thirdperson_bind->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, thirdperson * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Thirdperson");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, thirdperson * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				thirdperson = std::clamp(thirdperson + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				thirdperson = std::clamp(thirdperson - animation_speed * 300.f, 0.f, 255.f);

			}
		}

		if (vars.misc.thirdperson_bind->type == 0)
		{
			thirdperson = 0;
		}
		if (vars.keybind.autodir->type == 0)
		{
			add = 0;
		}


		if (vars.antiaim.slowwalk->active || sw > 0) {
			string text;
			int Modiff = 0;
			auto t = vars.antiaim.slowwalk->type;

			auto A = vars.antiaim.slowwalk->active;
			switch (t) {
			case 1: text = "[hold]"; break;
			case 2: text = "[toggle]"; break;
			case 3: text = "[hold]"; break;
			case 4: text = "[on]"; break;
			}
			auto E = 0;
			switch (t) {
			case 1: E = 10; break;
			case 2: E = 18; break;
			case 3: E = 10; break;
			case 4: E = 0; break;
			}
			if (Size2 < E)
			{
				Size2 = E;
			}
			g_Render->DrawString(KLX - Modiff, keybindY, color_t(255, 255, 255, sw * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, "Slow walk");
			g_Render->DrawString(KTX - Modiff + ((E == 18) ? 18 : 21) - E, keybindY, color_t(255, 255, 255, sw * (AllAlpha / 255)), 0, fonts::SmallPixelWatermark, text.c_str());
			keybindY += 14;
			showkeybinds = true;
			if (A)
			{
				sw = std::clamp(sw + animation_speed * 300.f, 0.f, 255.f);

			}
			else
			{
				sw = std::clamp(sw - animation_speed * 300.f, 0.f, 255.f);

			}
		}


		g_Render->FilledRect(dx, dy + 4, 150 + Size2, 22, color_t(0, 0, 0, std::clamp((AllAlpha / 255) * 90, 0.f, 255.f)));

		g_Render->FilledRect(dx, dy + 2, 150 + Size2, 2, vars.movable.leagcy ? color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], std::clamp((AllAlpha), 0.f, 255.f)) : color_t(210, 145, 255, std::clamp((AllAlpha), 0.f, 255.f)));

		//g_Render->DrawString(dx + 10, dy + 2.5, color_t(89, 147, 255), 0, fonts::Keybinds, "A");
		g_Render->DrawString(dx + 50 + Size2 / 2, dy + 6 + 2, color_t(255, 255, 255, AllAlpha), 0, fonts::SmallPixelWatermark, "Keybinds");

		if (showkeybinds)
		{
			AllAlpha = std::clamp(AllAlpha + animation_speed * 350.f, 0.f, 255.f);
		}
		else
		{
			AllAlpha = std::clamp(AllAlpha - animation_speed * 350.f, 0.f, 255.f);
		}

		static bool IsHoldingM1;
		static Vector2D MouseModif;

		if (AllAlpha > 0.f && vars.menu.open && ImGui::GetIO().MouseDownDuration[0] > 0.f)
		{
			if (g_mouse.x > vars.menu.keybindX - 5 && g_mouse.x < vars.menu.keybindX + 150 + Size2 + 5 && g_mouse.y > vars.menu.keybindY - 3 && g_mouse.y < vars.menu.keybindY + 25 + 3)
			{
				if (!IsHoldingM1)
				{
					MouseModif.x = g_mouse.x - vars.menu.keybindX;
					MouseModif.y = g_mouse.y - vars.menu.keybindY;
					IsHoldingM1 = true;
				}
				vars.menu.keybindX = clamp(g_mouse.x - MouseModif.x, 0.f, (float)w - (150 + Size2));
				vars.menu.keybindY = clamp(g_mouse.y - MouseModif.y, 0.f, (float)h - 25);
			}
		}
		else
		{
			IsHoldingM1 = false;
		}
	}
	if (vars.visuals.watermark)
	{
		bool alpha = true;
		int width, height;
		interfaces.engine->GetScreenSize(width, height); // owowowo
		auto net_channel = interfaces.engine->GetNetChannelInfo();
		std::string incoming = csgo->local ? std::to_string((int)(net_channel->GetLatency(FLOW_INCOMING) * 1000)) : "0";

		int dx = csgo->w, dy = csgo->h;


		std::stringstream ss;
		std::stringstream rr;

		//getting the user


		//fps
		int old_tick_count = static_cast<int>(1.f / interfaces.global_vars->frametime);;
		old_tick_count = clamp(old_tick_count, 0, INT_MAX);
		string fps = to_string(old_tick_count) + " FPS";
		//rectangles and all that shi9
		//g_Render->FilledRect(dx - 290 + 100, 12, 260 - 100, 30, color_t(40, 40, 40, 225));

		//g_Render->Rect(dx - 285 + 100, 17, 250 - 100, 20, color_t(200, 200, 200, 50));
		//g_Render->Rect(dx - 290 + 100, 12, 260 - 100, 30, color_t(200, 200, 200, 50));

		//g_Render->filled_rect_gradient(dx - 292 + 100, 17, 129 - 100, 1, color_t(180, 80, 295), color_t(195, 207, 78), color_t(153, 296, 21), color_t(290, 50, 146));

		if (vars.movable.leagcy)
		{
			int infd = csgo->local ? (int)(net_channel->GetLatency(FLOW_INCOMING) * 1000) : 0;


			
			int MMd = 0;
			int WX = 0 ;
			int WH = 22;
			int WOffset = 8;


			//auto sysc = std::chrono::system_clock::now();


			time_t systime = time(0);
			tm* ctime = localtime(&systime);
			string c = ctime->tm_sec > 9 ? to_string(ctime->tm_sec) : "0" + to_string(ctime->tm_sec);
			string m = ctime->tm_min > 9 ? to_string(ctime->tm_min) : "0" + to_string(ctime->tm_min);
			string h = ctime->tm_hour > 9 ? to_string(ctime->tm_hour) : "0" + to_string(ctime->tm_hour);
			string stime = h + ":" + m + ":" + c;
			string LGWatermark = "FREZZYHOOK [beta] | " + csgo->FullUsername + " | delay: " + to_string(infd) + "ms | " + stime;
			ImVec2 MSSize;
			ImGui::PushFont(fonts::WTLG);
			MSSize = ImGui::CalcTextSize(LGWatermark.c_str());
			ImGui::PopFont();

			WX += MSSize.x + 13;


			g_Render->FilledRect(dx - WX - WOffset - MMd, WOffset + 2, WX + MMd, WH, color_t(0, 0, 0, 120));
			g_Render->FilledRect(dx - WX - WOffset - MMd, WOffset, WX + MMd, 2, color_t(vars.movable.PCol[0], vars.movable.PCol[1], vars.movable.PCol[2], 255));



			g_Render->DrawString(dx - WX - WOffset + 6 - MMd, WOffset + 4 + 2, color_t(255, 255, 255), 0, fonts::WTLG, LGWatermark.c_str());
		}
		else
		{
			g_Render->FilledRect(dx - 285 + 100 - 30 + 12 - 63 - 18 - 4 + 3, 17 - 5 - 5, 250 - 100 + 40 + 63 + 10 + 16 + 4 - 5, 20 + 10, color_t(0, 0, 34, 190));
			g_Render->DrawString(dx - 279 + 100 - 29 + 12 - 63 - 18 - 3 + 3, 20.5 - 4 - 5, color_t(255, 255, 255), 0, fonts::Watermark2, "FREZZY");
			g_Render->DrawString(dx - 279 + 100 - 29 + 7 + 12 - 10 - 9 + 3, 20.5 - 4 - 5, color_t(210, 145, 255), 0, fonts::Watermark2, ".GAY");
			std::string ping = incoming + " MS";
			std::string fpss = fps + "FPS";
			g_Render->DrawString(dx - 279 + 100 - 29 + 7 + 60 + 12 + 15 - 4 + 3, 20.5 - 5 - 5, color_t(255, 255, 255), render::centered_x, fonts::Watermark, ping.c_str());
			g_Render->DrawString(dx - 279 + 100 - 29 + 7 + 140 + 12 + 3, 20.5 - 5 - 5, color_t(255, 255, 255), render::centered_x, fonts::Watermark, fps.c_str());
		}

	}
}

std::string str_toupper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return toupper(c); }
	);
	return s;
}
void DrawFovArrows(IBasePlayer* entity, float alpha, Vector origin)
{
	if (!vars.visuals.out_of_fov || !csgo->local->isAlive())
		return;

	auto idx = entity->GetIndex() - 1;
	Vector poopvec;

	Vector vEnemyOrigin = origin;
	Vector vLocalOrigin = csgo->origin;

	Vector screenPos;

	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!Math::WorldToScreen(origin, screen))
			return false;

		return (csgo->w > screen.x && screen.x > 0) && (csgo->h > screen.y && screen.y > 0);
	};

	if (!entity->IsDormant()) {
		if (isOnScreen(entity->GetBonePos(entity->GetBoneCache().Base(), 2), screenPos))
			return;
		else
			if (isOnScreen(vEnemyOrigin, screenPos))
				return;

	}
	Vector dir;
	interfaces.engine->GetViewAngles(dir);

	float view_angle = dir.y;

	if (view_angle < 0.f)
	{
		view_angle += 360.f;
	}
	view_angle = DEG2RAD(view_angle);

	auto entity_angle = Math::CalculateAngle(vLocalOrigin, vEnemyOrigin);
	entity_angle.Normalized();

	if (entity_angle.y < 0.f) {
		entity_angle.y += 360.f;
	}
	entity_angle.y = DEG2RAD(entity_angle.y);
	entity_angle.y -= view_angle;

	float dist = vars.visuals.out_of_fov_distance * 10;
	static float alphaa = 0.f;
	static bool alphaaa = false;
	if (!alphaaa && alphaa < 255.f)
	{
		alphaa = std::clamp(alphaa + 0.3f, 0.f, 255.f);
	}
	else if (!alphaaa && alphaa >= 255.f)
	{
		alphaaa = true;
	}
	else if (alphaaa && alphaa > 0.f)
	{
		alphaa = std::clamp(alphaa - 0.3f, 0.f, 255.f);
	}
	else if (alphaaa && alphaa <= 0.f)
	{
		alphaaa = false;
	}
	if (!(vars.visuals.FOVArrowsFlags & 8)) {
		alphaa = 255.f;
	}
	auto position = Vector2D(csgo->w / 2, csgo->h / 2);
	position.x -= dist;
	if (vars.visuals.FOVArrowsFlags & 1) {
		if (alpha < 100) {
			alpha = 100;
		}
	}
	auto clr = color_t(
		vars.visuals.out_of_fov_color[0],
		vars.visuals.out_of_fov_color[1],
		vars.visuals.out_of_fov_color[2],
		(alpha / 255) * (vars.visuals.out_of_fov_color[3] * (alphaa / 255.f)));

	Drawing::rotate_point(position, Vector2D(csgo->w / 2, csgo->h / 2), false, entity_angle.y);

	auto size = vars.visuals.out_of_fov_size / 2;


	Drawing::filled_tilted_triangle(position, Vector2D(size - 1, size), position, true, -entity_angle.y, clr);

	float box = position.x - 27;
	float boxy = position.y - 27;
	size += 27;


	if (vars.visuals.name && vars.visuals.FOVArrowsFlags & 4)
	{
		auto text_size = Drawing::GetTextSize(fonts::esp_name, entity->GetName().c_str());

		auto clr = color_t(255, 255, 255, alpha);

		Drawing::DrawString(fonts::esp_name, box + size / 2 - text_size.right / 2, boxy - 13, clr, FONT_LEFT, entity->GetName().c_str());
		//g_Render->DrawGradient3DCircle(csgo->local->GetOrigin(), 15.f, color_t(210, 145, 255, 200), color_t(210, 145, 255, 0));
	}

	if (vars.visuals.healthbar && vars.visuals.FOVArrowsFlags & 2)
	{
		int hp = entity->GetHealth();

		if (hp > 100)
			hp = 100;

		int hp_percent = size - (int)((size * hp) / 100);

		int width = (size * (hp / 100.f));

		int red = 255 - (hp * 2.55);
		int green = hp * 2.55;

		char hps[10] = "";

		sprintf_s(hps, "%i", hp);

		auto text_size = Drawing::GetTextSize(fonts::esp_info, hps);

		auto clr = color_t(255, 255, 255, 255);

		auto hp_color = vars.visuals.override_hp ? color_t(
			vars.visuals.hp_color[0],
			vars.visuals.hp_color[1],
			vars.visuals.hp_color[2],
			(alpha / 255) * (vars.visuals.hp_color[3] * (alphaa / 255.f))) : color_t(red, green, 0, (alpha / 255) * vars.visuals.hp_color[3]);

		Drawing::DrawRect(box - 5, boxy - 1, 4, size + 2, color_t(80, 80, 80, (alpha / 255) * vars.visuals.hp_color[3]));
		//Drawing::DrawOutlinedRect(box.x - 6, box.y - 1, 4, box.h + 2, color_t(10, 10, 10, (alpha * 0.8f)));
		Drawing::DrawRect(box - 4, boxy + hp_percent, 2, size - hp_percent, hp_color);

		if (hp <= 99) // draws hp count at the same pos as top of bar
			Drawing::DrawString(fonts::esp_info, box - text_size.left, boxy + hp_percent - 1,
				color_t(255, 255, 255, alpha), FONT_LEFT, hps);

	}
}


void CVisuals::Draw()
{



	if (!interfaces.engine->IsConnected() && !interfaces.engine->IsInGame())
		return;

	RecordInfo();



	

	static int Optimization;
	if (Optimization != vars.visuals.op_strength + 1 && vars.visuals.optimize && vars.visuals.To_Op & 1)
		goto skip;

	if (vars.visuals.enable)
	{
		for (auto entity : players) {

			if (entity == nullptr
				|| !entity->IsPlayer()
				|| entity == csgo->local
				|| entity->GetTeam() == csgo->local->GetTeam())
				continue;

			bool dormant = entity->IsDormant();
			if (vars.visuals.dormant)
			{
				if (!entity->isAlive())
				{
					// хули тут фейд был? чувак же сдох и все пиздец его могло показать в пизде карты
					csgo->EntityAlpha[entity->GetIndex()] = 0;
					continue;
				}
				strored_origin[entity->GetIndex()] = entity->GetRenderOrigin();
				if (dormant) {
					if (csgo->EntityAlpha[entity->GetIndex()] > 0)
						csgo->EntityAlpha[entity->GetIndex()] -= 2;

					if (csgo->EntityAlpha[entity->GetIndex()] <= 0)
						continue;
				}
				else
				{
					if (csgo->EntityAlpha[entity->GetIndex()] < 255)
						csgo->EntityAlpha[entity->GetIndex()] += 2;
				}

				csgo->EntityAlpha[entity->GetIndex()] = std::clamp(csgo->EntityAlpha[entity->GetIndex()], 0.f, 255.f);
			}
			else
			{
				if (!entity->isAlive())
					continue;

				if (dormant) {
					if (csgo->EntityAlpha[entity->GetIndex()] > 0)
						csgo->EntityAlpha[entity->GetIndex()] -= 2;
				}
				else
				{
					if (csgo->EntityAlpha[entity->GetIndex()] < 255)
						csgo->EntityAlpha[entity->GetIndex()] += 2;
				}
				csgo->EntityAlpha[entity->GetIndex()] = std::clamp(csgo->EntityAlpha[entity->GetIndex()], 0.f, 255.f);
				DrawFovArrows(entity, csgo->EntityAlpha[entity->GetIndex()], strored_origin[entity->GetIndex()]);

				if (dormant)
					continue;

				strored_origin[entity->GetIndex()] = entity->GetRenderOrigin();
			}

			DrawFovArrows(entity, csgo->EntityAlpha[entity->GetIndex()], strored_origin[entity->GetIndex()]);

			int alpha = vars.visuals.dormant ? csgo->EntityAlpha[entity->GetIndex()] : 255;

			auto weapon = entity->GetWeapon();
			if (!weapon)
				continue;

			//	DrawFovArrows(entity, (float)alpha, strored_origin[entity->GetIndex()]);

			BOX box;
			if (!GetBox(entity, box.x, box.y, box.w, box.h, strored_origin[entity->GetIndex()]))
				continue;
			/*
			if (!dormant && weapon->IsNade() && (*(bool*)(weapon + 0x33e2))) {
				auto defindex = weapon->GetItemDefinitionIndex();
				if (defindex != WEAPON_DECOY) {
					Vector Predicted = CEnemyGrenadePrediction::Get().Predict(entity->GetEyePosition(), entity->GetEyeAngles(), entity->GetVelocity(), (*((float*)(weapon + 0x33ec))), weapon);
					if (Predicted.IsValid()) {
						Vector Screen;
						if (Math::WorldToScreen(Predicted, Screen))
						{
							switch (weapon->GetItemDefinitionIndex()) {
							case WEAPON_SMOKEGRENADE:


								g_Render->DrawString(Screen.x, Screen.y, color_t(255, 255, 255, 255), render::centered_x | render::centered_y, fonts::ESPIcons2, "C");
								break;


							case WEAPON_MOLOTOV:
							case WEAPON_INCGRENADE:
								g_Render->DrawString(Screen.x, Screen.y, color_t(255, 107, 107, 255), render::centered_x | render::centered_y, fonts::ESPIcons2, "P");
								break;

							case WEAPON_FLASHBANG:
								g_Render->DrawString(Screen.x, Screen.y, color_t(255, 255, 255, 255), render::centered_x | render::centered_y, fonts::ESPIcons2, "I");
								break;
							case WEAPON_HEGRENADE:
								g_Render->DrawString(Screen.x, Screen.y, color_t(255, 107, 107, 255), render::centered_x | render::centered_y, fonts::ESPIcons2, "W");
								break;
							}
						}
					}
				}
			}
*/
			if (vars.visuals.box)
			{
				auto clr = dormant ? color_t(255, 255, 255, alpha - 50) : color_t(
					vars.visuals.box_color[0],
					vars.visuals.box_color[1],
					vars.visuals.box_color[2],
					alpha - 50);

				auto clr2 = dormant ? color_t(0, 0, 0, alpha - 50) : color_t(
					0,
					0,
					0,
					alpha - 50);

				Drawing::DrawOutlinedRect(box.x, box.y, box.w, box.h, clr);
				Drawing::DrawOutlinedRect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, clr2);
				Drawing::DrawOutlinedRect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, clr2);
				//Drawing::DrawOutlinedRect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, color_t(10, 10, 10, (alpha * 0.8f)));
				//Drawing::DrawOutlinedRect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, color_t(10, 10, 10, (alpha * 0.8f)));
			}

			int count = 0;
			if (vars.visuals.name)
			{
				auto text_size = Drawing::GetTextSize(fonts::esp_name, entity->GetName().c_str());

				auto clr = dormant ? color_t(255, 255, 255, 255) : color_t(255, 255, 255, 255);

				Drawing::DrawString(fonts::esp_name, box.x + box.w / 2 - text_size.right / 2, box.y - 10 - 3, clr, FONT_LEFT, entity->GetName().c_str());
				//g_Render->DrawGradient3DCircle(csgo->local->GetOrigin(), 15.f, color_t(210, 145, 255, 200), color_t(210, 145, 255, 0));
			}

			if (!dormant) {
				auto record = g_Animfix->get_latest_animation(entity);
				if (record.has_value() && record.value()->player) {
					auto clr = color_t(
						vars.visuals.skeleton_color[0],
						vars.visuals.skeleton_color[1],
						vars.visuals.skeleton_color[2],
						alpha - 50);

					if (vars.visuals.skeleton) {
						studiohdr_t* pStudioHdr = interfaces.models.model_info->GetStudioModel(entity->GetModel());

						if (!pStudioHdr)
							return;

						Vector vParent, vChild, sParent, sChild;

						for (int j = 0; j < pStudioHdr->numbones; j++)
						{
							mstudiobone_t* pBone = pStudioHdr->pBone(j);

							if (pBone && (pBone->flags & 0x100) && (pBone->parent != -1))
							{
								vChild = entity->GetBonePos(record.value()->bones, j);
								vParent = entity->GetBonePos(record.value()->bones, pBone->parent);

								if (Math::WorldToScreen(vParent, sParent) && Math::WorldToScreen(vChild, sChild))
								{
									Drawing::DrawLine(sParent[0], sParent[1], sChild[0], sChild[1], clr);
								}
							}
						}
					}
				}
			}

			if (vars.visuals.healthbar)
			{
				int hp = entity->GetHealth();

				if (hp > 100)
					hp = 100;

				int hp_percent = box.h - (int)((box.h * hp) / 100);

				int width = (box.w * (hp / 100.f));

				int red = 255 - (hp * 2.55);
				int green = hp * 2.55;

				char hps[10] = "";

				sprintf_s(hps, "%i", hp);

				auto text_size = Drawing::GetTextSize(fonts::esp_info, hps);

				auto clr = dormant ? color_t(200, 200, 200, alpha) : color_t(255, 255, 255, alpha);

				auto hp_color = dormant ? color_t(200, 200, 200, alpha) : vars.visuals.override_hp ? color_t(
					vars.visuals.hp_color[0],
					vars.visuals.hp_color[1],
					vars.visuals.hp_color[2],
					vars.visuals.hp_color[3] * (alpha / 255.f)) : color_t(red, green, 0, alpha);

				Drawing::DrawRect(box.x - 6, box.y - 1, 4, box.h + 2, color_t(80, 80, 80, alpha * 0.49f));
				//Drawing::DrawOutlinedRect(box.x - 6, box.y - 1, 4, box.h + 2, color_t(10, 10, 10, (alpha * 0.8f)));
				Drawing::DrawRect(box.x - 5, box.y + hp_percent, 2, box.h - hp_percent, hp_color);

				if (hp <= 99) // draws hp count at the same pos as top of bar
					Drawing::DrawString(fonts::esp_info, box.x - text_size.left, box.y + hp_percent - 1,
						color_t(255, 255, 255, alpha), FONT_LEFT, hps);

			}
			/*DrawAngleLine(strored_origin[entity->GetIndex()], resolver->GetAngle(entity), color_t(255, 255, 255, alpha));
			DrawAngleLine(strored_origin[entity->GetIndex()], resolver->GetForwardYaw(entity), color_t(60, 125, 245, alpha));
			DrawAngleLine(strored_origin[entity->GetIndex()], resolver->GetBackwardYaw(entity), color_t(0, 255, 0, alpha));*/

			if (weapon && !dormant)
			{
				if (weapon->IsGun())
				{

					auto ammo = weapon->GetAmmo(false);
					auto max_ammo = weapon->GetAmmo(true);
					if (vars.visuals.ammo)
					{
						auto clr = color_t(
							vars.visuals.ammo_color[0],
							vars.visuals.ammo_color[1],
							vars.visuals.ammo_color[2],
							vars.visuals.ammo_color[3] * (alpha / 255.f));

						int hp_percent = box.w - (int)((box.w * ammo) / 100);

						int width = (box.w * (ammo / float(max_ammo)));

						char ammostr[10];
						sprintf_s(ammostr, "%d", ammo);

						const auto text_size = Drawing::GetTextSize(fonts::esp_info, ammostr);

						Drawing::DrawRect(box.x, box.y + 2 + box.h, box.w + 1, 4, color_t(80, 80, 80, alpha * 0.49f));
						//Drawing::DrawOutlinedRect(box.x, box.y + 2 + box.h, box.w + 1, 4, color_t(10, 10, 10, (alpha * 0.8f)));
						Drawing::DrawRect(box.x + 1, box.y + 3 + box.h, width - 1, 2, clr);

						if (ammo < (max_ammo / 2) && ammo > 0)
							Drawing::DrawString(fonts::esp_name, box.x + width - 1 - text_size.right, box.y + box.h,
								color_t(255, 255, 255, alpha - 55.f), FONT_LEFT, ammostr);

						count++;
					}
				}
				if (vars.visuals.weapon)
				{
					auto clr = vars.visuals.weapon_color;

					auto weap_info = entity->GetWeapon()->GetCSWpnData();

					char wpn_name[100] = "";

					sprintf_s(wpn_name, "%s", str_toupper(weap_info->m_szWeaponName).c_str() + 7);

					if (entity->GetWeapon()->GetItemDefinitionIndex() == 64)
						strcpy_s(wpn_name, "REVOLVER");

					auto wpn_name_size = Drawing::GetTextSize(fonts::esp_info, wpn_name);

					Drawing::DrawString(fonts::esp_info, box.x + box.w / 2 - wpn_name_size.right / 2, box.y + 1 + box.h + (count++ * 6.f),
						clr, FONT_LEFT, wpn_name);
					count++;

				}
				if (vars.visuals.weapon_icon)
				{




					auto icon = GetIconChar(entity->GetWeapon());

					auto wpn_name_sizei = Drawing::GetTextSize(fonts::esp_icon, icon);
					Drawing::DrawString(fonts::esp_icon, box.x + box.w / 2 - wpn_name_sizei.right / 2, box.y + 2 + box.h + (count * 6.f) - 5, color_t(15, 15, 15, 115), FONT_LEFT, icon);
					Drawing::DrawString(fonts::esp_icon, box.x + box.w / 2 - wpn_name_sizei.right / 2, box.y + 1 + box.h + (count++ * 6.f) - 6, color_t(255, 255, 255), FONT_LEFT, icon);

					count++;
				}

			}

			int offset = 0;
			if (weapon && !dormant)
			{
				auto clr = vars.visuals.flags_color;

				if (vars.visuals.flags & 1 && entity->GetArmor() > 0) {
					if (entity->HasHelmet())
						Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT,
							"HK");
					else
						Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, "K");
				}
				if (vars.visuals.flags & 2 && entity->IsScoped()) {
					Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, "ZOOM");
				}
				if (vars.visuals.flags & 4 && entity->IsFlashed() > 0) {
					Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, "BLIND");
				}
				if (vars.visuals.flags & 8 && csgo->hitchance) {
					Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, "HIT");
					csgo->hitchance = false;
				}

				if (vars.visuals.flags & 16 && vars.ragebot.resolver != 2 && ResolverMode[entity->EntIndex()] != "") {
					Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, ResolverMode[entity->EntIndex()].c_str());
				}
				if (vars.visuals.flags & 32 && vars.ragebot.resolver != 2) {
					auto str = "CHOKE: " + std::to_string(entity->GetChokedPackets());
					Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, str.c_str());
				}
#ifdef _DEBUG
				if (vars.visuals.dormant && dormant)
				{
					Drawing::DrawString(fonts::esp_info, box.x + box.w + 3, box.y + offset++ * 9, clr, FONT_LEFT, "DORMANT");
				}
#endif
			}
		}
		//DrawAngleLine(csgo->local->GetRenderOrigin(), csgo->local->GetLBY(), color_t(255, 255, 255, 255));
	}


skip:
	if (Optimization > 3)
		Optimization = 0;
	Optimization++;

}

bool CVisuals::IsValidSound(SndInfo_t& sound) {
	// We don't want the sound to keep following client's predicted origin.
	for (int iter = 0; iter < m_utlvecSoundBuffer.Count(); iter++)
	{
		SndInfo_t& cached_sound = m_utlvecSoundBuffer[iter];
		if (cached_sound.m_nGuid == sound.m_nGuid)
		{
			return false;
		}
	}

	return true;
}

void CVisuals::OnDormant() {
	if (!interfaces.engine->IsInGame() || !csgo->local || !csgo->local->isAlive() || csgo->disable_dt)
		return;

	m_utlCurSoundList.RemoveAll();
	interfaces.engine_sound->GetActiveSounds(m_utlCurSoundList);

	// No active sounds.
	if (!m_utlCurSoundList.Count())
		return;

	// Accumulate sounds for esp correction
	for (int iter = 0; iter < m_utlCurSoundList.Count(); iter++)
	{
		SndInfo_t& sound = m_utlCurSoundList[iter];
		if (sound.m_nSoundSource == 0 || // World
			sound.m_nSoundSource > 64)   // Most likely invalid
			continue;

		auto player = interfaces.ent_list->GetClientEntity(sound.m_nSoundSource);

		if (!player || player == csgo->local || sound.m_pOrigin->IsZero())
			continue;

		if (!IsValidSound(sound))
			continue;

		AdjustDormant(player, sound);

		csgo->LastSeenTime[player->GetIndex()] = csgo->get_absolute_time();

		m_cSoundPlayers[sound.m_nSoundSource - 1].Override(sound);
	}

	m_utlvecSoundBuffer = m_utlCurSoundList;
}

void CVisuals::AdjustDormant(IBasePlayer* player, SndInfo_t& sound)
{
	Vector src3D, dst3D;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = player;
	src3D = (*sound.m_pOrigin) + Vector(0, 0, 1); // So they dont dig into ground incase shit happens /shrug
	dst3D = src3D - Vector(0, 0, 100);
	ray.Init(src3D, dst3D);

	interfaces.trace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &tr);

	// step = (tr.fraction < 0.20)
	// shot = (tr.fraction > 0.20)
	// stand = (tr.fraction > 0.50)
	// crouch = (tr.fraction < 0.50)

	/* Corrects origin and important flags. */

	// Player stuck, idk how this happened
	if (tr.allsolid)
	{
		m_cSoundPlayers[sound.m_nSoundSource - 1].m_iReceiveTime = -1;
	}

	*sound.m_pOrigin = ((tr.fraction < 0.97) ? tr.endpos : *sound.m_pOrigin);
	m_cSoundPlayers[sound.m_nSoundSource - 1].m_nFlags = player->GetFlags();
	m_cSoundPlayers[sound.m_nSoundSource - 1].m_nFlags |= (tr.fraction < 0.50f ? FL_DUCKING : 0) | (tr.fraction != 1 ? FL_ONGROUND : 0);   // Turn flags on
	m_cSoundPlayers[sound.m_nSoundSource - 1].m_nFlags &= (tr.fraction > 0.50f ? ~FL_DUCKING : 0) | (tr.fraction == 1 ? ~FL_ONGROUND : 0); // Turn flags off
}

void CVisuals::RecordInfo()
{
	players.clear();
	for (auto i = 0; i < interfaces.global_vars->maxClients; i++)
	{
		IBasePlayer* entity = interfaces.ent_list->GetClientEntity(i);
		if (!entity)
			continue;

		players.push_back(entity);
	}
}

void CGrenadePrediction::Tick(int buttons)
{
	if (!vars.visuals.nadepred)
		return;
	bool in_attack = buttons & IN_ATTACK;
	bool in_attack2 = buttons & IN_ATTACK2;

	//g_pICvar->Consolecolor_tPrintf(SDK::color_t(255, 255, 255, 255), "OView");

	act = (in_attack && in_attack2) ? ACT_LOB :
		(in_attack2) ? ACT_DROP :
		(in_attack) ? ACT_THROW :
		ACT_NONE;
}
void CGrenadePrediction::View(CViewSetup* setup)
{
	if (!vars.visuals.nadepred)
		return;

	if (csgo->local && csgo->local->isAlive())
	{
		IBaseCombatWeapon* weapon = csgo->local->GetWeapon();

		if (weapon && weapon->IsNade())
		{
			type = weapon->GetItemDefinitionIndex();
			Simulate(setup);
		}
		else
		{
			type = 0;
		}
	}
}

void CGrenadePrediction::Paint(IDirect3DDevice9* pDevice)
{


	if (!vars.visuals.nadepred)
		return;

	if ((type) && !(path.empty()))
	{
		Vector nadeStart, nadeEnd;
		Vector nadeStart1, nadeEnd1;

		Vector prev = path[0];

		for (auto it = path.begin(), end = path.end(); it != end; ++it)
		{
			if (Math::WorldToScreen(prev, nadeStart) && Math::WorldToScreen(*it, nadeEnd))
			{
				g_Render->DrawLine(nadeStart.x, nadeStart.y, nadeEnd.x, nadeEnd.y, color_t(vars.visuals.nadepred_color[0],
					vars.visuals.nadepred_color[1],
					vars.visuals.nadepred_color[2],
					50), 3.f);

				g_Render->DrawLine(nadeStart.x + 1, nadeStart.y + 1, nadeEnd.x + 1, nadeEnd.y + 1, color_t(vars.visuals.nadepred_color[0],
					vars.visuals.nadepred_color[1],
					vars.visuals.nadepred_color[2],
					50), 3.f);
			}
			prev = *it;
		}

		trace_t endp;
		VSTraceLine(prev, prev - Vector(0, 0, 1000), MASK_SOLID, csgo->local, &endp);
		//g_Render->Render3DCircle(pDevice,prev, 75, vars.visuals.nadepred_color);
		static auto FilledCircle = [](Vector location, float radius, color_t Col, float pAlpha) {
			static constexpr float Step = PI * 2.0f / 60;
			std::vector<ImVec2> points;
			for (float lat = 0.f; lat <= PI * 2.0f; lat += Step)
			{
				const auto& point3d = Vector(sin(lat), cos(lat), 0.f) * radius;
				Vector point2d;
				if (Math::WorldToScreen(location + point3d, point2d))
					points.push_back(ImVec2(point2d.x, point2d.y));
			}
			g_Render->_drawList->AddConvexPolyFilled(points.data(), points.size(), color_t(Col[0],
				Col[1], Col[2], (pAlpha / 255) * 80).u32());
			g_Render->_drawList->AddPolyline(points.data(), points.size(), color_t(Col[0],
				Col[1], Col[2], (pAlpha / 255) * 120).u32(), true, 2.f);
		};
		FilledCircle(prev, 75.f, vars.visuals.nadepred_color, 255.f);
		g_Render->Gradient3DCircleOp(pDevice, endp.endpos, 75, color_t(vars.visuals.nadepred_color[0], vars.visuals.nadepred_color[1], vars.visuals.nadepred_color[2], 0), color_t(vars.visuals.nadepred_color[0], vars.visuals.nadepred_color[1], vars.visuals.nadepred_color[2], 90));
		//Drawing::DrawFilledCircle(nadeEnd.x, nadeEnd.y, 5, 100, color_t(0, 0, 0, 200));
		//Drawing::DrawFilledCircle(nadeEnd.x, nadeEnd.y, 4, 100, color_t(60, 125, 245, 200));
	}
}
const auto PIRAD = 0.01745329251f;
void angle_vectors2(const Vector& angles, Vector* forward, Vector* right, Vector* up)
{
	float sr, sp, sy, cr, cp, cy;

	sp = static_cast<float>(sin(double(angles.x) * PIRAD));
	cp = static_cast<float>(cos(double(angles.x) * PIRAD));
	sy = static_cast<float>(sin(double(angles.y) * PIRAD));
	cy = static_cast<float>(cos(double(angles.y) * PIRAD));
	sr = static_cast<float>(sin(double(angles.z) * PIRAD));
	cr = static_cast<float>(cos(double(angles.z) * PIRAD));

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}
Vector CEnemyGrenadePrediction::Predict(Vector EyePos, Vector EyeAngle, Vector velocity, float throwstrength, IBaseCombatWeapon* weap) {
	

	static auto CheckDetonate = [](const Vector& vecThrow, const trace_t& tr, int tick, float interval, int itemdef) -> bool
	{
		static auto DetonateTime = interfaces.cvars->FindVar("molotov_throw_detonate_time");
		switch (itemdef)
		{
		case WEAPON_SMOKEGRENADE:
		case WEAPON_DECOY:
			// Velocity must be <0.1, this is only checked every 0.2s
			if (vecThrow.Length2D() < 0.1f)
			{
				int det_tick_mod = static_cast<int>(0.2f / interval);
				return !(tick % det_tick_mod);
			}
			return false;

		case WEAPON_MOLOTOV:
		case WEAPON_INCGRENADE:
			// Detonate when hitting the floor
			if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
				return true;
			// OR we've been flying for too long

		case WEAPON_FLASHBANG:
		case WEAPON_HEGRENADE:
	
			return static_cast<float>(tick) * interval > DetonateTime->GetFloat() && !(tick % static_cast<int>(0.2f / interval));

	
		}
	};
	static auto Step = [&](Vector & origin, Vector & vel, int tick, float frameinterval, int itemdef) -> bool {
	
		Vector move;
		
		SetupMove(move, vel, frameinterval);

	
		trace_t tr;
		Vector vecAbsEnd = origin;
		vecAbsEnd += move;

		TraceHull(origin, vecAbsEnd, tr);

	

		if (CheckDetonate(vel, tr, tick, frameinterval, itemdef)) {
			if (itemdef == WEAPON_MOLOTOV || itemdef == WEAPON_INCGRENADE) {
				Vector vecAbsEnd = origin;
				vecAbsEnd += Vector(0,0,-200);
				TraceHull(origin, vecAbsEnd, tr);
				if (tr.fraction != 1.0f)
				{
					origin = tr.endpos;
				}
			}
			return true;
		}


		if (tr.fraction != 1.0f)
		{

			ResolveCollision(tr, vel, frameinterval);
		}


		origin = tr.endpos;

		return false;
	};
	static auto SetupNade = [](Vector& vecSrc, Vector& vecThrow, Vector viewangles, float throwstrength) -> void {


		Vector angThrow = viewangles;
		float pitch = angThrow.x;

		if (pitch <= 90.0f)
		{
			if (pitch < -90.0f)
			{
				pitch += 360.0f;
			}
		}
		else
		{
			pitch -= 360.0f;
		}
		float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
		angThrow.x = a;


		float flVel = 750.0f * 0.9f;



		float p = throwstrength;
		float b = p;
		// Clamped to [0,1]
		b = b * 0.7f;
		b = b + 0.3f;
		flVel *= b;

		Vector vForward, vRight, vUp;
		angle_vectors2(angThrow, &vForward, &vRight, &vUp); 


		float off = (p * 12.0f) - 12.0f;
		vecSrc.z += off;


		trace_t tr;
		Vector vecDest = vecSrc;
		vecDest += vForward * 22.0f;

		TraceHull(vecSrc, vecDest, tr);

		Vector vecBack = vForward; vecBack *= 6.0f;
		vecSrc = tr.endpos;
		vecSrc -= vecBack;


		vecThrow *= 1.25f;
		vecThrow += vForward * flVel; 
	};

	Vector Origin = EyePos;
	Vector Velocity = velocity;
	Vector Forward = EyeAngle;
	SetupNade(Origin, Velocity, Forward, throwstrength);

	float interval = interfaces.global_vars->interval_per_tick;

	auto weapondef = weap->GetItemDefinitionIndex();
	for (unsigned int i = 0; i < 2000; ++i)
	{


		if (Step(Origin, Velocity, i, interval, weapondef))
			break;
	}
	

	return Origin;
}
void CGrenadePrediction::Setup(Vector& vecSrc, Vector& vecThrow, Vector viewangles)
{
	if (!vars.visuals.nadepred)
		return;

	Vector angThrow = viewangles;
	float pitch = angThrow.x;

	if (pitch <= 90.0f)
	{
		if (pitch < -90.0f)
		{
			pitch += 360.0f;
		}
	}
	else
	{
		pitch -= 360.0f;
	}
	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.x = a;

	// Gets ThrowVelocity from weapon files
	// Clamped to [15,750]
	float flVel = 750.0f * 0.9f;

	// Do magic on member of grenade object [esi+9E4h]
	// m1=1  m1+m2=0.5  m2=0
	
	float p = *((float*)(csgo->weapon + 0x33ec));
	float b = p;
	// Clamped to [0,1]
	b = b * 0.7f;
	b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	angle_vectors2(angThrow, &vForward, &vRight, &vUp); //angThrow.ToVector(vForward, vRight, vUp);

	vecSrc = csgo->unpred_eyepos;
	float off = (p * 12.0f) - 12.0f;
	vecSrc.z += off;

	// Game calls UTIL_TraceHull here with hull and assigns vecSrc tr.endpos
	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f; //vecDest.MultAdd(vForward, 22.0f);

	TraceHull(vecSrc, vecDest, tr);

	// After the hull trace it moves 6 units back along vForward
	// vecSrc = tr.endpos - vForward * 6
	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	// Finally calculate velocity
	vecThrow = csgo->local->GetVelocity(); vecThrow *= 1.25f;
	vecThrow += vForward * flVel; //	vecThrow.MultAdd(vForward, flVel);
}

void CGrenadePrediction::Simulate(CViewSetup* setup)
{
	if (!vars.visuals.nadepred)
		return;
	Vector vecSrc, vecThrow;
	Vector angles; interfaces.engine->GetViewAngles(angles);
	Setup(vecSrc, vecThrow, angles);

	float interval = interfaces.global_vars->interval_per_tick;

	// Log positions 20 times per sec
	int logstep = static_cast<int>(0.05f / interval);
	int logtimer = 0;


	path.clear();
	for (unsigned int i = 0; i < path.max_size() - 1; ++i)
	{
		if (!logtimer)
			path.push_back(vecSrc);

		int s = Step(vecSrc, vecThrow, i, interval);
		if ((s & 1)) break;

		// Reset the log timer every logstep OR we bounced
		if ((s & 2) || logtimer >= logstep) logtimer = 0;
		else ++logtimer;
	}
	path.push_back(vecSrc);
}

int CGrenadePrediction::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
{

	// Apply gravity
	Vector move;
	AddGravityMove(move, vecThrow, interval, false);

	// Push entity
	trace_t tr;
	PushEntity(vecSrc, move, tr);

	int result = 0;
	// Check ending conditions
	if (CheckDetonate(vecThrow, tr, tick, interval))
	{
		if (type == WEAPON_MOLOTOV || type == WEAPON_INCGRENADE) {
			vecSrc = tr.endpos;
			vecThrow.Zero();
			trace_t tr;
			PushEntity(vecSrc, Vector(0,0,-200), tr);
			if (tr.fraction != 1.0f)
			{
				vecSrc = tr.endpos;
			}
		}
		result |= 1;
		return result;
	}

	// Resolve collisions
	if (tr.fraction != 1.0f)
	{
		result |= 2; // Collision!
		ResolveFlyCollisionCustom(tr, vecThrow, interval);
	}

	// Set new position
	vecSrc = tr.endpos;

	return result;
}


bool CGrenadePrediction::CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
{
	static auto DetonateTime = interfaces.cvars->FindVar("molotov_throw_detonate_time");
	switch (type)
	{
	case WEAPON_SMOKEGRENADE:
	case WEAPON_DECOY:
		// Velocity must be <0.1, this is only checked every 0.2s
		if (vecThrow.Length2D() < 0.1f)
		{
			int det_tick_mod = static_cast<int>(0.2f / interval);
			return !(tick % det_tick_mod);
		}
		return false;

	case WEAPON_MOLOTOV:
	case WEAPON_INCGRENADE:
		// Detonate when hitting the floor
		if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
			return true;
		// OR we've been flying for too long

	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		// Pure timer based, detonate at 1.5s, checked every 0.2s
		return static_cast<float>(tick) * interval > DetonateTime->GetFloat() && !(tick % static_cast<int>(0.2f / interval));

	default:
		assert(false);
		return false;
	}
}

void CGrenadePrediction::TraceHull(Vector& src, Vector& end, trace_t& tr)
{
	if (!vars.visuals.nadepred)
		return;
	Ray_t ray;
	ray.Init(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));

	CTraceFilterWorldAndPropsOnly filter;
	//filter.SetIgnoreClass("BaseCSGrenadeProjectile");
	//filter.bShouldHitPlayers = false;

	interfaces.trace->TraceRay(ray, 0x200400B, &filter, &tr);
}

void CGrenadePrediction::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	if (!vars.visuals.nadepred)
		return;
	Vector basevel(0.0f, 0.0f, 0.0f);

	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if (onground)
	{
		move.z = (vel.z + basevel.z) * frametime;
	}
	else
	{
		// Game calls GetActualGravity( this );
		static auto Grav = interfaces.cvars->FindVar("sv_gravity");

		float gravity = Grav->GetFloat() * 0.4f;
		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;

		vel.z = newZ;
	}
}

void CGrenadePrediction::PushEntity(Vector& src, const Vector& move, trace_t& tr)
{
	if (!vars.visuals.nadepred)
		return;
	Vector vecAbsEnd = src;
	vecAbsEnd += move;

	// Trace through world
	TraceHull(src, vecAbsEnd, tr);
}

void CGrenadePrediction::ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval)
{
	if (!vars.visuals.nadepred)
		return;
	// Calculate elasticity
	float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
	float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	// Calculate bounce
	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	// Stop completely once we move too slow
	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
	if (flSpeedSqr < flMinSpeedSqr)
	{
		//vecAbsVelocity.Zero();
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	// Stop if on ground
	if (tr.plane.normal.z > 0.7f)
	{
		float speed = vecAbsVelocity.LengthSqr();


		if (speed > 96000.f) {

			static auto NormalizeVector = [](Vector vec) {

				Vector res = vec;
				res /= (res.Length() + std::numeric_limits< float >::epsilon());
				return res;

			};
			float len = vecAbsVelocity.normalized().Dot(tr.plane.normal);
			if (len > 0.5f)
				vecAbsVelocity *= 1.5f - len;


		}


		if (speed < 400.f) {
			vecAbsVelocity.Zero();
		}
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval); //vecAbsVelocity.Mult((1.0f - tr.fraction) * interval);
		PushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
	{
		vecVelocity = vecAbsVelocity;
	}
}

int CGrenadePrediction::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float    backoff;
	float    change;
	float    angle;
	int        i, blocked;

	blocked = 0;

	angle = normal[2];

	if (angle > 0)
	{
		blocked |= 1;        // floor
	}
	if (!angle)
	{
		blocked |= 2;        // step
	}

	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
		{
			out[i] = 0;
		}
	}

	return blocked;
}

void CVisuals::NightMode()
{
	static auto load_named_sky = reinterpret_cast<void(__fastcall*)(const char*)>(
		csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
			hs::load_named_sky::s().c_str()));

	static auto sv_skyname = interfaces.cvars->FindVar(hs::sv_skyname::s().c_str());
	sv_skyname->m_fnChangeCallbacks.m_Size = 0;

	static auto r_3dsky = interfaces.cvars->FindVar(hs::r_3dsky::s().c_str());
	r_3dsky->m_fnChangeCallbacks.m_Size = 0;
	r_3dsky->SetValue(0);

	static auto backup = sv_skyname->GetString();

	const auto reset = [&]()
	{
		load_named_sky(hs::sky_dust::s().c_str());

		for (auto i = interfaces.material_system->FirstMaterial(); i != interfaces.material_system->InvalidMaterial(); i = interfaces.material_system->NextMaterial(i))
		{
			auto mat = interfaces.material_system->GetMaterial(i);
			if (!mat)
				continue;

			if (mat->IsErrorMaterial())
				continue;

			std::string name = mat->GetName();
			auto tex_name = mat->GetTextureGroupName();


			if (strstr(tex_name, hs::World::s().c_str()) || strstr(tex_name, hs::StaticProp::s().c_str()) || strstr(tex_name, hs::SkyBox::s().c_str()))
			{
				mat->ColorModulate(1.f, 1.f, 1.f);
				mat->AlphaModulate(1.f);
			}
		}
	};

	static bool Reset = false;
	static bool Reset2 = false;
	static int CurrentSky = 0;

	if (vars.visuals.nightmode)
	{
		Reset = false;
		switch (vars.misc.skyboxx)
		{
		case 0:	load_named_sky("sky_descent"); break;
		case 1:	load_named_sky("cs_tibet"); break;
		case 2:	load_named_sky("sky_cs15_daylight03_hdr"); break;
		case 3:	load_named_sky("sky_csgo_night02"); break;
		case 4:	load_named_sky("sky_csgo_night02b"); break;
		case 5:	load_named_sky("sky_csgo_night_flat"); break;
		case 6:	load_named_sky("sky_csgo_cloudy01"); break;
		case 7:	load_named_sky("vertigoblue_hdr"); break;
		case 8:	load_named_sky("vietnam"); break;

		}



		static float OldClr[3];
		static float OldClr1[3];
		static float OldClr2[3];

		if (csgo->mapChanged || !Reset2 || OldClr[0] != vars.visuals.nightmode_color[0]
			|| OldClr[1] != vars.visuals.nightmode_color[1]
			|| OldClr[2] != vars.visuals.nightmode_color[2])
		{
			for (auto i = interfaces.material_system->FirstMaterial(); i != interfaces.material_system->InvalidMaterial(); i = interfaces.material_system->NextMaterial(i))
			{
				auto mat = interfaces.material_system->GetMaterial(i);
				if (!mat)
					continue;

				auto tex_name = mat->GetTextureGroupName();

				if (strstr(tex_name, hs::World::s().c_str()))
				{
					mat->ColorModulate(vars.visuals.nightmode_color[0] / 255.f,
						vars.visuals.nightmode_color[1] / 255.f, vars.visuals.nightmode_color[2] / 255.f);
				}
			}

			OldClr[0] = vars.visuals.nightmode_color[0];
			OldClr[1] = vars.visuals.nightmode_color[1];
			OldClr[2] = vars.visuals.nightmode_color[2];
			csgo->mapChanged = false;
			Reset2 = true;
		}

		if (csgo->mapChanged || !Reset2 || OldClr1[0] != vars.visuals.nightmode_prop_color[0]
			|| OldClr1[1] != vars.visuals.nightmode_prop_color[1]
			|| OldClr1[2] != vars.visuals.nightmode_prop_color[2])
		{
			for (auto i = interfaces.material_system->FirstMaterial(); i != interfaces.material_system->InvalidMaterial(); i = interfaces.material_system->NextMaterial(i))
			{
				auto mat = interfaces.material_system->GetMaterial(i);
				if (!mat)
					continue;

				auto tex_name = mat->GetTextureGroupName();

				if (strstr(tex_name, hs::StaticProp::s().c_str()))
				{
					mat->ColorModulate(vars.visuals.nightmode_prop_color[0] / 255.f,
						vars.visuals.nightmode_prop_color[1] / 255.f, vars.visuals.nightmode_prop_color[2] / 255.f);
				}
			}

			OldClr1[0] = vars.visuals.nightmode_prop_color[0];
			OldClr1[1] = vars.visuals.nightmode_prop_color[1];
			OldClr1[2] = vars.visuals.nightmode_prop_color[2];
			csgo->mapChanged = false;
			Reset2 = true;
		}

		if (csgo->mapChanged || !Reset2 || OldClr2[0] != vars.visuals.nightmode_skybox_color[0]
			|| OldClr2[1] != vars.visuals.nightmode_skybox_color[1]
			|| OldClr2[2] != vars.visuals.nightmode_skybox_color[2])
		{
			for (auto i = interfaces.material_system->FirstMaterial(); i != interfaces.material_system->InvalidMaterial(); i = interfaces.material_system->NextMaterial(i))
			{
				auto mat = interfaces.material_system->GetMaterial(i);
				if (!mat)
					continue;

				auto tex_name = mat->GetTextureGroupName();

				if (strstr(tex_name, hs::SkyBox::s().c_str()))
				{
					mat->ColorModulate(vars.visuals.nightmode_skybox_color[0] / 255.f,
						vars.visuals.nightmode_skybox_color[1] / 255.f, vars.visuals.nightmode_skybox_color[2] / 255.f);
				}
			}

			OldClr2[0] = vars.visuals.nightmode_skybox_color[0];
			OldClr2[1] = vars.visuals.nightmode_skybox_color[1];
			OldClr2[2] = vars.visuals.nightmode_skybox_color[2];
			csgo->mapChanged = false;
			Reset2 = true;
		}
	}
	else
	{
		Reset2 = false;

		if (!Reset)
		{
			reset();
			Reset = true;
		}
		csgo->mapChanged = false;
	}
	/*
	if (CurrentSky != vars.misc.skyboxx)
	{
		Reset2 = false;

		if (!Reset)
		{
			reset();
			Reset = true;
		}
		csgo->mapChanged = false;
		CurrentSky = vars.misc.skyboxx;
	}*/
}