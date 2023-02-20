#include "miscellaneous.h"

bool InRangeVector2D(Vector input, Vector start, Vector end)
{
	return(input.x > start.x && input.y > start.y && input.x > end.x && input.y > end.y);
}

void MovePlayerTo(Vector MoveTo, CUserCmd* cmd) {
	auto localPlayer = csgo->local;
	if (!localPlayer || localPlayer->IsDormant() || !localPlayer->isAlive()) return;
	Vector playerLoc = localPlayer->GetAbsOrigin();

	float yaw = cmd->viewangles.y;
	Vector VecForward = playerLoc - MoveTo;

	Vector translatedVelocity = Vector{
		(float)(VecForward.x * cos(yaw / 180 * (float)PI) + VecForward.y * sin(yaw / 180 * (float)PI)),
		(float)(VecForward.y * cos(yaw / 180 * (float)PI) - VecForward.x * sin(yaw / 180 * (float)PI)),
		VecForward.z
	};
	cmd->forwardmove = -translatedVelocity.x * 20.f;
	cmd->sidemove = translatedVelocity.y * 20.f;
}

void Miscellaneous::AutoPeek(CUserCmd* cmd)
{


	if (vars.misc.AutoPeek->active && !Init)
	{
		AutoPeekStored = csgo->local->GetAbsOrigin();
		Init = true;
	}
	if (!vars.misc.AutoPeek->active)
	{
		Init = false;
		return;
	}
	if (cmd->buttons & IN_ATTACK && Init)
	{
		Run = true;
	}
	Vector InRangeInaccuracy;
	InRangeInaccuracy.x = 3;
	InRangeInaccuracy.y = 3;
	if (Run && !InRangeVector2D(csgo->local->GetAbsOrigin(), AutoPeekStored - InRangeInaccuracy, AutoPeekStored + InRangeInaccuracy))
	{
		MovePlayerTo(AutoPeekStored, cmd);
	}
	else if (Run && InRangeVector2D(csgo->local->GetAbsOrigin(), AutoPeekStored - InRangeInaccuracy, AutoPeekStored + InRangeInaccuracy))
	{
		Run = false;
		Init = false;
	
	}
	
}