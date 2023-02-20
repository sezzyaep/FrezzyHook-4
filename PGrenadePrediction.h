#pragma once
#include "Hooks.h"




class GrenadePrediction : public Singleton<GrenadePrediction>
{
private:
	std::vector<Vector> path[65];
	std::vector<std::pair<Vector, color_t>> others;
	int type[99999];
	int act = 0;

	void Setup(Vector& vecSrc, Vector& vecThrow, const Vector& viewangles, IBasePlayer* entitytodraw);
	void Simulate(CViewSetup* setup, IBasePlayer* entitytodraw);

	int Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval, IBasePlayer* entitytodraw);
	bool CheckDetonate(Vector& vecThrow, const trace_t& tr, int tick, float interval, IBasePlayer* entitytodraw);

	void TraceHull(Vector& src, Vector& end, trace_t& tr);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void PushEntity(Vector& src, const Vector& move, trace_t& tr);
	void ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, const Vector& move, float interval);
	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
public:
	void Tick(int buttons);
	void View(CViewSetup* setup, IBaseCombatWeapon* weapon, IBasePlayer* entitytodraw, int type2332);
	void Paint(IBasePlayer* entitytodraw);
	bool AlreadyFirst[99999];
	bool AlreadyEverythink[99999];
	Vector eyeangle[99999];
	Vector position[99999];
	Vector position3[99999];
	Vector VecVelocity[99999];
	float firstthrowtime[99999];
	Vector position2[99999];
};

class throwngrenade_t {

public:
	Vector m_vecnadebeginforpaint;
	Vector m_veceyeang;
	Vector m_vecvelocity;
	float m_flvelocitynade;
	Vector m_vecnadeendforpaint;
	float m_flthrowtime;
	bool m_bAlreadySet;
	int m_itemdefindx;
	float m_fllife;
	float m_fldistance;
};