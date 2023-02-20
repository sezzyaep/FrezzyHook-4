#include "Autowall.h"

struct PLegitConfigInfo
{
	bool Enabled;
	bool BShots;
	bool IgnoreFlash;
	bool KeyActive;
	bool Bind;
	bool AutoFire;
	int LockOnType;
	int MaxInnacuracy;
	bool ScopedOnly;
	int FOV;
	int bone;
};

class PLegitbot : public Singleton<PLegitbot> {
public:
	void TraceLineNonMemoryIntensive(Vector& start, Vector& end, unsigned int mask, IBasePlayer* ignore, trace_t* trace);
	bool IsBreakableEntity(IBasePlayer* e);
	bool PTraceToExit(trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace);
	float LegitHandleBulletPenetration(surfacedata_t* enterSurfaceData, trace_t& enterTrace, const Vector& direction, Vector& result, float penetration, float damage);
	bool CanScanEntity(IBasePlayer* entity, const Vector& destination, int minDamage);
	float GetMultiplier(int hitGroup);
	bool IsPropArmour(int H, bool G);
	void DScaleDamage(IBasePlayer* e, CCSWeaponInfo* weapon_info, int hitgroup, float& current_damage);
	PLegitConfigInfo GetConfig();
	bool NeedRecoilControl(IBaseCombatWeapon* weapon);
	Vector GetPoint(IBasePlayer* pBaseEntity, int iHitbox, matrix BoneMatrix[128]);
	Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles);
	bool IsVisible(IBasePlayer* ent, int ID);
	Vector GetHitboxPos(IBasePlayer* ent,int ID);
	void Run(CUserCmd* cmd);
private:
};