#pragma once
#include "Hooks.h"
#include "RageBackTracking.h"
#include "Hitmarker.h"
struct GrenadeInfo_t {
	Vector ViewAngles;
	std::vector<Vector> GrenadePath;
	int Type;
	int Act;
	bool Init;
	Vector ThrowPos;
	Vector ThrowVelocity;

};
class PGrenadeWarning : public Singleton<PGrenadeWarning>
{
private:
	
	

	void Setup(GrenadeInfo_t& info, Vector& vecSrc, Vector& vecThrow, Vector viewangles);
	void Simulate(GrenadeInfo_t& info, CViewSetup* setup);

	int Step(GrenadeInfo_t& info, Vector& vecSrc, Vector& vecThrow, int tick, float interval);
	bool CheckDetonate(GrenadeInfo_t& info, const Vector& vecThrow, const trace_t& tr, int tick, float interval);
	void UpdatePath(GrenadeInfo_t& info,Vector& out);
	void TraceHull(Vector& src, Vector& end, trace_t& tr);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void PushEntity(GrenadeInfo_t& info, Vector& src, const Vector& move, trace_t& tr);
	void ResolveFlyCollisionCustom(GrenadeInfo_t& info, trace_t& tr, Vector& vecVelocity, float interval);
	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
public:
	void Tick(GrenadeInfo_t& info);
	void View(GrenadeInfo_t& info, CViewSetup* setup);
	GrenadeInfo_t GrenadeInfo[500];
	void Paint();
};


