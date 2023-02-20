#pragma once
#include "Utils.h"
#include "DLL_MAIN.h"
#include "interfaces.h"
#include "i_base_player.h"
#include "c_usercmd.h"
#include "render.h"
#define TICK_INTERVAL            ( interfaces.global_vars->interval_per_tick )
#define TIME_TO_TICKS( dt )        ( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )        ( TICK_INTERVAL *( t ) )

#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD        1
#define HITGROUP_CHEST        2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10

class INetChannel;
class CInput;
class ConVar;
class VMTHook;
class CClientState;

using namespace std;
////////////////////////////////////////////////////////////////////////////////////////////////////

struct cINIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
};
struct command
{
	int previous_command_number = 0;
	bool is_used = false;
	int cmd_number = 0;
	bool is_outgoing = false;
};
struct correction_data
{
	int tickcount = 0;
	int choked_commands = 0;
	int command_number = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

struct MatrixRecorded;
extern MatrixRecorded g_Records[64];
struct animation;

class cStructManager
{
public:
	bool ConnectedToInternet = false;
	bool NoUser = false;
	cINIT Init; 
	bool DoUnload;
	long long expire_date;
	string username;
	IDirect3DDevice9* render_device;
	std::string PUsername;
	std::string FullUsername;
	int FullUsernameLength;
	int PDaysLeft;
	bool ShowConfig;
	int CLMOVEShift;
	struct {

		Vector Position;
		bool ShouldGoBack;
	}AutoPeek;
	cUtils Utils;
	int unpdred_tick;
	Vector unpred_eyepos;
	IBasePlayer* local;
	int NoTeleportRecharge;
	LPDIRECT3DTEXTURE9 Logo;
	bool LagExploit;
	float impact_time;
	Vector impact_origin;
	Vector origin;
	bool mapChanged;
	bool InLbyUpdate;
	struct data_t {
		float sim_time;
		Vector origin;
		float interp_time;
	}all_data[2];
	struct {
		float TimeLeft;
		float DefuseTimeLeft;
		bool IsDefusing;
		float MaxTime;
		bool Planted;
	} PDBomb;
	struct {
		bool work;
		bool invalid_cycle;
		float next_attack;
	}weapon_struct;

	// meme
	Vector last_shoot_pos;
	int m_nTickbaseShift;
	bool need_recharge_rn;
	int w;
	int h;
	float tick_rate;
	bool bInSendMove;
	bool charge;
	int32_t nSinceUse;
	bool TickShifted;
	bool bShootedFirstBullet;
	bool bFirstSendMovePack;
	Vector eyepos;
	int last_forced_tickcount;
	matrix fakematrix[128];
	matrix realmatrix[128];
	matrix fakelag_matrix[128];
	bool hitchance;
	Vector last_sended_origin;
	float lc_factor;
	bool round_end;
	bool SwitchAA, EnableSwitchAA;
	int SwitchAA2;
	float next_lby_update;
	void updatelocalplayer();
	bool send_packet;
	bool shifting = false;
	bool should_update_animations;
	bool fake_duck;
	bool CSI_FD;
	bool stand;
	int scroll_amount;
	string subs_end;
	bool didShot[65];
	float delta;
	CCSGOPlayerAnimState* animstate;
	bool canBreakLC, canDrawLC;
	bool EnableBones, Rebuild, ShouldUpdate, UpdateMatrix;
	bool should_sidemove;
	CUserCmd* cmd;
	bool choke_meme;
	alignas(16) matrix BoneMatrix[128];
	float old_simtime[64];
	bool should_stop;
	bool should_stop_slide;
	bool should_stop_fast;
	int PDefensiveTicks;

	bool SkyCheckk;

	float stop_speed;
	bool EnemyVisible;
	bool visiblee;
	IBaseCombatWeapon* weapon;
	CCSWeaponInfo* weapon_data;
	Vector CameraPos;
	int weapon_id;
	Vector viewangles;
	std::vector<int> choked_number;
	std::unique_ptr< VMTHook > g_pNetChannelHook;
	std::unique_ptr< VMTHook > g_pPlayerHook;
	vector<int> choked_history;
	int eventtimer;
	float zero_pitch;
	float last_clamped_yaw;
	Vector VisualAngle;
	Vector UnChokedAng;
	int last_choked;
	float NextPredictedLBYUpdate;
	Vector vecUnpredictedVel;
	Vector original;
	Vector FakeAngle;
	Vector last_shot_angle;
	float feetYaw;
	float g_flVelMod;
	bool g_bOverrideVelMod;
	CClientState* client_state;
	float velocity_modifier;
	CCSGameRules* game_rules;
	INetChannel* pNetChannel;
	int max_fakelag_choke;
	int skip_ticks;
	bool can_charge_on_shot;
	bool dt_charged;
	bool disable_dt;
	float ping;
	float desync_angle;
	int damagedshots;
	int not_damagedshots;
	int chockedticks, timer;
	float viewMatrix[4][4] = { 0 };
	bool ForceOffAA;
	bool enableclantag;
	bool ResetKillfeed = false;
	//bool legitaaon = false;
	float EntityAlpha[65];
	float LastSeenTime[65];
	bool StrafeModifiedAngles;
	Vector StrafeAngles;
	float resolved_yaw[65];
	int missedshots[65];
	int firedshots[65];
	int hitshots[65];
	int dont_miss_feet_nigger[65];
	float weaponspread;
	std::deque<correction_data> c_data;
	std::deque<command> packets;
	float curtime;
	int tickcount;
	float spread, innacuracy;
	uint32_t shot_cmd{};
	uintptr_t uRandomSeed;
	bool ispenetrable;
	bool key_pressed[256];
	int key_pressedticks[256];
	bool key_down[256];
	bool InThirdperson;
	bool InAutowall;
	bool canR8shoot;
	string build_date = __DATE__;
	float get_absolute_time() {
		return (float)(clock() / (float)1000.f);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////