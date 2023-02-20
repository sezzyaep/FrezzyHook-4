#pragma once

#include "Hooks.h"

enum GrenadePredictionType
{
	GRENADE_INC,
	GRENADE_HE
};



class AGrenadePrediction : public Singleton<AGrenadePrediction>
{
	void STraceLine(Vector& start, Vector& end, unsigned int mask, IBasePlayer* ignore, trace_t* trace)
	{
		Ray_t ray;
		ray.Init(start, end);

		CTraceFilter filter;
		filter.pSkip = ignore;

		interfaces.trace->TraceRay(ray, mask, &filter, trace);
	}

	void PTraceHull(Vector& src, Vector& end, trace_t& tr)
	{

		Ray_t ray;
		ray.Init(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));

		CTraceFilterWorldAndPropsOnly filter;

		interfaces.trace->TraceRay(ray, 0x200400B, &filter, &tr);
	}

	void PGravity(Vector& move, Vector& vel, float frametime, bool onground)
	{

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
			float gravity = 800.0f * 0.4f;

			float newZ = vel.z - (gravity * frametime);
			move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;

			vel.z = newZ;
		}
	}


	Vector PredictNade(Vector Origin, Vector Velocity, GrenadePredictionType type);

}
