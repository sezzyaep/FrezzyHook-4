#pragma once
#include "DLL_MAIN.h"
#include "json/json.h"
#include "steam_sdk/isteamutils.h"
#include "steam_sdk/isteamfriends.h"

using namespace std;

enum removals
{
	vis_recoil = 0,
	vis_smoke,
	flash,
	scope,
	zoom,
	post_processing,
	fog,
	shadow,
};

enum visual_indicators
{
	fake = 0, // 1
	lag_comp, // 2
	fake_duck, // 4
	override_dmg,// 8
	force_baim, // 16
	double_tap, // 32
	manual_antiaim // 64
};

enum chams_type
{
	arms,
	weapon,
	//sleeve,
	history,
	desync
};

enum weap_type {
	def,
	scar,
	scout,
	_awp,
	rifles,
	pistols,
	heavy_pistols
};
enum weap_category_legit : int {
	Def = 0,
	Pistol,
	Rifle,
	HeavyPistols,
	Sniper,
	TAWP,
	SHOTGUNS,
	total
};

struct CLegitWeaponConfig {
	bool enable;
	int reactiontime;
	int mindamagetype;
	int mindamage;
	bool visibleonly;
	float smoothing;
	int fov;
	unsigned int Hitscan;

	int hitchancetype;
	int hitchance;

	int silentreactiontime;
	bool silentenable;
	int silentfov;
	unsigned int silentHitscan;

	float RCS;

	bool autostop;
	int autostopreaction;


};

struct CWeaponConfig {
	bool enable;
	int mindamage;
	int mindamage_override;
	int hitchance;

	int hitchancetype;
	bool multipoint;
	bool quickstop;
	int quickstop_options;
	int pointscale_head;
	int pointscale_body;
	int pointscale_limbs;

	unsigned int hitscan;
	bool ignore_limbs;
	bool adaptive_baim;
	unsigned int hitscan_baim;
	unsigned int safepoint;
	bool dynamicdamage;
	unsigned int baim;
	int baim_under_hp;

	int max_misses;
};

struct c_bind;
struct CGlobalVariables
{
	struct {
		bool enable;
		bool resolver;
		bool backtrack;
		bool fronttrack;
		CLegitWeaponConfig weapons[weap_category_legit::total];
		int CurrentWeaponConfig;

	}legitbot;
	struct
	{
		bool enable;
		bool autoscope;
		bool disable_dt_delay;
		bool autoshoot;
		bool delayshot;
		bool hitchance_consider_hitbox;
		bool posadj;
		bool fronttrack;
		bool backshoot_bt;
		int resolver;
		bool random_resolver;
		bool onshot_invert;
		bool onshot_resolver;
		bool autoinvert;
		int invertspeed;
		bool autospeed;
		bool customAA;
		int base_angle;
		bool shotrecord;
		int BDT;
		color_t shot_clr = color_t(255, 255, 255, 255);;
		c_bind* override_dmg = new c_bind();
		c_bind* force_body = new c_bind();
		c_bind* double_tap = new c_bind();
		c_bind* hideshots = new c_bind();
		int hideshotmode;
		c_bind* teleport = new c_bind();
		bool double_tap_modes;

		// scar, scout, awp, rifles, pistols, heavy pistols
		CWeaponConfig weapon[7];

		bool extend_backtrack;
		int active_index;
		bool FullTP;
		unsigned int DoubletapFlags;
		int LagExploitS;
		bool NOnsilent;
	} ragebot;
	struct
	{
		c_bind* fakepeek = new c_bind();
		bool infakepeek = false;
		bool fakepeeking = false;
		bool enable;
		c_bind* inverter = new c_bind();

		int fakelag;
		int fakelagfactor;
		int fakelagvariance;
		int fakejitr;
		int min, max;
		int val_a, val_b;

		c_bind* fakeduck = new c_bind();
		c_bind* slowwalk = new c_bind();
		int slowwalk_speed;

		struct
		{
			bool enable;
			c_bind* left = new c_bind();
			c_bind* right = new c_bind();
			c_bind* back = new c_bind();
		} aa_override;
		int desync_type;
		int customaa_desync_type;
		bool LBYFlick;
		int LBYFlick_Intervals;
		int LBYFlick_Intervals_S;
		c_bind* inv_LBYF = new c_bind();
		c_bind* LBYF = new c_bind();
		c_bind* FLOK = new c_bind();
		c_bind* BYEBYE = new c_bind();
		int pitch;
		int DTT;
		int jittervalue;
		int desync_amount;
		bool LBYRecharge;
		int customAAT;
		int LBYT;
		int fakelagonshot;
		int DesyncAMM;
		bool cusjit;
		bool enable_cusjit;
		int cusjitamm;
		bool customRandom;
		int swaymin;
		int swaymax;
		int swayspeed;
		int swayratio;
		int yaw_mod_type;
		int yaw_mod_val;
		bool AI;
		bool SlowDT;
		bool aa_customextend;
		bool jitter_randomizer;
		bool legitaaonuse;
		bool lby_breaker_bool;
		float delta_val;
		bool fakelag_onshot;
		bool preset;
		int sharedfakelag;
		int sharedyaw;
		bool indicatorr;
		bool zi180;
		bool send_packet;
		bool speciallag;
		int FakelagVarr;
		bool NFLDT;
		int FD;
		int FD_Spam;
		bool GG;
		bool onshottt;
		bool antionshotretard;
		int direction;
		color_t manualindicator = color_t(255, 255, 255, 255);
		float slowwalk__speed;
		int baseangle;
		int baseangle_direction;
		int yaw_angle_start;
		int yawmodifier;
		int fakelag_mode;
		int fakelag_min;
		int yawmodifier_value;
		int Legs;
		int fakelagjitter;
		int airl;
		struct {
			c_bind* edgeyaw = new c_bind();
			int direction;
			c_bind* left = new c_bind();

			c_bind* right = new c_bind();
		} manual;
	} antiaim;

	struct
	{
		float animationspeed = 100.f;
		int CurTab;
	} CMenu;

	struct
	{
		bool override_knife;
		int skins_knife_model;
		bool override_gloves;
		int skins_glove_model;
		int skins_glove_skin;
		int currentWeapon;
		struct
		{
			bool weapon_skinchanger;
			bool weapon_startrack;
			float weapon_quality;
			int weapon_seed;
			int weapon_skin;
		} W[550];
	}
	skins;
	struct
	{
		bool enable = false;
		bool teammates;
		bool dormant;
		bool box;
		color_t box_color = color_t(255, 255, 255, 255);
		bool healthbar;
		bool name;
		color_t name_color = color_t(255, 255, 255, 255);
		unsigned int flags;
		color_t flags_color = color_t(255, 255, 255, 255);
		bool weapon;
		color_t weapon_color = color_t(255, 255, 255, 255);

		bool skeleton;
		color_t skeleton_color = color_t(255, 255, 255, 255);

		bool ammo;
		color_t ammo_color = color_t(150, 150, 255, 255);

		int hitmarker;
		color_t hitmarker_color = color_t(255, 255, 255, 255);
		color_t aimbot_color = color_t(142, 138, 255, 255);
		int hitmarker_sound;
		unsigned int FOVArrowsFlags;
		unsigned int remove;
		int scope_thickness;

		bool glow;
		color_t glow_color = color_t(170, 170, 255, 255);
		int glowtype;

		bool DURKA;

		bool local_glow;
		color_t local_glow_clr;


		unsigned int indicators;

		bool innacuracyoverlay;
		bool PreserveKills;
		int innacuracyoverlaytype;
		color_t innacuracyoverlay_color = color_t(0, 0, 0, 40);
		int innacuracyoverlay_rotatingspeed;
		int innacuracyoverlay_alpha;
		bool bullet_tracer;
		color_t bullet_tracer_color = color_t(92, 133, 255, 255);
		bool bullet_impact;
		bool bullet_tracer_local;
		color_t bullet_tracer_local_color = color_t(176, 92, 255, 255);
		color_t bullet_impact_color = color_t(255, 255, 255, 0);
		color_t client_impact_color = color_t(255, 255, 255, 0);
		float impacts_size = 2.f;
		int bullet_tracer_type = 1;
		int aspect_ratio;
		bool shot_multipoint;
		//bool removesmoke, removeflash, removescope, removezoom;
		bool snipercrosshair;
		bool clantagspammer;
		int transparentprops;
		//bool removevisualrecoil;
		bool chams;
		bool chamsteam;
		bool chamsxqz;
		int overlay = 2;
		bool watermark = false;
		int watermark_type;
		int overlay_xqz = 2;
		color_t chamscolor = color_t(166, 255, 0, 255);
		color_t chamscolor_xqz = color_t(153, 148, 255, 255);
		color_t chamscolor_glow = color_t(166, 255, 0, 255);;
		color_t chamscolor_glow_xqz = color_t(153, 148, 255, 255);;
		//bool backtrackchams;
		int chamstype;
		bool aaindicator, aaindicator2;
		bool autowallcrosshair;
		bool lbyindicator;
		bool showgrenades;
		bool showdroppedweapons;
		bool showbomb;

		bool out_of_fov;
		int out_of_fov_distance = 30;
		int out_of_fov_size = 30;
		color_t out_of_fov_color = color_t(115, 115, 255, 255);

		color_t arrow22_clr;
		int chams_brightness;
		int local_chams_brightness;
		int trnsparency;
		bool interpolated_model;
		bool interpolated_dsy;
		bool interpolated_bt;
		struct MiscChams_t
		{
			bool enable;
			int material;
			color_t clr = color_t(255, 255, 255, 255);;
			color_t glow_clr = color_t(255, 255, 255, 255);;

			int chams_brightness;

			// metallic
			int phong_exponent;
			int phong_boost;
			int rim;
			color_t metallic_clr;
			color_t metallic_clr2;

			// meme
			int pearlescent;
			int overlay;
		};
		int active_chams_index;
		MiscChams_t misc_chams[4];

		// local player esp
		bool localchams;
		bool localpulsatingeffect;
		int localchamstype;
		color_t local_glow_color = color_t(255, 255, 255, 255);
		color_t localchams_color = color_t(255, 255, 255, 255);

		bool nightmode;
		int ambientlight;
		int modellight;
		color_t ambient_color = color_t(255, 255, 255, 255);
		//bool removepostprocessing;
		int nightmode_val = 100;
		color_t nightmode_color = color_t(170, 170, 170, 255);
		color_t nightmode_prop_color = color_t(170, 170, 170, 255);
		color_t nightmode_skybox_color = color_t(255, 255, 255, 255);
		int thirdperson_dist = 120;
		bool chamsragdoll;
		int phong_exponent;
		int phong_boost;
		int rim;
		int pearlescent;
		color_t metallic_clr;
		color_t metallic_clr2;
		color_t glow_col = color_t(110, 168, 0, 255);
		color_t glow_col_xqz = color_t(72, 70, 122, 255);

		int points_size;

		struct
		{
			int phong_exponent;
			int phong_boost;
			int rim;
			color_t metallic_clr;
			color_t metallic_clr2;
			int pearlescent;
			int overlay;
		}local_chams;

		bool override_hp;
		color_t hp_color = color_t(115, 115, 255, 255);

		int prop, wall;

		bool show_points;
		color_t points_color = color_t(255, 255, 255, 255);
		bool nadepred;
		color_t nadepred_color = color_t(115, 115, 255, 255);
		bool eventlog = true;
		color_t eventlog_color = color_t(255, 255, 255, 255);
		bool speclist;
		bool monitor;
		bool force_crosshair;

		bool kill_effect;

		int anim_id;
		bool weapon_icon;
		bool blend_on_scope;
		int blend_value;
		bool DTindi;
		int disclay = 0;
		int indicatoroffset = 300;
		bool galaxy;
		bool optimize;
		int op_strength;
		unsigned int To_Op;
		int ScopeTHing;
		color_t ScopeColor = color_t(255, 255, 255, 255);
		struct {
			int X = 300;
			int Y = 600;
		}doubletapindicator;
		color_t MollyColor = color_t(120, 136, 255, 255);
		color_t BombColor = color_t(120, 136, 255, 255);
		color_t GrenadeColor = color_t(170,170,255,255);
		color_t localmemecolor = color_t(115, 115, 255, 255);
		unsigned int MollyWarning;
		unsigned int GrenadeProximityWarning;
		unsigned int BombT;
		struct {
			int X = 650;
			int Y = 650;
		} BombTimerloc;
		unsigned int localmemes;
		int WallbangIndi;
	} visuals;

	struct
	{
		int skyboxx = 3;
		bool bunnyhop;
		bool thirdperson;
		c_bind* thirdperson_bind = new c_bind();
		bool knifebot;
		bool antiuntrusted = true;
		bool hold_firinganims;
		bool removelocalplayermomentum = false;
		int viewmodelfov = 68;
		int viewmodel_x, viewmodel_y, viewmodel_z;
		int worldfov = 90;
		c_bind* AutoPeek = new c_bind();
		color_t AutoPeekCol = color_t(170, 170, 255, 255);
		int chatspam;
		struct {
			bool enable;
			int main;
			int pistol;
			unsigned int misc;
		}autobuy;
		struct {
			bool enablekillsay;
			bool enabledeathsay;
			std::string killsay;
			std::string deathsay;
			unsigned int killsayflags;
		}killsaysandshit;
		float fakeping;
	} misc;

	struct
	{
		int shareddesync;
		int keybindx = 400;
		int keybindy = 400;
		int antiaimindicatorx;
		int antiaimindicatory;
		bool leagcy = true;
		int freestand;
		int freestand_side;
		bool overidefreestand;
		bool autodir;
		bool OnshotAA;
		c_bind* OnshotAAA = new c_bind();
		c_bind* OnPeekAA = new c_bind();
		bool OnPeekAAM;
		int sharedmindmg;
		bool peeking;
		int colormode;
		color_t PCol = color_t(143, 150, 255, 255);
		bool HideAAM;
		c_bind* HideAA = new c_bind();
	} movable;

	struct
	{
		bool asf;
		c_bind* onshot = new c_bind();
		c_bind* autodir = new c_bind();
		c_bind* yawbase = new c_bind();
		c_bind* yawmodifier = new c_bind();
		c_bind* freestand = new c_bind();
		c_bind* pitch = new c_bind();
		c_bind* fakelag = new c_bind();
		c_bind* safepoint = new c_bind();
	} keybind;

	struct
	{
		bool yawbase;
		bool yawmodifier;
		bool fakelag;
		unsigned int fakelagflags;
		int fakelagaa;
		bool freestand;
		bool pitch;
	} checkbox;

	struct
	{
		std::string active_config_name;
		std::string serverIP;
		bool BYEEEE;
		std::string fakevote;
		int active_config_index;
		int active_scripts_index;
		std::string LoadedConfig = "No Config Loaded";
		color_t color;
		bool open;
		int y;
		bool guiinited;
		int current_tab;
		bool outline = true;
		bool keybinds = false;
		int keybindX = 150;
		int keybindY = 150;
		bool remove = true;
		int p;
	} menu;


};

extern CGlobalVariables vars;

typedef Json::Value json_t;

class CConfig
{
private:

	string GetModuleFilePath(HMODULE hModule);
	string GetModuleBaseDir(HMODULE hModule);

public:
	vector<std::string> ConfigList;
	void ReadConfigs(LPCTSTR lpszFileName);
	void ResetToDefault(bool notify);
	bool init = false;
	CConfig()
	{
		Setup();
	}
	void SaveColor(color_t color, const char* name, json_t* json) {
		auto& j = *json;
		j[name]["red"] = color.get_red();
		j[name]["green"] = color.get_green();
		j[name]["blue"] = color.get_blue();
		j[name]["alpha"] = color.get_alpha();
	}
	void SaveBind(c_bind* bind, const char* name, json_t* json) {
		auto& j = *json;
		j[name]["key"] = bind->key;
		j[name]["type"] = bind->type;
		j[name]["active"] = bind->active;
	}
	void LoadBool(bool* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asBool();
	}
	void LoadInt(int* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asInt();
	}
	void LoadUInt(unsigned int* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asUInt();
	}
	void LoadFloat(float* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asFloat();
	}
	void ALoadString(string* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asString();
	}
	void LoadColor(color_t* pointer, const char* name, json_t json) {
		if (!json.isMember(name))
			return;
		auto location = json[name];
		if (location.isMember("red") && location.isMember("green")
			&& location.isMember("blue") && location.isMember("alpha")) {
			pointer->set_red(location["red"].asInt());
			pointer->set_green(location["green"].asInt());
			pointer->set_blue(location["blue"].asInt());
			pointer->set_alpha(location["alpha"].asInt());
		}
	}
	void LoadBind(c_bind* pointer, const char* name, json_t json) {
		if (!json.isMember(name))
			return;
		auto location = json[name];
		if (location.isMember("key") && location.isMember("type")) {
			pointer->key = location["key"].asUInt();
			pointer->type = location["type"].asUInt();
			pointer->active = location["active"].asBool();
		}
	}
	void Setup();

	void Save(string cfg_name);
	void Load(string cfg_name);
	void Export(string name);
};

extern CConfig Config;

extern LPDIRECT3DTEXTURE9 GetImage(CSteamID SteamId);