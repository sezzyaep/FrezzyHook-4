#pragma once
#include "AnimationFix.h"



class Miscellaneous : public Singleton<Miscellaneous>
{
public:
	void AutoPeek(CUserCmd* cmd);
	bool Run;
	bool Init;
	bool DT;
	Vector AutoPeekStored;
};
extern Miscellaneous g_Miscellaneous;