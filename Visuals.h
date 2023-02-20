#pragma once
#include "Hooks.h"
#include "RageBackTracking.h"
#include "Hitmarker.h"
#include "GrenadeWarning.h"
struct BOX
{
	int h, w, x, y;
};

struct EventLogMessage
{
	std::string Message;
	int TimeToEnd;
	float Alpha;
	color_t color;
	float Xmodf;
	float YModF;
};

struct NotifyMessage
{
	std::string Owner;
	std::string Text;
	float TimeToEnd;
	float XmodF;
	float alpha;
	float Width;
	bool useicon;
	int icon;
	float YModF;
};

#define AbstractClass class







//Single Visual [Animated ESP]

AbstractClass CSingleVisualsBase {
public:
	virtual void Render(float BaseAnimationSpeed) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(float BaseAnimationSpeed) = 0;
	virtual bool CanErase() = 0;
};



class CSingleMolotovProximityWarning {
private:
	IBasePlayer* Entity;
	Vector Origin;
	std::vector<Vector> VirtualPoints;
	std::vector<vertex> RenderablePoints;
	float TimeLeft;
	float Range;
	void Update();
public:
	CSingleMolotovProximityWarning(IBasePlayer* BindEntity);
	~CSingleMolotovProximityWarning();
	void Render(bool Timer);
	void Shutdown();
};

enum EGrenadeWarningType {
	GW_HE_GRENADE = 0,
	GW_MOLOTOV = 1,
	GW_SMOKE = 2,
	GW_DECOY = 3,
	GW_FLASHBANG = 4
};


class CSingleGrenadeProximityWarning {
public:
	CSingleGrenadeProximityWarning(IBasePlayer* BindEntity, bool Trail, EGrenadeWarningType Type);
	~CSingleGrenadeProximityWarning();
	void Render(float m_flAnimationSpeed, bool m_bValid);
	EGrenadeWarningType m_eType;
	float m_flAlpha;
	float m_flBlend;
	float m_flMolotovBlend;
private:
	void RenderHeGrenade();
	void RenderSmokeGrenade();
	void RenderFlashBang();
	void RenderDecoy();
	void RenderMolotov();
	void PredictGrenade(IBasePlayer* Entity, bool Trail);
	bool CacheModel(const char* szModelName)
	{
		auto m_pModelPrecacheTable = interfaces.net_string_container->findTable("modelprecache");

		if (m_pModelPrecacheTable)
		{
			m_pModelPrecacheTable->addString(false, szModelName);
		}
		return true;
	}
	Vector m_vEndPosition;
	bool m_bSafe;
	bool m_bVisible;
	float m_flDistance;
	bool m_bWillLand;
	int m_iLastTick;
};















struct dlight_p {
	dlight_t* light;
};

class CVisuals
{
public:
	std::map<CBaseHandle, CSingleGrenadeProximityWarning> GrenadeProximityWarnings;

	std::vector<NotifyMessage> cnotify;
	std::vector<EventLogMessage> cmessages;
	std::vector<IBasePlayer*> players;
	void DrawLocalShit(IDirect3DDevice9* pDevice);
	void Draw();
	void RecordInfo();
	void NightMode();
	void DrawAngleLine(Vector origin, float angle, color_t color);
	void OnDormant();
	void AdjustDormant(IBasePlayer* player, SndInfo_t& sound);
	bool IsValidSound(SndInfo_t& sound);
	Vector strored_origin[64];

	struct SoundPlayer
	{
		void reset()
		{
			m_iIndex = 0;
			m_iReceiveTime = 0;
			m_vecOrigin = Vector(0, 0, 0);
			/* Restore data */
			m_nFlags = 0;
			player = nullptr;
			m_vecAbsOrigin = Vector(0, 0, 0);
			m_bDormant = false;
		}

		void Override(SndInfo_t& sound) {
			m_iIndex = sound.m_nSoundSource;
			m_vecOrigin = *sound.m_pOrigin;
			m_iReceiveTime = interfaces.global_vars->realtime;
		}

		int m_iIndex = 0;
		float m_iReceiveTime = 0;
		Vector m_vecOrigin = Vector(0, 0, 0);
		/* Restore data */
		int m_nFlags = 0;
		IBasePlayer* player = nullptr;
		Vector m_vecAbsOrigin = Vector(0, 0, 0);
		bool m_bDormant = false;
		int KeybindAlpha[11];
	} m_cSoundPlayers[64];
	CUtlVector<SndInfo_t> m_utlvecSoundBuffer;
	CUtlVector<SndInfo_t> m_utlCurSoundList;
	std::vector<SoundPlayer> m_arRestorePlayers;
};
enum ACT
{
	ACT_NONE,
	ACT_THROW,
	ACT_LOB,
	ACT_DROP,
};


class CEnemyGrenadePrediction : public Singleton < CEnemyGrenadePrediction >
{
public:
	Vector Predict(Vector EyePos, Vector EyeAngle,Vector velocity, float throwstrength, IBaseCombatWeapon* weap);
};

class CGrenadePrediction : public Singleton<CGrenadePrediction>
{
private:
	std::vector<Vector> path;

	int type = 0;
	int act = 0;

	void Setup(Vector& vecSrc, Vector& vecThrow, Vector viewangles);
	void Simulate(CViewSetup* setup);

	int Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval);
	bool CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval);

	void TraceHull(Vector& src, Vector& end, trace_t& tr);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void PushEntity(Vector& src, const Vector& move, trace_t& tr);
	void ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval);
	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
public:
	void Tick(int buttons);
	void View(CViewSetup* setup);
	void Paint(IDirect3DDevice9* pDevice);
};
