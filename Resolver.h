#pragma once
struct animation;
class CResolver
{
private:
	float GetLeftYaw(IBasePlayer*);
	float GetRightYaw(IBasePlayer*);
	bool TargetSide(IBasePlayer*);
	void DetectSide(IBasePlayer* player);
	bool TargetJitter(IBasePlayer*, bool);
	float max_desync_delta(IBasePlayer* player);
	bool DoesHaveJitter(IBasePlayer* player);
	float resolved_angle[65];
	float lastbruteforce[65];
	int side[65];
	int jitterside[65];
	bool UseFreestand[65];
	int FreestandSide[65];
public:
	float GetAngle(IBasePlayer*);
	float GetForwardYaw(IBasePlayer*);
	float GetBackwardYaw(IBasePlayer*);
	void Do(IBasePlayer*);
	void StoreAntifreestand();
};
extern CResolver* resolver;
extern std::string ResolverMode[65];