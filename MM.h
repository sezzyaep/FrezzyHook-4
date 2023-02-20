#pragma once
#include "Autowall.h"
#include "AnimationFix.h"

struct LegitbotAnimationCache
{
	LegitbotAnimationCache() {};
	void Clear() { this->Stored = false; }
	IBasePlayer* Entity;
	Vector Origin;
	Vector AbsOrigin;
	matrix* Bones;
	Vector Mins;
	Vector Maxs;
	bool Stored;
};

struct CLegitbotCache {
	void Clear() { this->BestDistance = INT_MAX; TargettedHitboxes.clear(); }
	float BestDistance;
	Vector BestPoint;
	std::vector<int> TargettedHitboxes;
	matrix TargettedMatrix[128];
	Vector ViewAngles;
	float TargettedDamage;

};

struct CLegitScanCache {
	void Clear() { this->BestDistance = INT_MAX; Point.Zero(); }
	float BestDistance;
	Vector Point;
};

class Legitbot : public Singleton<Legitbot>
{
public:
	void Run(CUserCmd*);
	void Reset();
	float Scan(IBasePlayer*);
	std::vector<Vector> GetPoints(IBasePlayer* pBaseEntity, int iHitbox, matrix BoneMatrix[128]);
	std::vector<int> GetTargettedHitboxes();
	bool PointVisible(Vector& point);
private:
	void SetAnimations(animation*);
	void RestoreAnimations();
	LegitbotAnimationCache AnimationCache;
	CLegitbotCache Cache;
	CLegitWeaponConfig GetConfig();
	CLegitWeaponConfig Config;
	CLegitScanCache ScanCache;
};