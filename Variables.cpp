#include "Variables.h"
#include "Hooks.h"
#include "aes256/aes256.hpp"
#include <sstream>


char enc_key[32] = { 0x1f, 0x01, 0x02, 0x03, 0x04, 0x13, 0x05, 0x06, 0x07,0x17, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
									 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x18, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x19, 0x00 };


ByteArray encryption_key;

void CConfig::ReadConfigs(LPCTSTR lpszFileName)
{
	ConfigList.push_back(lpszFileName);
}
void ResetBind(c_bind* bind) {
	bind->active = false;
	bind->key = 0;
	bind->type = 0;
}
void PConvert(const uint8_t* rgba, uint8_t* out, const size_t size)
{
	auto in = reinterpret_cast<const uint32_t*>(rgba);
	auto buf = reinterpret_cast<uint32_t*>(out);
	for (auto i = 0u; i < (size / 4); ++i)
	{
		const auto pixel = *in++;
		*buf++ = (pixel & 0xFF00FF00) | ((pixel & 0xFF0000) >> 16) | ((pixel & 0xFF) << 16);
	}
}

LPDIRECT3DTEXTURE9 GetImage(CSteamID SteamId)
{
	LPDIRECT3DTEXTURE9 asdgsdgadsg;



	int iImage = SteamFriends->GetLargeFriendAvatar(SteamId);
	if (iImage == -1)
		return nullptr;
	uint32 uAvatarWidth, uAvatarHeight;
	if (!SteamUtils->GetImageSize(iImage, &uAvatarWidth, &uAvatarHeight))
		return nullptr;
	const int uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	uint8* pAvatarRGBA = new uint8[uImageSizeInBytes];
	if (!SteamUtils->GetImageRGBA(iImage, pAvatarRGBA, uImageSizeInBytes))
	{
		delete[] pAvatarRGBA;
		return nullptr;
	}
	auto res = g_Render->GetDevice()->CreateTexture(uAvatarWidth,
		uAvatarHeight,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&asdgsdgadsg,
		nullptr);
	std::vector<uint8_t> texData;
	texData.resize(uAvatarWidth * uAvatarHeight * 4u);
	PConvert(pAvatarRGBA,
		texData.data(),
		uAvatarWidth * uAvatarHeight * 4u);
	D3DLOCKED_RECT rect;
	res = asdgsdgadsg->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);
	auto src = texData.data();
	auto dst = reinterpret_cast<uint8_t*>(rect.pBits);
	for (auto y = 0u; y < uAvatarHeight; ++y)
	{
		std::copy(src, src + (uAvatarWidth * 4), dst);

		src += uAvatarWidth * 4;
		dst += rect.Pitch;
	}
	res = asdgsdgadsg->UnlockRect(0);
	delete[] pAvatarRGBA;
	return asdgsdgadsg;
}

void CConfig::ResetToDefault(bool notify)
{
	vars.movable.colormode = 2;
	vars.ragebot.enable = false;
	vars.ragebot.autoscope = false;
	vars.ragebot.disable_dt_delay = false;
	vars.ragebot.autoshoot = false;
	vars.ragebot.delayshot = false;
	vars.ragebot.hitchance_consider_hitbox = false;
	vars.ragebot.posadj = false;
	vars.ragebot.backshoot_bt = false;
	vars.ragebot.resolver = 0;
	vars.ragebot.random_resolver = false;
	vars.ragebot.onshot_invert = false;
	vars.ragebot.onshot_resolver = false;
	vars.ragebot.autoinvert = false;
	vars.ragebot.invertspeed = 0;
	vars.ragebot.autospeed = false;
	vars.ragebot.customAA = false;
	vars.ragebot.base_angle = 0;
	vars.ragebot.shotrecord = false;
	vars.ragebot.BDT = 0;
	vars.ragebot.shot_clr = color_t(255, 255, 255, 255);
	ResetBind(vars.ragebot.override_dmg);
	ResetBind(vars.ragebot.force_body);
	ResetBind(vars.ragebot.double_tap);
	ResetBind(vars.ragebot.teleport);
	vars.ragebot.double_tap_modes = 0;




	for (int i = 0; i < 6; i++) {
		vars.ragebot.weapon[i].adaptive_baim = false;
		vars.ragebot.weapon[i].baim_under_hp = 0;
		vars.ragebot.weapon[i].enable = false;
		vars.ragebot.weapon[i].hitchance = 0;
		vars.ragebot.weapon[i].hitchancetype = 0;
		vars.ragebot.weapon[i].hitscan = 0;
		vars.ragebot.weapon[i].hitscan_baim = 0;
		vars.ragebot.weapon[i].ignore_limbs = false;
		vars.ragebot.weapon[i].max_misses = 0;
		vars.ragebot.weapon[i].mindamage = 0;
		vars.ragebot.weapon[i].mindamage_override = 0;
		vars.ragebot.weapon[i].multipoint = false;
		vars.ragebot.weapon[i].pointscale_body = 0;
		vars.ragebot.weapon[i].pointscale_head = 0;
		vars.ragebot.weapon[i].pointscale_limbs = 0;
		vars.ragebot.weapon[i].quickstop = false;
		vars.ragebot.weapon[i].quickstop_options = 0;
	}

	vars.ragebot.extend_backtrack = false;

	vars.ragebot.FullTP = false;
	vars.ragebot.DoubletapFlags = 0;
	vars.ragebot.LagExploitS = 0;



	ResetBind(vars.antiaim.fakepeek);
	vars.antiaim.infakepeek = false;
	vars.antiaim.fakepeeking = false;
	vars.antiaim.enable = false;
	ResetBind(vars.antiaim.inverter);

	vars.antiaim.fakelag = 0;
	vars.antiaim.fakelagfactor = 0;
	vars.antiaim.fakelagvariance = 0;
	vars.antiaim.fakejitr = 0;
	vars.antiaim.min = 0;
	vars.antiaim.max = 0;
	vars.antiaim.val_a = 0;
	vars.antiaim.val_b = 0;

	ResetBind(vars.antiaim.fakeduck);
	ResetBind(vars.antiaim.slowwalk);
	vars.antiaim.slowwalk_speed = 0;


	vars.antiaim.enable = false;


	vars.antiaim.desync_type = 0;
	vars.antiaim.customaa_desync_type = 0;
	vars.antiaim.LBYFlick = false;
	vars.antiaim.LBYFlick_Intervals = 0;
	vars.antiaim.LBYFlick_Intervals_S = 0;
	ResetBind(vars.antiaim.inv_LBYF);
	ResetBind(vars.antiaim.LBYF);
	ResetBind(vars.antiaim.FLOK);
	ResetBind(vars.antiaim.BYEBYE);
	vars.antiaim.pitch = 0;
	vars.antiaim.DTT = 0;
	vars.antiaim.jittervalue = 0;
	vars.antiaim.desync_amount = 0;
	vars.antiaim.LBYRecharge = false;
	vars.antiaim.customAAT = 0;
	vars.antiaim.fakelagonshot = 0;
	vars.antiaim.DesyncAMM = 0;
	vars.antiaim.cusjit = false;
	vars.antiaim.enable_cusjit = false;
	vars.antiaim.cusjitamm = 0;
	vars.antiaim.customRandom = false;
	vars.antiaim.swaymin = 0;
	vars.antiaim.swaymax = 0;
	vars.antiaim.swayspeed = 0;
	vars.antiaim.swayratio = 0;
	vars.antiaim.yaw_mod_type = 0;
	vars.antiaim.yaw_mod_val = 0;
	vars.antiaim.AI = false;
	vars.antiaim.SlowDT = false;
	vars.antiaim.aa_customextend = false;
	vars.antiaim.jitter_randomizer = false;
	vars.antiaim.legitaaonuse = false;
	vars.antiaim.lby_breaker_bool = false;
	vars.antiaim.delta_val = 0;
	vars.antiaim.fakelag_onshot = false;
	vars.antiaim.preset = false;
	vars.antiaim.sharedfakelag = 0;
	vars.antiaim.sharedyaw = 0;
	vars.antiaim.indicatorr = false;
	vars.antiaim.zi180 = false;
	vars.antiaim.send_packet = false;
	vars.antiaim.speciallag = false;
	vars.antiaim.FakelagVarr = 0;
	vars.antiaim.NFLDT = false;
	vars.antiaim.FD = 0;
	vars.antiaim.FD_Spam = 0;
	vars.antiaim.GG = false;
	vars.antiaim.onshottt = false;
	vars.antiaim.antionshotretard = false;
	vars.antiaim.direction = 0;
	vars.antiaim.manualindicator = color_t(255, 255, 255, 255);
	vars.antiaim.slowwalk__speed = 0;
	vars.antiaim.baseangle = 0;
	vars.antiaim.baseangle_direction = 0;
	vars.antiaim.yaw_angle_start = 0;
	vars.antiaim.yawmodifier = 0;
	vars.antiaim.fakelag_mode = 0;
	vars.antiaim.fakelag_min = 0;
	vars.antiaim.yawmodifier_value = 0;
	vars.antiaim.Legs = 0;
	vars.antiaim.fakelagjitter = 0;
	vars.antiaim.airl = 0;

	ResetBind(vars.antiaim.manual.edgeyaw);
	vars.antiaim.direction = 0;
	ResetBind(vars.antiaim.manual.left);

	ResetBind(vars.antiaim.manual.right);







	vars.visuals.enable = false;
	vars.visuals.teammates = false;
	vars.visuals.dormant = false;
	vars.visuals.box = false;
	vars.visuals.box_color = color_t(255, 255, 255, 255);
	vars.visuals.healthbar = false;
	vars.visuals.name = false;
	vars.visuals.name_color = color_t(255, 255, 255, 255);
	vars.visuals.flags = 0;
	vars.visuals.flags_color = color_t(255, 255, 255, 255);
	vars.visuals.weapon = false;
	vars.visuals.weapon_color = color_t(255, 255, 255, 255);

	vars.visuals.skeleton = false;
	vars.visuals.skeleton_color = color_t(255, 255, 255, 255);

	vars.visuals.ammo = false;
	vars.visuals.ammo_color = color_t(150, 150, 255, 255);

	vars.visuals.hitmarker = false;
	vars.visuals.hitmarker_color = color_t(255, 255, 255, 255);
	vars.visuals.hitmarker_sound = 0;
	vars.visuals.FOVArrowsFlags = 0;
	vars.visuals.remove = 0;
	vars.visuals.scope_thickness = 0;

	vars.visuals.glow = false;
	vars.visuals.glow_color = color_t(170, 170, 255, 255);
	vars.visuals.glowtype = 0;

	vars.visuals.DURKA = false;

	vars.visuals.local_glow = false;



	vars.visuals.indicators = 0;

	vars.visuals.innacuracyoverlay = false;
	vars.visuals.PreserveKills = false;
	vars.visuals.innacuracyoverlaytype = 0;
	vars.visuals.innacuracyoverlay_color = color_t(0, 0, 0, 40);

	vars.visuals.bullet_tracer = false;
	vars.visuals.bullet_tracer_color = color_t(92, 133, 255, 255);
	vars.visuals.bullet_impact = false;
	vars.visuals.bullet_tracer_local = false;
	vars.visuals.bullet_tracer_local_color = color_t(176, 92, 255, 255);
	vars.visuals.bullet_impact_color = color_t(255, 255, 255, 0);
	vars.visuals.client_impact_color = color_t(255, 255, 255, 0);
	vars.visuals.impacts_size = 2.f;
	vars.visuals.bullet_tracer_type = 1;
	vars.visuals.aspect_ratio = 0;
	vars.visuals.shot_multipoint = false;
	// removesmoke, removeflash, removescope, removezoom;
	vars.visuals.snipercrosshair = false;
	vars.visuals.clantagspammer = false;
	vars.visuals.transparentprops = 0;
	// removevisualrecoil;
	vars.visuals.chams = false;
	vars.visuals.chamsteam = false;
	vars.visuals.chamsxqz = false;
	vars.visuals.overlay = 2;
	if(notify) 
		vars.visuals.watermark = true;

	vars.visuals.watermark = false;
	vars.visuals.watermark_type = 0;
	vars.visuals.overlay_xqz = 2;
	vars.visuals.chamscolor = color_t(166, 255, 0, 255);
	vars.visuals.chamscolor_xqz = color_t(153, 148, 255, 255);
	vars.visuals.chamscolor_glow = color_t(166, 255, 0, 255);;
	vars.visuals.chamscolor_glow_xqz = color_t(153, 148, 255, 255);;
	// backtrackchams;
	vars.visuals.chamstype = 0;
	vars.visuals.aaindicator = false;
	vars.visuals.aaindicator2 = false;
	vars.visuals.autowallcrosshair = false;
	vars.visuals.lbyindicator = false;
	vars.visuals.showgrenades = false;
	vars.visuals.showdroppedweapons = false;
	vars.visuals.showbomb = false;

	vars.visuals.out_of_fov = false;
	vars.visuals.out_of_fov_distance = 30;
	vars.visuals.out_of_fov_size = 30;
	vars.visuals.out_of_fov_color = color_t(115, 115, 255, 255);

	vars.visuals.ambientlight = 0;
	vars.visuals.ambient_color = color_t(255, 255, 255, 255);
	vars.visuals.chams_brightness = 0;
	vars.visuals.local_chams_brightness = 0;
	vars.visuals.trnsparency = 0;
	vars.visuals.interpolated_model = false;
	vars.visuals.interpolated_dsy = false;
	vars.visuals.interpolated_bt = false;


	for (int i = 0; i < 3; i++) {
		vars.visuals.misc_chams[i].chams_brightness = 0;
		vars.visuals.misc_chams[i].clr = color_t(255, 255, 255, 255);
		vars.visuals.misc_chams[i].enable = false;
		vars.visuals.misc_chams[i].glow_clr = color_t(255, 255, 255, 255);
		vars.visuals.misc_chams[i].overlay = 0;
	}

	// local player esp
	vars.visuals.localchams = false;
	vars.visuals.localpulsatingeffect = false;
	vars.visuals.localchamstype = 0;
	vars.visuals.local_glow_color = color_t(255, 255, 255, 255);
	vars.visuals.localchams_color = color_t(255, 255, 255, 255);

	vars.visuals.nightmode = false;
	// removepostprocessing;
	vars.visuals.nightmode_val = 100;
	vars.visuals.nightmode_color = color_t(170, 170, 170, 255);
	vars.visuals.nightmode_prop_color = color_t(170, 170, 170, 255);
	vars.visuals.nightmode_skybox_color = color_t(255, 255, 255, 255);
	vars.visuals.thirdperson_dist = 120;
	vars.visuals.chamsragdoll = false;
	vars.visuals.phong_exponent = 0;
	vars.visuals.phong_boost = 0;
	vars.visuals.rim = 0;
	vars.visuals.pearlescent = 0;
	vars.visuals.metallic_clr;
	vars.visuals.metallic_clr2;
	vars.visuals.glow_col = color_t(110, 168, 0, 255);
	vars.visuals.glow_col_xqz = color_t(72, 70, 122, 255);

	vars.visuals.points_size = 0;


	vars.visuals.phong_exponent = 0;
	vars.visuals.phong_boost = 0;
	vars.visuals.rim = 0;

	vars.visuals.pearlescent = 0;
	vars.visuals.overlay = 2;

	vars.visuals.override_hp = false;
	vars.visuals.hp_color = color_t(115, 115, 255, 255);

	vars.visuals.prop = 0;
	vars.visuals.wall = 0;

	vars.visuals.show_points = false;
	vars.visuals.points_color = color_t(255, 255, 255, 255);
	vars.visuals.nadepred = false;
	vars.visuals.nadepred_color = color_t(115, 115, 255, 255);
	vars.visuals.eventlog = true;
	vars.visuals.eventlog_color = color_t(255, 255, 255, 255);

	vars.visuals.monitor = false;
	vars.visuals.force_crosshair = false;

	vars.visuals.kill_effect = false;

	vars.visuals.anim_id = 0;
	vars.visuals.weapon_icon = false;
	vars.visuals.blend_on_scope = false;
	vars.visuals.blend_value = 0;
	vars.visuals.DTindi = false;
	vars.visuals.disclay = 0;
	vars.visuals.indicatoroffset = 300;
	vars.visuals.galaxy = false;
	vars.visuals.optimize = false;
	vars.visuals.op_strength = 0;
	vars.visuals.To_Op = 0;
	vars.visuals.ScopeTHing = 0;
	vars.visuals.ScopeColor = color_t(255, 255, 255, 255);
	vars.visuals.doubletapindicator.X = 300;
	vars.visuals.doubletapindicator.Y = 600;
	vars.visuals.MollyColor = color_t(120, 136, 255, 255);
	vars.visuals.MollyWarning = 0;

	vars.misc.skyboxx = 3;
	vars.misc.bunnyhop = false;
	vars.misc.thirdperson = false;
	ResetBind(vars.misc.thirdperson_bind);
	vars.misc.knifebot = false;
	vars.misc.antiuntrusted = true;
	vars.misc.hold_firinganims = false;
	vars.misc.removelocalplayermomentum = false;
	vars.misc.viewmodelfov = 68;
	vars.misc.viewmodel_x = 0;
	vars.misc.viewmodel_y = 0;
	vars.misc.viewmodel_z = 0;
	vars.misc.worldfov = 90;
	ResetBind(vars.misc.AutoPeek);

	vars.misc.autobuy.enable = false;
	vars.misc.autobuy.main = 0;
	vars.misc.autobuy.pistol = 0;
	vars.misc.autobuy.misc = 0;

	auto& A = vars.movable;
	vars.movable.shareddesync = 0;
	vars.movable.keybindx = 400;
	vars.movable.keybindy = 400;
	vars.movable.antiaimindicatorx = 0;
	vars.movable.antiaimindicatory = 0;
	vars.movable.leagcy = true;
	vars.movable.freestand = 0;
	vars.movable.freestand_side = 0;
	vars.movable.overidefreestand = false;
	vars.movable.autodir = false;
	vars.movable.OnshotAA = false;
	ResetBind(vars.movable.OnshotAAA);
	ResetBind(vars.movable.OnPeekAA);
	vars.movable.OnPeekAAM = false;
	vars.movable.sharedmindmg = 0;
	vars.movable.peeking = false;
	vars.CMenu.animationspeed = 100.f;
	vars.movable.colormode = 0;
	vars.movable.PCol = color_t(143, 150, 255, 255);
	vars.movable.HideAAM = false;
	ResetBind(vars.movable.HideAA);

	vars.keybind.asf = false;
	ResetBind(vars.keybind.onshot);
	ResetBind(vars.keybind.autodir);
	ResetBind(vars.keybind.yawbase);
	ResetBind(vars.keybind.yawmodifier);
	ResetBind(vars.keybind.freestand);
	ResetBind(vars.keybind.pitch);
	ResetBind(vars.keybind.fakelag);
	ResetBind(vars.keybind.safepoint);

	vars.checkbox.yawbase = false;
	vars.checkbox.yawmodifier = false;
	vars.checkbox.fakelag = false;
	vars.checkbox.fakelagflags = 0;
	vars.checkbox.fakelagaa = 0;
	vars.checkbox.freestand = false;
	vars.checkbox.pitch = false;
	vars.visuals.aimbot_color = color_t(142, 138, 255, 255);

	vars.menu.keybinds = false;
	vars.menu.keybindX = 150;
	vars.menu.keybindY = 150;
	vars.misc.killsaysandshit.deathsay = "";
	vars.misc.killsaysandshit.enabledeathsay = false;
	vars.misc.killsaysandshit.killsayflags = 0;
	vars.misc.killsaysandshit.enablekillsay = false;
	vars.misc.killsaysandshit.killsay = "";
	vars.visuals.localmemes = 0;
	vars.visuals.localmemecolor = color_t(115, 115, 255, 255);
	vars.misc.chatspam = 0;
	if(notify)
		PNotify("Config", "Reset to Default", 2);

}

void CConfig::Setup()
{
	for (char i = 0; i < 32; i++)
		encryption_key.push_back(enc_key[i]);
	//ResetToDefault();
};

string CConfig::GetModuleFilePath(HMODULE hModule)
{
	string ModuleName = "";
	char szFileName[MAX_PATH] = { 0 };

	if (GetModuleFileNameA(hModule, szFileName, MAX_PATH))
		ModuleName = szFileName;

	return ModuleName;
}

string CConfig::GetModuleBaseDir(HMODULE hModule)
{
	string ModulePath = GetModuleFilePath(hModule);
	return ModulePath.substr(0, ModulePath.find_last_of("\\/"));
}

void CConfig::Save(string cfg_name)
{
	json_t configuration;

	auto& json = configuration["config"];
	json["name"] = cfg_name;

	auto& ragebot = json["ragebot"]; {
		ragebot["enable"] = vars.ragebot.enable;
		ragebot["auto_shoot"] = vars.ragebot.autoshoot;
		ragebot["auto_scope"] = vars.ragebot.autoscope;
		ragebot["delayshot"] = vars.ragebot.delayshot;
		ragebot["hitchance_consider_hitbox"] = vars.ragebot.hitchance_consider_hitbox;
		SaveBind(vars.keybind.onshot, "onshot", &ragebot);
		ragebot["posadj"] = vars.ragebot.posadj;
		ragebot["ftack"] = vars.ragebot.fronttrack;
		ragebot["backshoot_bt"] = vars.ragebot.backshoot_bt;
		ragebot["resolver"] = vars.ragebot.resolver;
		ragebot["shot_record"] = vars.ragebot.shotrecord;
		ragebot["visualize"] = vars.ragebot.NOnsilent;
		ragebot["DT_Defensive"] = vars.ragebot.BDT;
		ragebot["DT_Offensive"] = vars.antiaim.SlowDT;
		ragebot["Recharge_Mode"] = vars.antiaim.DTT;
		SaveBind(vars.ragebot.override_dmg, "override_dmg", &ragebot);
		SaveBind(vars.ragebot.double_tap, "double_tap", &ragebot);
		//	SaveBind(vars.ragebot.hide_shots, "hide_shots", &ragebot);
		SaveBind(vars.ragebot.force_body, "force_body", &ragebot);

		SaveColor(vars.ragebot.shot_clr, "shot_clr", &ragebot);
		SaveBind(vars.keybind.safepoint, "Safepoint", &ragebot);
		ragebot["DTFLAG"] = vars.ragebot.DoubletapFlags;
		ragebot["DTLAG"] = vars.ragebot.LagExploitS;
		auto& weapon_cfg = ragebot["weapon_cfg"];
		for (int i = 0; i < 7; i++) {
			string category = "weapon_" + std::to_string(i);
			weapon_cfg[category]["enable"] = vars.ragebot.weapon[i].enable;
			weapon_cfg[category]["mindamage"] = vars.ragebot.weapon[i].mindamage;
			weapon_cfg[category]["mindamage_override"] = vars.ragebot.weapon[i].mindamage_override;
			weapon_cfg[category]["hitchance"] = vars.ragebot.weapon[i].hitchance;
			weapon_cfg[category]["hitchance_type"] = vars.ragebot.weapon[i].hitchancetype;
			weapon_cfg[category]["multipoint"] = vars.ragebot.weapon[i].multipoint;
			weapon_cfg[category]["quickstop"] = vars.ragebot.weapon[i].quickstop;
			weapon_cfg[category]["quickstop_options"] = vars.ragebot.weapon[i].quickstop_options;
			weapon_cfg[category]["pointscale_head"] = vars.ragebot.weapon[i].pointscale_head;
			weapon_cfg[category]["pointscale_body"] = vars.ragebot.weapon[i].pointscale_body;
			weapon_cfg[category]["pointscale_limbs"] = vars.ragebot.weapon[i].pointscale_limbs;
			weapon_cfg[category]["hitscan"] = vars.ragebot.weapon[i].hitscan;
			weapon_cfg[category]["hitscan_baim"] = vars.ragebot.weapon[i].hitscan_baim;
			weapon_cfg[category]["baim"] = vars.ragebot.weapon[i].baim;
			weapon_cfg[category]["baim_under_hp"] = vars.ragebot.weapon[i].baim_under_hp;
			weapon_cfg[category]["adaptive_baim"] = vars.ragebot.weapon[i].adaptive_baim;
			weapon_cfg[category]["max_misses"] = vars.ragebot.weapon[i].max_misses;
		}
	}

	auto& antiaim = json["antiaim"]; {
		antiaim["enable"] = vars.antiaim.enable;
		antiaim["desync_type"] = vars.antiaim.desync_type;
		//
		antiaim["Custom_AA"] = vars.ragebot.customAA;
		antiaim["Base_Angle"] = vars.antiaim.baseangle;
		antiaim["Base_Dir"] = vars.antiaim.baseangle_direction;
		antiaim["Yaw"] = vars.antiaim.yaw_angle_start;
		antiaim["Modifier"] = vars.antiaim.yawmodifier;
		antiaim["Modifier_Value"] = vars.antiaim.yawmodifier_value;
		antiaim["Custom_Desync"] = vars.antiaim.customAAT;
		antiaim["Ex_Desync"] = vars.menu.p;
		antiaim["Desync_Value"] = vars.antiaim.DesyncAMM;
		antiaim["freestand"] = vars.movable.freestand;
		antiaim["autodir_"] = vars.movable.autodir;
		SaveBind(vars.keybind.autodir, "autodir", &antiaim);


		antiaim["fs_"] = vars.checkbox.freestand;
		SaveBind(vars.keybind.freestand, "fs", &antiaim);






		antiaim["pt_"] = vars.checkbox.pitch;
		SaveBind(vars.keybind.pitch, "pt", &antiaim);
		antiaim["ym_"] = vars.checkbox.yawmodifier;
		SaveBind(vars.keybind.yawmodifier, "ym", &antiaim);

		antiaim["yb_"] = vars.checkbox.yawbase;
		SaveBind(vars.keybind.yawbase, "yb", &antiaim);

		//vars.antiaim.manualindicator
		antiaim["Indicator"] = vars.antiaim.indicatorr; {
			antiaim["color"]["red"] = vars.antiaim.manualindicator.get_red();
			antiaim["color"]["green"] = vars.antiaim.manualindicator.get_green();
			antiaim["color"]["blue"] = vars.antiaim.manualindicator.get_blue();
			antiaim["color"]["alpha"] = vars.antiaim.manualindicator.get_alpha();
		}
		antiaim["LBYFlick"] = vars.antiaim.LBYFlick;
		SaveBind(vars.antiaim.LBYF, "LBYFlick_Key", &antiaim);
		antiaim["LBYFlick_Intervals"] = vars.antiaim.LBYFlick_Intervals;
		antiaim["LBYFlick_Time"] = vars.antiaim.LBYFlick_Intervals_S;
		antiaim["Inverter_Jitter"] = vars.ragebot.autoinvert;
		antiaim["Auto_Speed"] = vars.ragebot.autospeed;
		antiaim["Speed"] = vars.ragebot.invertspeed;
		antiaim["Invert_OS"] = vars.antiaim.onshottt;
		antiaim["Smart_Jitter"] = vars.antiaim.antionshotretard;
		antiaim["Fake_Duck_Mode"] = vars.antiaim.FD;
		SaveBind(vars.antiaim.inv_LBYF, "Invert_LBY", &antiaim);
		SaveBind(vars.antiaim.BYEBYE, "FloodServer", &antiaim);
		//
		SaveBind(vars.antiaim.fakeduck, "fakeduck", &antiaim);
		SaveBind(vars.antiaim.slowwalk, "slowwalk", &antiaim);
		SaveBind(vars.antiaim.inverter, "inverter", &antiaim);


		SaveBind(vars.movable.OnPeekAA, "OnPeekAA", &antiaim);
		antiaim["OnPeekAA2"] = vars.movable.OnPeekAAM;

		SaveBind(vars.movable.OnshotAAA, "OnShotAA", &antiaim);
		antiaim["OnShotAA2"] = vars.movable.OnshotAA;

		SaveBind(vars.movable.HideAA, "HideAA", &antiaim);
		antiaim["HideAA2"] = vars.movable.HideAAM;


		SaveBind(vars.antiaim.manual.edgeyaw, "EdgeYaw2", &antiaim);
		SaveBind(vars.antiaim.manual.left, "PLeft", &antiaim);
		SaveBind(vars.antiaim.manual.right, "PRight", &antiaim);
		SaveBind(vars.antiaim.fakepeek, "FakePeek", &antiaim);

		auto& antiaim_override = antiaim["antiaim_override"]; {
			antiaim_override["enable"] = vars.antiaim.aa_override.enable;
			SaveBind(vars.antiaim.aa_override.left, "left", &antiaim_override);
			SaveBind(vars.antiaim.aa_override.right, "right", &antiaim_override);
			SaveBind(vars.antiaim.aa_override.back, "back", &antiaim_override);
		}
		auto& fakelag = antiaim["fakelag"]; {
			fakelag["type"] = vars.antiaim.fakelag;
			fakelag["factor"] = vars.antiaim.fakelagfactor;
			fakelag["fakelag_min"] = vars.antiaim.fakelag_min;
			fakelag["fakelag_jit"] = vars.antiaim.fakelagjitter;
			fakelag["onshot"] = vars.antiaim.fakelagonshot;
			SaveBind(vars.keybind.fakelag, "bind", &fakelag);
			fakelag["fl"] = vars.checkbox.fakelag;
			fakelag["flags"] = vars.checkbox.fakelagflags;
			fakelag["trigger"] = vars.checkbox.fakelagaa;
		}
	}

	auto& players = json["players"]; {
		auto& esp = players["esp"]; {
			esp["enable"] = vars.visuals.enable;
			esp["dormant"] = vars.visuals.dormant;

			esp["box"] = vars.visuals.box; {
				esp["box_color"]["red"] = vars.visuals.box_color.get_red();
				esp["box_color"]["green"] = vars.visuals.box_color.get_green();
				esp["box_color"]["blue"] = vars.visuals.box_color.get_blue();
				esp["box_color"]["alpha"] = vars.visuals.box_color.get_alpha();
			}

			esp["skeleton"] = vars.visuals.skeleton; {
				esp["skeleton_color"]["red"] = vars.visuals.skeleton_color.get_red();
				esp["skeleton_color"]["green"] = vars.visuals.skeleton_color.get_green();
				esp["skeleton_color"]["blue"] = vars.visuals.skeleton_color.get_blue();
				esp["skeleton_color"]["alpha"] = vars.visuals.skeleton_color.get_alpha();
			}

			esp["healthbar"]["enable"] = vars.visuals.healthbar;
			esp["healthbar"]["override_hp"] = vars.visuals.override_hp; {
				esp["healthbar"]["hp_color"]["red"] = vars.visuals.hp_color.get_red();
				esp["healthbar"]["hp_color"]["green"] = vars.visuals.hp_color.get_green();
				esp["healthbar"]["hp_color"]["blue"] = vars.visuals.hp_color.get_blue();
				esp["healthbar"]["hp_color"]["alpha"] = vars.visuals.hp_color.get_alpha();
			}

			esp["name"] = vars.visuals.name; {
				esp["name_color"]["red"] = vars.visuals.name_color.get_red();
				esp["name_color"]["green"] = vars.visuals.name_color.get_green();
				esp["name_color"]["blue"] = vars.visuals.name_color.get_blue();
				esp["name_color"]["alpha"] = vars.visuals.name_color.get_alpha();
			}

			esp["weapon"] = vars.visuals.weapon; {
				esp["weapon_color"]["red"] = vars.visuals.weapon_color.get_red();
				esp["weapon_color"]["green"] = vars.visuals.weapon_color.get_green();
				esp["weapon_color"]["blue"] = vars.visuals.weapon_color.get_blue();
				esp["weapon_color"]["alpha"] = vars.visuals.weapon_color.get_alpha();
			}

			esp["Weapon_Icon"] = vars.visuals.weapon_icon;

			esp["ammo"] = vars.visuals.ammo; {
				esp["ammo_color"]["red"] = vars.visuals.ammo_color.get_red();
				esp["ammo_color"]["green"] = vars.visuals.ammo_color.get_green();
				esp["ammo_color"]["blue"] = vars.visuals.ammo_color.get_blue();
				esp["ammo_color"]["alpha"] = vars.visuals.ammo_color.get_alpha();
			}

			esp["flags"] = vars.visuals.flags; {
				esp["flags_color"]["red"] = vars.visuals.flags_color.get_red();
				esp["flags_color"]["green"] = vars.visuals.flags_color.get_green();
				esp["flags_color"]["blue"] = vars.visuals.flags_color.get_blue();
				esp["flags_color"]["alpha"] = vars.visuals.flags_color.get_alpha();
			}
			esp["show_multipoint"] = vars.visuals.shot_multipoint;
			SaveColor(vars.visuals.aimbot_color, "AimbotCol", &esp);
			esp["out_of_fov"]["enable"] = vars.visuals.out_of_fov; {
				esp["out_of_fov"]["size"] = vars.visuals.out_of_fov_size;
				esp["out_of_fov"]["distance"] = vars.visuals.out_of_fov_distance;

				esp["out_of_fov"]["color"]["red"] = vars.visuals.out_of_fov_color.get_red();
				esp["out_of_fov"]["color"]["green"] = vars.visuals.out_of_fov_color.get_green();
				esp["out_of_fov"]["color"]["blue"] = vars.visuals.out_of_fov_color.get_blue();
				esp["out_of_fov"]["color"]["alpha"] = vars.visuals.out_of_fov_color.get_alpha();
			}
			esp["FOVFLAGS"] = vars.visuals.FOVArrowsFlags;
			esp["MollyWarning"] = vars.visuals.MollyWarning;
			SaveColor(vars.visuals.MollyColor, "MollyColor", &esp);
		}
		auto& models = players["models"]; {  // ебал € в рот это все переносить пизда (@opai), пиздец еще и лоад делать ƒј ЅЋя“№
			auto& chams_cfg = models["chams"]; {

				chams_cfg["enemy"]["enable"] = vars.visuals.chams; {
					chams_cfg["enemy"]["visible_color"]["red"] = vars.visuals.chamscolor.get_red();
					chams_cfg["enemy"]["visible_color"]["green"] = vars.visuals.chamscolor.get_green();
					chams_cfg["enemy"]["visible_color"]["blue"] = vars.visuals.chamscolor.get_blue();
					chams_cfg["enemy"]["visible_color"]["alpha"] = vars.visuals.chamscolor.get_alpha();

					chams_cfg["enemy"]["through_walls"] = vars.visuals.chamsxqz;
					chams_cfg["enemy"]["through_walls_color"]["red"] = vars.visuals.chamscolor_xqz.get_red();
					chams_cfg["enemy"]["through_walls_color"]["green"] = vars.visuals.chamscolor_xqz.get_green();
					chams_cfg["enemy"]["through_walls_color"]["blue"] = vars.visuals.chamscolor_xqz.get_blue();
					chams_cfg["enemy"]["through_walls_color"]["alpha"] = vars.visuals.chamscolor_xqz.get_alpha();

					chams_cfg["enemy"]["overlay_color"]["red"] = vars.visuals.glow_col.get_red();
					chams_cfg["enemy"]["overlay_color"]["green"] = vars.visuals.glow_col.get_green();
					chams_cfg["enemy"]["overlay_color"]["blue"] = vars.visuals.glow_col.get_blue();
					chams_cfg["enemy"]["overlay_color"]["alpha"] = vars.visuals.glow_col.get_alpha();

					chams_cfg["enemy"]["xqz_overlay_color"]["red"] = vars.visuals.glow_col_xqz.get_red();
					chams_cfg["enemy"]["xqz_overlay_color"]["green"] = vars.visuals.glow_col_xqz.get_green();
					chams_cfg["enemy"]["xqz_overlay_color"]["blue"] = vars.visuals.glow_col_xqz.get_blue();
					chams_cfg["enemy"]["xqz_overlay_color"]["alpha"] = vars.visuals.glow_col_xqz.get_alpha();

					chams_cfg["enemy"]["material"] = vars.visuals.chamstype;
					chams_cfg["enemy"]["phong_exponent"] = vars.visuals.phong_exponent;
					chams_cfg["enemy"]["phong_boost"] = vars.visuals.phong_boost;
					chams_cfg["enemy"]["rim"] = vars.visuals.rim;
					chams_cfg["enemy"]["brightness"] = vars.visuals.chams_brightness;
					chams_cfg["enemy"]["pearlescent"] = vars.visuals.pearlescent;
					chams_cfg["enemy"]["overlay"] = vars.visuals.overlay;
					chams_cfg["enemy"]["overlay_xqz"] = vars.visuals.overlay_xqz;

					chams_cfg["enemy"]["metallic_color"]["red"] = vars.visuals.metallic_clr.get_red();
					chams_cfg["enemy"]["metallic_color"]["green"] = vars.visuals.metallic_clr.get_green();
					chams_cfg["enemy"]["metallic_color"]["blue"] = vars.visuals.metallic_clr.get_blue();
					chams_cfg["enemy"]["metallic_color"]["alpha"] = vars.visuals.metallic_clr.get_alpha();

					chams_cfg["enemy"]["phong_color"]["red"] = vars.visuals.metallic_clr2.get_red();
					chams_cfg["enemy"]["phong_color"]["green"] = vars.visuals.metallic_clr2.get_green();
					chams_cfg["enemy"]["phong_color"]["blue"] = vars.visuals.metallic_clr2.get_blue();
					chams_cfg["enemy"]["phong_color"]["alpha"] = vars.visuals.metallic_clr2.get_alpha();
				}

				chams_cfg["local"]["enable"] = vars.visuals.localchams; {
					chams_cfg["local"]["interpolated"] = vars.visuals.interpolated_model;
					chams_cfg["local"]["material"] = vars.visuals.localchamstype;
					chams_cfg["local"]["blend_on_scope"] = vars.visuals.blend_on_scope;
					chams_cfg["local"]["blend_value"] = vars.visuals.blend_value;

					chams_cfg["local"]["overlay_color"]["red"] = vars.visuals.local_glow_color.get_red();
					chams_cfg["local"]["overlay_color"]["green"] = vars.visuals.local_glow_color.get_green();
					chams_cfg["local"]["overlay_color"]["blue"] = vars.visuals.local_glow_color.get_blue();
					chams_cfg["local"]["overlay_color"]["alpha"] = vars.visuals.local_glow_color.get_alpha();

					chams_cfg["local"]["visible_color"]["red"] = vars.visuals.localchams_color.get_red();
					chams_cfg["local"]["visible_color"]["green"] = vars.visuals.localchams_color.get_green();
					chams_cfg["local"]["visible_color"]["blue"] = vars.visuals.localchams_color.get_blue();
					chams_cfg["local"]["visible_color"]["alpha"] = vars.visuals.localchams_color.get_alpha();

					chams_cfg["local"]["phong_exponent"] = vars.visuals.local_chams.phong_exponent;
					chams_cfg["local"]["phong_boost"] = vars.visuals.local_chams.phong_boost;
					chams_cfg["local"]["rim"] = vars.visuals.local_chams.rim;
					chams_cfg["local"]["brightness"] = vars.visuals.local_chams_brightness;
					chams_cfg["local"]["pearlescent"] = vars.visuals.local_chams.pearlescent;
					chams_cfg["local"]["overlay"] = vars.visuals.local_chams.overlay;

					chams_cfg["local"]["metallic_color"]["red"] = vars.visuals.local_chams.metallic_clr.get_red();
					chams_cfg["local"]["metallic_color"]["green"] = vars.visuals.local_chams.metallic_clr.get_green();
					chams_cfg["local"]["metallic_color"]["blue"] = vars.visuals.local_chams.metallic_clr.get_blue();
					chams_cfg["local"]["metallic_color"]["alpha"] = vars.visuals.local_chams.metallic_clr.get_alpha();

					chams_cfg["local"]["phong_color"]["red"] = vars.visuals.local_chams.metallic_clr2.get_red();
					chams_cfg["local"]["phong_color"]["green"] = vars.visuals.local_chams.metallic_clr2.get_green();
					chams_cfg["local"]["phong_color"]["blue"] = vars.visuals.local_chams.metallic_clr2.get_blue();
					chams_cfg["local"]["phong_color"]["alpha"] = vars.visuals.local_chams.metallic_clr2.get_alpha();
				}

				chams_cfg["misc_chams_2"]["interpolated"] = vars.visuals.interpolated_bt;
				chams_cfg["misc_chams_3"]["interpolated"] = vars.visuals.interpolated_dsy;
				for (int i = 0; i < 4; i++) {
					string category = "misc_chams_" + std::to_string(i);
					chams_cfg[category]["enable"] = vars.visuals.misc_chams[i].enable;
					chams_cfg[category]["material"] = vars.visuals.misc_chams[i].material;
					chams_cfg[category]["brightness"] = vars.visuals.misc_chams[i].chams_brightness;
					chams_cfg[category]["pearlescent"] = vars.visuals.misc_chams[i].pearlescent;
					chams_cfg[category]["phong_exponent"] = vars.visuals.misc_chams[i].phong_exponent;
					chams_cfg[category]["phong_boost"] = vars.visuals.misc_chams[i].phong_boost;
					chams_cfg[category]["rim"] = vars.visuals.misc_chams[i].rim;
					chams_cfg[category]["overlay"] = vars.visuals.misc_chams[i].overlay;

					chams_cfg[category]["model_color"]["red"] = vars.visuals.misc_chams[i].clr.get_red();
					chams_cfg[category]["model_color"]["green"] = vars.visuals.misc_chams[i].clr.get_green();
					chams_cfg[category]["model_color"]["blue"] = vars.visuals.misc_chams[i].clr.get_blue();
					chams_cfg[category]["model_color"]["alpha"] = vars.visuals.misc_chams[i].clr.get_alpha();

					chams_cfg[category]["overlay_color"]["red"] = vars.visuals.misc_chams[i].glow_clr.get_red();
					chams_cfg[category]["overlay_color"]["green"] = vars.visuals.misc_chams[i].glow_clr.get_green();
					chams_cfg[category]["overlay_color"]["blue"] = vars.visuals.misc_chams[i].glow_clr.get_blue();
					chams_cfg[category]["overlay_color"]["alpha"] = vars.visuals.misc_chams[i].glow_clr.get_alpha();

					chams_cfg[category]["metallic_color"]["red"] = vars.visuals.misc_chams[i].metallic_clr.get_red();
					chams_cfg[category]["metallic_color"]["green"] = vars.visuals.misc_chams[i].metallic_clr.get_green();
					chams_cfg[category]["metallic_color"]["blue"] = vars.visuals.misc_chams[i].metallic_clr.get_blue();
					chams_cfg[category]["metallic_color"]["alpha"] = vars.visuals.misc_chams[i].metallic_clr.get_alpha();

					chams_cfg[category]["phong_color"]["red"] = vars.visuals.misc_chams[i].metallic_clr2.get_red();
					chams_cfg[category]["phong_color"]["green"] = vars.visuals.misc_chams[i].metallic_clr2.get_green();
					chams_cfg[category]["phong_color"]["blue"] = vars.visuals.misc_chams[i].metallic_clr2.get_blue();
					chams_cfg[category]["phong_color"]["alpha"] = vars.visuals.misc_chams[i].metallic_clr2.get_alpha();
				}
			}
			auto& glow = models["glow"]; {
				glow["style"] = vars.visuals.glowtype;

				glow["enemy"]["enable"] = vars.visuals.glow;
				glow["enemy"]["color"]["red"] = vars.visuals.glow_color.get_red();
				glow["enemy"]["color"]["green"] = vars.visuals.glow_color.get_green();
				glow["enemy"]["color"]["blue"] = vars.visuals.glow_color.get_blue();
				glow["enemy"]["color"]["alpha"] = vars.visuals.glow_color.get_alpha();

				glow["local"]["enable"] = vars.visuals.local_glow;
				glow["local"]["color"]["red"] = vars.visuals.local_glow_clr.get_red();
				glow["local"]["color"]["green"] = vars.visuals.local_glow_clr.get_green();
				glow["local"]["color"]["blue"] = vars.visuals.local_glow_clr.get_blue();
				glow["local"]["color"]["alpha"] = vars.visuals.local_glow_clr.get_alpha();
			}
		}
		auto& misc = players["misc"]; {
			misc["spec_list"] = vars.visuals.speclist;
			misc["monitor"] = vars.visuals.monitor;
			misc["nadepred"]["enable"] = vars.visuals.nadepred;

			SaveColor(vars.visuals.nadepred_color, "color", &misc["nadepred"]);

			misc["indicators"] = vars.visuals.indicators;
			misc["aspect_ratio"] = vars.visuals.aspect_ratio;
			misc["thirdperson"]["enable"] = vars.misc.thirdperson;
			misc["thirdperson"]["distance"] = vars.visuals.thirdperson_dist;
			SaveBind(vars.misc.thirdperson_bind, "bind", &misc["thirdperson"]);

			misc["eventlog"]["enable"] = vars.visuals.eventlog;

			misc["eventlog"]["color"]["red"] = vars.visuals.eventlog_color.get_red();
			misc["eventlog"]["color"]["green"] = vars.visuals.eventlog_color.get_green();
			misc["eventlog"]["color"]["blue"] = vars.visuals.eventlog_color.get_blue();
			misc["eventlog"]["color"]["alpha"] = vars.visuals.eventlog_color.get_alpha();

			misc["hitmarker"]["enable"] = vars.visuals.hitmarker;
			misc["hitmarker"]["enable_sound"] = vars.visuals.hitmarker_sound;

			misc["hitmarker"]["color"]["red"] = vars.visuals.hitmarker_color.get_red();
			misc["hitmarker"]["color"]["green"] = vars.visuals.hitmarker_color.get_green();
			misc["hitmarker"]["color"]["blue"] = vars.visuals.hitmarker_color.get_blue();
			misc["hitmarker"]["color"]["alpha"] = vars.visuals.hitmarker_color.get_alpha();




		}
	}

	auto& world = json["world"]; {
		world["ScopeT"] = vars.visuals.ScopeTHing;
		SaveColor(vars.visuals.ScopeColor, "ScopeCol", &world);
		auto& tracers = world["tracers"]; {
			tracers["bullet_tracer"]["size"] = vars.visuals.impacts_size;
			tracers["bullet_tracer"]["enable"] = vars.visuals.bullet_tracer; {
				tracers["bullet_tracer"]["sprite"] = vars.visuals.bullet_tracer_type;
				tracers["bullet_tracer"]["color"]["red"] = vars.visuals.bullet_tracer_color.get_red();
				tracers["bullet_tracer"]["color"]["green"] = vars.visuals.bullet_tracer_color.get_green();
				tracers["bullet_tracer"]["color"]["blue"] = vars.visuals.bullet_tracer_color.get_blue();
				tracers["bullet_tracer"]["color"]["alpha"] = vars.visuals.bullet_tracer_color.get_alpha();

				tracers["bullet_tracer"]["local"] = vars.visuals.bullet_tracer_local;
				tracers["bullet_tracer"]["local_color"]["red"] = vars.visuals.bullet_tracer_local_color.get_red();
				tracers["bullet_tracer"]["local_color"]["green"] = vars.visuals.bullet_tracer_local_color.get_green();
				tracers["bullet_tracer"]["local_color"]["blue"] = vars.visuals.bullet_tracer_local_color.get_blue();
				tracers["bullet_tracer"]["local_color"]["alpha"] = vars.visuals.bullet_tracer_local_color.get_alpha();
			}

			tracers["bullet_impact"]["enable"] = vars.visuals.bullet_impact; {
				tracers["bullet_impact"]["color"]["red"] = vars.visuals.bullet_impact_color.get_red();
				tracers["bullet_impact"]["color"]["green"] = vars.visuals.bullet_impact_color.get_green();
				tracers["bullet_impact"]["color"]["blue"] = vars.visuals.bullet_impact_color.get_blue();
				tracers["bullet_impact"]["color"]["alpha"] = vars.visuals.bullet_impact_color.get_alpha();

				tracers["bullet_impact_client"]["color"]["red"] = vars.visuals.client_impact_color.get_red();
				tracers["bullet_impact_client"]["color"]["green"] = vars.visuals.client_impact_color.get_green();
				tracers["bullet_impact_client"]["color"]["blue"] = vars.visuals.client_impact_color.get_blue();
				tracers["bullet_impact_client"]["color"]["alpha"] = vars.visuals.client_impact_color.get_alpha();
			}
		}
		auto& effects = world["effects"]; {
			effects["removals"] = vars.visuals.remove;
			effects["force_crosshair"] = vars.visuals.force_crosshair;
			effects["kill_effect"] = vars.visuals.kill_effect;
			effects["world_fov"] = vars.misc.worldfov;
			effects["viewmodel_fov"] = vars.misc.viewmodelfov;

			effects["nightmode"]["enable"] = vars.visuals.nightmode; {
				effects["nightmode"]["color"]["world"]["red"] = vars.visuals.nightmode_color.get_red();
				effects["nightmode"]["color"]["world"]["green"] = vars.visuals.nightmode_color.get_green();
				effects["nightmode"]["color"]["world"]["blue"] = vars.visuals.nightmode_color.get_blue();
				effects["nightmode"]["color"]["world"]["alpha"] = vars.visuals.nightmode_color.get_alpha();

				effects["nightmode"]["color"]["prop"]["red"] = vars.visuals.nightmode_prop_color.get_red();
				effects["nightmode"]["color"]["prop"]["green"] = vars.visuals.nightmode_prop_color.get_green();
				effects["nightmode"]["color"]["prop"]["blue"] = vars.visuals.nightmode_prop_color.get_blue();
				effects["nightmode"]["color"]["prop"]["alpha"] = vars.visuals.nightmode_prop_color.get_alpha();

				effects["nightmode"]["color"]["skybox"]["red"] = vars.visuals.nightmode_skybox_color.get_red();
				effects["nightmode"]["color"]["skybox"]["green"] = vars.visuals.nightmode_skybox_color.get_green();
				effects["nightmode"]["color"]["skybox"]["blue"] = vars.visuals.nightmode_skybox_color.get_blue();
				effects["nightmode"]["color"]["skybox"]["alpha"] = vars.visuals.nightmode_skybox_color.get_alpha();
			}
		}
	}

	auto& misc = json["misc"]; {
		misc["aml"] = vars.visuals.ambientlight;
		SaveColor(vars.visuals.ambient_color, "amlc", &misc);
		misc["anti_untrusted"] = vars.misc.antiuntrusted;
		misc["bunnyhop"] = vars.misc.bunnyhop;
		misc["knifebot"] = vars.misc.knifebot;
		misc["clantag"] = vars.visuals.clantagspammer;
		misc["hold_firinganims"] = vars.misc.hold_firinganims;
		misc["legs"] = vars.antiaim.Legs;
		misc["air"] = vars.antiaim.airl;
		//misc["legacy"] = vars.movable.leagcy;
		misc["autobuy"]["enable"] = vars.misc.autobuy.enable;
		misc["autobuy"]["main"] = vars.misc.autobuy.main;
		misc["autobuy"]["pistol"] = vars.misc.autobuy.pistol;
		misc["autobuy"]["misc"] = vars.misc.autobuy.misc;
		misc["Keybinds"]["Enable"] = vars.menu.keybinds;
		misc["Keybinds"]["X"] = vars.menu.keybindX;
		misc["Keybinds"]["Y"] = vars.menu.keybindY;
		misc["Indicator"]["Size"] = vars.visuals.disclay;
		misc["Indicator"]["Offset"] = vars.visuals.indicatoroffset;
		misc["Recharge_Indicator"] = vars.visuals.DTindi;
		misc["viewmodel"]["x"] = vars.misc.viewmodel_x;
		misc["viewmodel"]["y"] = vars.misc.viewmodel_y;
		misc["viewmodel"]["z"] = vars.misc.viewmodel_z;
		misc["sky"] = vars.misc.skyboxx;
		SaveColor(vars.movable.PCol, "KeybindColor", &misc);
		misc["PreserveKillfeed"] = vars.visuals.PreserveKills;
		misc["DTIX"] = vars.visuals.doubletapindicator.X;
		misc["DTIY"] = vars.visuals.doubletapindicator.Y;
		misc["killsay"] = vars.misc.killsaysandshit.killsay;
		misc["deathsay"] = vars.misc.killsaysandshit.deathsay;
		misc["enabledkillsay"] = vars.misc.killsaysandshit.enablekillsay;
		misc["enableddeathsay"] = vars.misc.killsaysandshit.enabledeathsay;
		misc["killsayflags"] = vars.misc.killsaysandshit.killsayflags;
		misc["localmemes"] = vars.visuals.localmemes;
		SaveColor(vars.visuals.localmemecolor, "localmemescol", &misc);
		misc["chatsay"] = vars.misc.chatspam;
		misc["bombtimer"] = vars.visuals.BombT;
		SaveColor(vars.visuals.BombColor,"bombtimercol" ,&misc);
	}

	static TCHAR path[256];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
	{
		folder = std::string(path) + ("\\.FrezzyHook\\");
		file = std::string(path) + ("\\.FrezzyHook\\" + cfg_name);
	}

	CreateDirectory(folder.c_str(), NULL);
	std::ofstream file_out(file);
	/*	string decrypted = configuration.toStyledString();
		ByteArray txt(decrypted.begin(), decrypted.end()), enc;
		Aes256::encrypt(encryption_key, txt, enc)*/;
		if (file_out.good())
			file_out << configuration/*string(enc.begin(), enc.end())*/;
		file_out.close();

		PNotify("Config", "Saved Successfully", 2);
}
ByteArray ReadAllBytes(char const* filename)
{
	ifstream ifs(filename, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();

	ByteArray result(pos);

	ifs.seekg(0, ios::beg);
	ifs.read((char*)&result[0], pos);

	return result;
}
void CConfig::Load(string cfg_name)
{
	json_t configuration;
	static TCHAR path[MAX_PATH];
	std::string folder, file;
	std::string script_folder, server_folder;
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
	{
		folder = std::string(path) + ("\\.FrezzyHook\\");
		file = std::string(path) + ("\\.FrezzyHook\\" + cfg_name);
		script_folder = std::string(path) + ("\\.FrezzyHook\\scripts");
		server_folder = std::string(path) + ("\\.FrezzyHook\\Server");
	}
	CreateDirectory(folder.c_str(), NULL);
	CreateDirectory(script_folder.c_str(), NULL);
	CreateDirectory(server_folder.c_str(), NULL);
	std::ifstream file_out(file);
	//string in;
	if (file_out.good())
		file_out >> configuration/*in*/;
	//file_out.close();
	//if (in.size() == 0)
	//	return;

	//ByteArray txt = ReadAllBytes(file.c_str()), enc;
	//Aes256::decrypt(encryption_key, txt, enc);
	//if (enc.size() == 0)
	//	return;
	/*configuration = json_t(string(enc.begin(), enc.end()))*/;
	if (!configuration.isMember("config"))
		return;

	auto& json = configuration["config"];
	json["name"] = cfg_name;
	vars.menu.LoadedConfig = cfg_name;
	auto& ragebot = json["ragebot"]; {
		LoadBool(&vars.ragebot.enable, "enable", ragebot);
		LoadBool(&vars.ragebot.autoshoot, "auto_shoot", ragebot);
		LoadBool(&vars.ragebot.autoscope, "auto_scope", ragebot);
		LoadBool(&vars.ragebot.delayshot, "delayshot", ragebot);
		LoadBool(&vars.ragebot.hitchance_consider_hitbox, "hitchance_consider_hitbox", ragebot);
		LoadBind(vars.keybind.onshot, "onshot", ragebot);
		LoadBool(&vars.ragebot.posadj, "posadj", ragebot);
		LoadBool(&vars.ragebot.fronttrack, "ftack", ragebot);
		LoadBool(&vars.ragebot.backshoot_bt, "backshoot_bt", ragebot);
		//LoadBool(&vars.ragebot.resolver, "resolver", ragebot);
		//LoadBool(&vars.ragebot.onshot_invert, "onshot_invert", ragebot);
		//LoadBool(&vars.ragebot.onshot_resolver, "onshot_resolver", ragebot);
		LoadBool(&vars.ragebot.shotrecord, "shot_record", ragebot);
		LoadBool(&vars.ragebot.NOnsilent, "visualize", ragebot);
		LoadBind(vars.ragebot.override_dmg, "override_dmg", ragebot);
		LoadBind(vars.ragebot.force_body, "force_body", ragebot);
		LoadBind(vars.ragebot.double_tap, "double_tap", ragebot);
		//	LoadBind(vars.ragebot.hide_shots, "hide_shots", ragebot);
		LoadInt(&vars.ragebot.BDT, "DT_Defensive", ragebot);
		LoadInt(&vars.antiaim.DTT, "Recharge_Mode", ragebot);
		LoadBind(vars.ragebot.override_dmg, "override_dmg", ragebot);
		LoadColor(&vars.ragebot.shot_clr, "shot_clr", ragebot);
		LoadBind(vars.keybind.safepoint, "Safepoint", ragebot);
		LoadUInt(&vars.ragebot.DoubletapFlags, "DTFLAG", ragebot);
		LoadInt(&vars.ragebot.LagExploitS, "DTLAG", ragebot);
		auto& weapon_cfg = ragebot["weapon_cfg"];
		for (int i = 0; i < 7; i++) {
			string category = "weapon_" + std::to_string(i);
			LoadBool(&vars.ragebot.weapon[i].enable, "enable", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].mindamage, "mindamage", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].mindamage_override, "mindamage_override", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].hitchance, "hitchance", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].hitchancetype, "hitchance_type", weapon_cfg[category]);
			LoadBool(&vars.ragebot.weapon[i].multipoint, "multipoint", weapon_cfg[category]);
			LoadBool(&vars.ragebot.weapon[i].quickstop, "quickstop", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].quickstop_options, "quickstop_options", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].pointscale_head, "pointscale_head", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].pointscale_body, "pointscale_body", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].pointscale_limbs, "pointscale_limbs", weapon_cfg[category]);
			LoadUInt(&vars.ragebot.weapon[i].hitscan, "hitscan", weapon_cfg[category]);
			LoadUInt(&vars.ragebot.weapon[i].hitscan_baim, "hitscan_baim", weapon_cfg[category]);
			LoadUInt(&vars.ragebot.weapon[i].baim, "baim", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].baim_under_hp, "baim_under_hp", weapon_cfg[category]);
			LoadBool(&vars.ragebot.weapon[i].adaptive_baim, "adaptive_baim", weapon_cfg[category]);
			LoadInt(&vars.ragebot.weapon[i].max_misses, "max_misses", weapon_cfg[category]);
		}
	}

	auto& antiaim = json["antiaim"]; {
		LoadBool(&vars.antiaim.enable, "enable", antiaim);
		LoadBind(vars.antiaim.fakeduck, "fakeduck", antiaim);
		LoadBind(vars.antiaim.slowwalk, "slowwalk", antiaim);
		LoadInt(&vars.antiaim.desync_type, "desync_type", antiaim);
		LoadInt(&vars.antiaim.slowwalk_speed, "slowwalk_speed", antiaim);
		LoadBool(&vars.ragebot.customAA, "Custom_AA", antiaim);
		LoadInt(&vars.antiaim.baseangle, "Base_Angle", antiaim);
		LoadInt(&vars.antiaim.baseangle_direction, "Base_Dir", antiaim);
		LoadInt(&vars.antiaim.yaw_angle_start, "Yaw", antiaim);
		LoadInt(&vars.antiaim.yawmodifier, "Modifier", antiaim);
		LoadInt(&vars.antiaim.yawmodifier_value, "Modifier_Value", antiaim);
		LoadInt(&vars.antiaim.customAAT, "Custom_Desync", antiaim);
		LoadInt(&vars.menu.p, "Ex_Desync", antiaim);
		LoadInt(&vars.antiaim.DesyncAMM, "Desync_Value", antiaim);
		LoadInt(&vars.movable.freestand, "freestand", antiaim);
		LoadColor(&vars.antiaim.manualindicator, "color", antiaim);
		LoadBool(&vars.antiaim.LBYFlick, "LBYFlick", antiaim);
		LoadBind(vars.antiaim.LBYF, "LBYFlick_Key", antiaim);
		LoadInt(&vars.antiaim.LBYFlick_Intervals, "LBYFlick_Intervals", antiaim);
		LoadInt(&vars.antiaim.LBYFlick_Intervals_S, "LBYFlick_Time", antiaim);
		LoadBool(&vars.ragebot.autoinvert, "Inverter_Jitter", antiaim);
		LoadBool(&vars.ragebot.autospeed, "Auto_Speed", antiaim);
		LoadBool(&vars.antiaim.onshottt, "Invert_OS", antiaim);
		LoadBool(&vars.antiaim.antionshotretard, "Smart_Jitter", antiaim);
		LoadInt(&vars.ragebot.invertspeed, "Speed", antiaim);
		LoadInt(&vars.antiaim.FD, "Fake_Duck_Mode", antiaim);
		LoadBind(vars.antiaim.inv_LBYF, "Invert_LBY", antiaim);
		LoadBind(vars.antiaim.BYEBYE, "FloodServer", antiaim);
		LoadBind(vars.antiaim.inverter, "inverter", antiaim);

		LoadBind(vars.movable.OnPeekAA, "OnPeekAA", antiaim);
		LoadBool(&vars.movable.OnPeekAAM, "OnPeekAA2", antiaim);

		LoadBind(vars.movable.OnshotAAA, "OnShotAA", antiaim);
		LoadBool(&vars.movable.OnshotAA, "OnShotAA2", antiaim);

		LoadBind(vars.movable.HideAA, "HideAA", antiaim);
		LoadBool(&vars.movable.HideAAM, "HideAA2", antiaim);

		LoadBind(vars.antiaim.manual.edgeyaw, "EdgeYaw2", antiaim);
		LoadBind(vars.antiaim.manual.left, "PLeft", antiaim);
		LoadBind(vars.antiaim.manual.right, "PRight", antiaim);
		LoadBind(vars.antiaim.fakepeek, "FakePeek", antiaim);
		LoadBool(&vars.movable.autodir, "autodir_", antiaim);
		LoadBind(vars.keybind.autodir, "autodir", antiaim);
		LoadBool(&vars.checkbox.freestand, "fs_", antiaim);
		LoadBind(vars.keybind.freestand, "fs", antiaim);
		LoadBool(&vars.checkbox.pitch, "pt_", antiaim);
		LoadBind(vars.keybind.pitch, "pt", antiaim);
		LoadBool(&vars.checkbox.yawmodifier, "ym_", antiaim);
		LoadBind(vars.keybind.yawmodifier, "ym", antiaim);
		LoadBool(&vars.checkbox.yawbase, "yb_", antiaim);
		LoadBind(vars.keybind.yawbase, "yb", antiaim);

		auto& antiaim_override = antiaim["antiaim_override"]; {
			LoadBool(&vars.antiaim.aa_override.enable, "enable", antiaim_override);
			LoadBind(vars.antiaim.aa_override.left, "left", antiaim_override);
			LoadBind(vars.antiaim.aa_override.right, "right", antiaim_override);
			LoadBind(vars.antiaim.aa_override.back, "back", antiaim_override);
		}
		auto& fakelag = antiaim["fakelag"]; {
			LoadInt(&vars.antiaim.fakelag, "type", fakelag);
			LoadInt(&vars.antiaim.fakelagfactor, "factor", fakelag);
			LoadInt(&vars.antiaim.fakelag_min, "fakelag_min", fakelag);
			LoadInt(&vars.antiaim.fakelagjitter, "fakelag_jit", fakelag);
			LoadInt(&vars.antiaim.fakelagonshot, "onshot", fakelag);
			LoadBind(vars.keybind.fakelag, "bind", fakelag);
			LoadBool(&vars.checkbox.fakelag, "fl", fakelag);
			LoadUInt(&vars.checkbox.fakelagflags, "flags", fakelag);
			LoadInt(&vars.checkbox.fakelagaa, "trigger", fakelag);



		}
	}

	auto& players = json["players"]; {
		auto& esp = players["esp"];

		LoadBool(&vars.visuals.enable, "enable", esp);
		LoadBool(&vars.visuals.dormant, "dormant", esp);
		LoadBool(&vars.visuals.skeleton, "skeleton", esp);
		LoadColor(&vars.visuals.skeleton_color, "skeleton_color", esp);
		LoadBool(&vars.visuals.box, "box", esp);
		LoadColor(&vars.visuals.box_color, "box_color", esp);
		LoadBool(&vars.visuals.healthbar, "enable", esp["healthbar"]);
		LoadBool(&vars.visuals.override_hp, "override_hp", esp["healthbar"]);
		LoadColor(&vars.visuals.hp_color, "hp_color", esp["healthbar"]);

		LoadBool(&vars.visuals.name, "name", esp);
		LoadColor(&vars.visuals.name_color, "name_color", esp);

		LoadBool(&vars.visuals.weapon, "weapon", esp);
		LoadColor(&vars.visuals.weapon_color, "weapon_color", esp);
		LoadBool(&vars.visuals.weapon_icon, "Weapon_Icon", esp);


		LoadBool(&vars.visuals.ammo, "ammo", esp);
		LoadColor(&vars.visuals.ammo_color, "ammo_color", esp);

		LoadUInt(&vars.visuals.flags, "flags", esp);
		LoadColor(&vars.visuals.flags_color, "flags_color", esp);
		LoadBool(&vars.visuals.shot_multipoint, "show_multipoint", esp);
		LoadColor(&vars.visuals.aimbot_color, "AimbotCol", esp);
		LoadBool(&vars.visuals.out_of_fov, "enable", esp["out_of_fov"]);
		LoadInt(&vars.visuals.out_of_fov_size, "size", esp["out_of_fov"]);
		LoadInt(&vars.visuals.out_of_fov_distance, "distance", esp["out_of_fov"]);
		LoadColor(&vars.visuals.out_of_fov_color, "color", esp["out_of_fov"]);
		LoadUInt(&vars.visuals.FOVArrowsFlags, "FOVFLAGS", esp);
		LoadUInt(&vars.visuals.MollyWarning, "MollyWarning", esp);
		LoadColor(&vars.visuals.MollyColor, "MollyColor", esp);
		auto& models = players["models"]; {
			auto& chams_cfg = models["chams"]; {
				LoadBool(&vars.visuals.chams, "enable", chams_cfg["enemy"]);
				LoadBool(&vars.visuals.chamsxqz, "through_walls", chams_cfg["enemy"]);
				LoadColor(&vars.visuals.chamscolor, "visible_color", chams_cfg["enemy"]);
				LoadColor(&vars.visuals.chamscolor_xqz, "through_walls_color", chams_cfg["enemy"]);
				LoadColor(&vars.visuals.glow_col, "overlay_color", chams_cfg["enemy"]);
				LoadColor(&vars.visuals.glow_col_xqz, "xqz_overlay_color", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.overlay, "overlay", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.overlay_xqz, "overlay_xqz", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.chamstype, "material", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.phong_exponent, "phong_exponent", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.phong_boost, "phong_boost", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.rim, "rim", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.chams_brightness, "brightness", chams_cfg["enemy"]);
				LoadInt(&vars.visuals.pearlescent, "pearlescent", chams_cfg["enemy"]);

				LoadColor(&vars.visuals.metallic_clr, "metallic_color", chams_cfg["enemy"]);
				LoadColor(&vars.visuals.metallic_clr2, "phong_color", chams_cfg["enemy"]);

				LoadBool(&vars.visuals.localchams, "enable", chams_cfg["local"]); {
					LoadBool(&vars.visuals.interpolated_model, "interpolated", chams_cfg["local"]);
					LoadBool(&vars.visuals.blend_on_scope, "blend_on_scope", chams_cfg["local"]);
					LoadInt(&vars.visuals.blend_value, "blend_value", chams_cfg["local"]);
					LoadBool(&vars.visuals.localchams, "enable", chams_cfg["local"]);
					LoadColor(&vars.visuals.localchams_color, "visible_color", chams_cfg["local"]);
					LoadColor(&vars.visuals.local_glow_color, "overlay_color", chams_cfg["local"]);
					LoadInt(&vars.visuals.localchamstype, "material", chams_cfg["local"]);
					LoadInt(&vars.visuals.local_chams.phong_exponent, "phong_exponent", chams_cfg["local"]);
					LoadInt(&vars.visuals.local_chams.phong_boost, "phong_boost", chams_cfg["local"]);
					LoadInt(&vars.visuals.local_chams.rim, "rim", chams_cfg["local"]);
					LoadInt(&vars.visuals.local_chams_brightness, "brightness", chams_cfg["local"]);
					LoadInt(&vars.visuals.local_chams.pearlescent, "pearlescent", chams_cfg["local"]);
					LoadInt(&vars.visuals.local_chams.overlay, "overlay", chams_cfg["local"]);

					LoadColor(&vars.visuals.local_chams.metallic_clr, "metallic_color", chams_cfg["local"]);
					LoadColor(&vars.visuals.local_chams.metallic_clr2, "phong_color", chams_cfg["local"]);
				}

				LoadBool(&vars.visuals.interpolated_bt, "interpolated", chams_cfg["misc_chams_2"]);
				LoadBool(&vars.visuals.interpolated_dsy, "interpolated", chams_cfg["misc_chams_3"]);

				for (int i = 0; i < 4; i++) {
					string category = "misc_chams_" + std::to_string(i);
					LoadBool(&vars.visuals.misc_chams[i].enable, "enable", chams_cfg[category]);
					LoadColor(&vars.visuals.misc_chams[i].clr, "model_color", chams_cfg[category]);
					LoadColor(&vars.visuals.misc_chams[i].glow_clr, "overlay_color", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].material, "material", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].phong_exponent, "phong_exponent", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].phong_boost, "phong_boost", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].rim, "rim", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].chams_brightness, "brightness", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].pearlescent, "pearlescent", chams_cfg[category]);
					LoadInt(&vars.visuals.misc_chams[i].overlay, "overlay", chams_cfg[category]);

					LoadColor(&vars.visuals.misc_chams[i].metallic_clr, "metallic_color", chams_cfg[category]);
					LoadColor(&vars.visuals.misc_chams[i].metallic_clr2, "phong_color", chams_cfg[category]);
				}
			}
			auto& glow = models["glow"]; {
				LoadInt(&vars.visuals.glowtype, "style", glow);
				LoadBool(&vars.visuals.glow, "enable", glow["enemy"]);
				LoadColor(&vars.visuals.glow_color, "color", glow["enemy"]);

				LoadBool(&vars.visuals.local_glow, "enable", glow["local"]);
				LoadColor(&vars.visuals.local_glow_clr, "color", glow["local"]);
			}
		}

		auto& misc = players["misc"]; {

			LoadBool(&vars.visuals.speclist, "spec_list", misc);
			LoadBool(&vars.visuals.monitor, "monitor", misc);
			LoadBool(&vars.misc.thirdperson, "enable", misc["thirdperson"]);
			LoadInt(&vars.visuals.thirdperson_dist, "distance", misc["thirdperson"]);
			LoadBind(vars.misc.thirdperson_bind, "bind", misc["thirdperson"]);
			LoadUInt(&vars.visuals.indicators, "indicators", misc);
			LoadInt(&vars.visuals.aspect_ratio, "aspect_ratio", misc);
			LoadBool(&vars.visuals.eventlog, "enable", misc["eventlog"]);
			LoadColor(&vars.visuals.eventlog_color, "color", misc["eventlog"]);

			LoadInt(&vars.visuals.hitmarker, "enable", misc["hitmarker"]);
			LoadInt(&vars.visuals.hitmarker_sound, "enable_sound", misc["hitmarker"]);
			LoadColor(&vars.visuals.hitmarker_color, "color", misc["hitmarker"]);

			LoadBool(&vars.visuals.nadepred, "enable", misc["nadepred"]);
			LoadColor(&vars.visuals.nadepred_color, "color", misc["nadepred"]);
		}
	}

	auto& world = json["world"]; {
		LoadInt(&vars.visuals.ScopeTHing, "ScopeT", world);
		LoadColor(&vars.visuals.ScopeColor, "ScopeCol", world);
		auto& tracers = world["tracers"]; {
			LoadBool(&vars.visuals.bullet_tracer, "enable", tracers["bullet_tracer"]);
			LoadFloat(&vars.visuals.impacts_size, "size", tracers["bullet_tracer"]);
			LoadInt(&vars.visuals.bullet_tracer_type, "sprite", tracers["bullet_tracer"]);
			LoadColor(&vars.visuals.bullet_tracer_color, "color", tracers["bullet_tracer"]);
			LoadBool(&vars.visuals.bullet_tracer_local, "local", tracers["bullet_tracer"]);
			LoadColor(&vars.visuals.bullet_tracer_local_color, "local_color", tracers["bullet_tracer"]);

			LoadBool(&vars.visuals.bullet_impact, "enable", tracers["bullet_impact"]);
			LoadColor(&vars.visuals.bullet_impact_color, "color", tracers["bullet_impact"]);
			LoadColor(&vars.visuals.client_impact_color, "color", tracers["bullet_impact_client"]);
			;
		}
		auto& effects = world["effects"]; {
			LoadUInt(&vars.visuals.remove, "removals", effects);
			LoadBool(&vars.visuals.force_crosshair, "force_crosshair", effects);
			LoadBool(&vars.visuals.kill_effect, "kill_effect", effects);
			LoadInt(&vars.misc.worldfov, "world_fov", effects);
			LoadInt(&vars.misc.viewmodelfov, "viewmodel_fov", effects);

			LoadBool(&vars.visuals.nightmode, "enable", effects["nightmode"]);
			LoadColor(&vars.visuals.nightmode_color, "world", effects["nightmode"]["color"]);
			LoadColor(&vars.visuals.nightmode_prop_color, "prop", effects["nightmode"]["color"]);
			LoadColor(&vars.visuals.nightmode_skybox_color, "skybox", effects["nightmode"]["color"]);
		}
	}

	auto& misc = json["misc"]; {
		LoadInt(&vars.visuals.ambientlight, "aml", misc);
		LoadColor(&vars.visuals.ambient_color, "amlc", misc);
		LoadBool(&vars.misc.antiuntrusted, "anti_untrusted", misc);
		LoadBool(&vars.misc.bunnyhop, "bunnyhop", misc);
		LoadBool(&vars.misc.knifebot, "knifebot", misc);
		LoadBool(&vars.visuals.clantagspammer, "clantag", misc);
		LoadBool(&vars.misc.hold_firinganims, "hold_firinganims", misc);
		LoadBool(&vars.misc.autobuy.enable, "enable", misc["autobuy"]);
		LoadInt(&vars.misc.autobuy.pistol, "pistol", misc["autobuy"]);
		LoadInt(&vars.misc.autobuy.main, "main", misc["autobuy"]);
		LoadUInt(&vars.misc.autobuy.misc, "misc", misc["autobuy"]);

		LoadBool(&vars.menu.keybinds, "Enable", misc["Keybinds"]);
		LoadInt(&vars.menu.keybindY, "Y", misc["Keybinds"]);
		LoadInt(&vars.menu.keybindX, "X", misc["Keybinds"]);
		LoadBool(&vars.visuals.DTindi, "Recharge_Indicator", misc);
		LoadInt(&vars.visuals.disclay, "Size", misc["Indicator"]);
		LoadInt(&vars.visuals.indicatoroffset, "Offset", misc["Indicator"]);

		LoadInt(&vars.antiaim.Legs, "legs", misc);
		LoadInt(&vars.antiaim.airl, "air", misc);
		LoadBool(&vars.movable.leagcy, "legacy", misc);
		LoadInt(&vars.misc.viewmodel_x, "x", misc["viewmodel"]);
		LoadInt(&vars.misc.viewmodel_y, "y", misc["viewmodel"]);
		LoadInt(&vars.misc.viewmodel_z, "z", misc["viewmodel"]);
		LoadInt(&vars.misc.skyboxx, "sky", misc);
		LoadColor(&vars.movable.PCol, "KeybindColor", misc);
		LoadBool(&vars.visuals.PreserveKills, "PreserveKillfeed", misc);
		LoadInt(&vars.visuals.doubletapindicator.X, "DTIX", misc);
		LoadInt(&vars.visuals.doubletapindicator.Y, "DTIY", misc);
		ALoadString(&vars.misc.killsaysandshit.killsay,"killsay", misc);
		ALoadString(&vars.misc.killsaysandshit.deathsay,"deathsay", misc);
		LoadBool(&vars.misc.killsaysandshit.enablekillsay,"enabledkillsay",misc);
		LoadBool(&vars.misc.killsaysandshit.enabledeathsay,"enableddeathsay",misc);
		LoadUInt(&vars.misc.killsaysandshit.killsayflags, "killsayflags", misc);
		LoadUInt(&vars.visuals.localmemes ,"localmemes", misc);
		LoadColor(&vars.visuals.localmemecolor, "localmemescol", misc);
		LoadInt(&vars.misc.chatspam,"chatsay",misc);
		LoadUInt(&vars.visuals.BombT, "bombtimer", misc);
		LoadColor(&vars.visuals.BombColor, "bombtimercol", misc);
	}
	Msg("config " + cfg_name + " has been loaded", vars.visuals.eventlog_color);
	PNotify("Config", "Loaded Successfully", 2);
}

CConfig Config;
CGlobalVariables vars;