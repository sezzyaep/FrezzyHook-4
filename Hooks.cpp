#include "Hooks.h"
#include "netvar_manager.h"

// Include hooks
#include "PaintTraverse.h"
#include "Createmove.h"
#include "FrameStageNotify.h"
#include "EndScene.h"
#include "WndProc.h"
#include "SceneEnd.h"
#include "minhook.h"
#include "DoPostScreenEffects.h"
#include "FireEventClientSide.h"
#include "BeginFrame.h"
#include "OverRideView.h"
#include "DrawModelExecuted.h"
#include "SetupAnimation.h"
#include "EngineHook.h"
#include "TraceRay.h"
#include "detours.h"
#include "steam_sdk/steam_api.h"
#include "steam_sdk/steamtypes.h"
#include "steam_sdk/steam_gameserver.h"
//get_eye_ang::fn H::GetEyeAng;//toate sunt aici

inline static uintptr_t	p_CL_Move;
vmthook* client_hook;
ClMoveFn H::ClMove;
CLSendMoveFn H::oCLSendMove;
GetForeignFallbackFontNameFn H::GetForeignFallbackFontName;
send_net_msg_fn H::SendNetMsg;
send_datagram_fn H::SendDatagram;
SetupBonesFn H::SetupBones;
temp_entities_fn H::TempEntities;
ClientEffectCallback H::ImpactCallback;
CClientEffectRegistration* H::Effects;
DoExtraBoneProcessingFn H::DoExtraBoneProcessing;
StandardBlendingRulesFn H::StandardBlendingRules;
ShouldSkipAnimFrameFn H::ShouldSkipAnimFrame;
UpdateClientSideAnimationFn H::UpdateClientSideAnimation;
Present_t H::SteamPresent;
Reset_t H::SteamReset;
BuildTransformationsFn H::BuildTransformations;
GetEyeAnglesFn H::GetEyeAnglesO;
namespace Index
{
	// client mode interface
	const int CreateMove = 24;
	const int OverrideView = 18;
	const int DoPostScreenEffects = 44;

	// panel interface
	const int PaintTraverse = 41;

	// client interface
	const int FrameStageNotify = 37;

	// render view interface
	const int SceneEnd = 9;

	// studio render interface
	const int DrawModel = 29;
	const int BeginFrame = 9;

	// events
	const int FireEventClientSide = 9;

	// model render interface
	const int DrawModelExecute = 21;

	// predict interface
	const int InPrediction = 14;

	// engine->GetBSPTreeQuery()
	const int ListLeavesInBox = 6;

	// cvar "sv_cheats"
	const int GetBool = 13;

	// cvar "cl_camera_height_restriction_debug"
	const int InCamera = 13;

	// direct interface
	const int Present = 17;
	const int Reset = 16;

	// engine interface
	const int IsBoxVisible = 32;
	const int IsHLTV = 93;

	// renderable
	const int IsRenderableInPVS = 8;
}

cStructManager* csgo = new cStructManager();
features_t* features = new features_t();
c_interfaces interfaces;
std::vector<IGameEventListener2*> g_pGameEventManager;

void cStructManager::updatelocalplayer()
{
	if (!interfaces.engine->IsConnected() || !interfaces.engine->IsInGame()) {
		local = nullptr;
		return;
	}
	local = interfaces.ent_list->GetClientEntity(interfaces.engine->GetLocalPlayer());
}

void Msg(string str, color_t clr)
{
	if (!vars.visuals.eventlog)
		return;

	EventLogMessage Message;
	Message.Message = str;
	Message.color = clr;
	Message.TimeToEnd = interfaces.global_vars->realtime + 4;
	Message.Alpha = 255;
	Message.Xmodf = 300;
	Message.YModF = -40;
	features->Visuals->cmessages.push_back(Message);
	interfaces.cvars->Consolecolor_tPrintf(vars.visuals.eventlog_color, "[ FREZZY ] ");
	interfaces.cvars->Consolecolor_tPrintf(vars.visuals.eventlog_color, "%s\n", str.c_str());
}

void PNotify(string Owner, string Message, int icon) {
	NotifyMessage msg;
	msg.Owner = Owner;
	msg.Text = Message;
	msg.TimeToEnd = interfaces.global_vars->realtime + 5;
	msg.XmodF = 180;
	msg.alpha = 255;
	msg.icon = icon;
	msg.useicon = icon > 0;
	msg.YModF = -40;
	ImGui::PushFont(fonts::NotifyBold);
	auto TZ1 = ImGui::CalcTextSize(Owner.c_str());
	ImGui::PopFont();
	ImGui::PushFont(fonts::EventLog);
	auto TZ2 = ImGui::CalcTextSize(Message.c_str());
	ImGui::PopFont();
	if (TZ1.x > TZ2.x) {
		msg.Width = TZ1.x + 20;
	}
	else
	{
		msg.Width = TZ2.x + 20;
	}
	features->Visuals->cnotify.push_back(msg);
}

template< typename T >
T* Interface(const char* strModule, const char* strInterface)
{
	typedef T* (*CreateInterfaceFn)(const char* szName, int iReturn);
	CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule), (hs::create_interface::s().c_str()));

	if (!CreateInterface)
	{/* exit(1); */
	}
	// throw std::runtime_error(hs::create_interface_ex::s().c_str());

	return CreateInterface(strInterface, 0);
}

void CSignatures::Initialize() {
/*#ifdef _DEBUG

	Present = Reset + 4;*/


	//fix later present and reset hook 


	Reset = *reinterpret_cast<std::uintptr_t*>(csgo->Utils.FindPatternIDA(GetModuleHandleA("gameoverlayrenderer.dll"), "C7 45 ? ? ? ? ? FF 15 ? ? ? ? 8B D8") + 9);
	Present = *reinterpret_cast<std::uintptr_t*>(csgo->Utils.FindPatternIDA(GetModuleHandleA("gameoverlayrenderer.dll"), "FF 15 ? ? ? ? 8B F0 EB 1B") + 2);

	

}
template<typename T>
static T* get_interface(const char* mod_name, const char* interface_name, bool exact = false) {
	T* iface = nullptr;
	InterfaceReg* register_list;
	int part_match_len = strlen(interface_name); //-V103

	DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(mod_name), ("CreateInterface")));

	if (!interface_fn) {
		return nullptr;
	}

	unsigned int jump_start = (unsigned int)(interface_fn)+4;
	unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;

	register_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);

	for (InterfaceReg* cur = register_list; cur; cur = cur->m_pNext) {
		if (exact == true) {
			if (strcmp(cur->m_pName, interface_name) == 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else {
			if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0) //-V106
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	return iface;
}

void I::Setup()
{
	
	csgo->Utils.FindPatternIDA = [](HMODULE hModule, const char* szSignature) -> uint8_t* {
		static auto pattern_to_byte = [](const char* pattern) {
			auto bytes = std::vector<int>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		//auto Module = GetModuleHandleA(szModule);

		auto dosHeader = (PIMAGE_DOS_HEADER)hModule;
		auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)hModule + dosHeader->e_lfanew);

		auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = pattern_to_byte(szSignature);
		auto scanBytes = reinterpret_cast<std::uint8_t*>(hModule);

		auto s = patternBytes.size();
		auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i) {
			bool found = true;
			for (auto j = 0ul; j < s; ++j) {
				if (scanBytes[i + j] != d[j] && d[j] != -1) {
					found = false;
					break;
				}
			}
			if (found) {
				return &scanBytes[i];
			}
		}
		return nullptr;
	};

	g_Modules[fnv::hash(hs::serverbrowser_dll::s().c_str())] = hs::serverbrowser_dll::s().c_str();
	g_Modules[fnv::hash(hs::client_dll::s().c_str())] = hs::client_dll::s().c_str();
	g_Modules[fnv::hash(hs::engine_dll::s().c_str())] = hs::engine_dll::s().c_str();
	g_Modules[fnv::hash(hs::vstdlib_dll::s().c_str())] = hs::vstdlib_dll::s().c_str();
	g_Modules[fnv::hash(hs::input_system_dll::s().c_str())] = hs::input_system_dll::s().c_str();
	g_Modules[fnv::hash(hs::server_dll::s().c_str())] = hs::server_dll::s().c_str();
	g_Modules[fnv::hash(hs::vgui_mat_surface_dll::s().c_str())] = hs::vgui_mat_surface_dll::s().c_str();
	g_Modules[fnv::hash(hs::vgui2_dll::s().c_str())] = hs::vgui2_dll::s().c_str();
	g_Modules[fnv::hash(hs::mat_sys_dll::s().c_str())] = hs::mat_sys_dll::s().c_str();
	g_Modules[fnv::hash(hs::studio_render_dll::s().c_str())] = hs::studio_render_dll::s().c_str();
	g_Modules[fnv::hash(hs::physics_dll::s().c_str())] = hs::physics_dll::s().c_str();
	g_Modules[fnv::hash(hs::data_cache_dll::s().c_str())] = hs::data_cache_dll::s().c_str();
	g_Modules[fnv::hash(hs::tier0_dll::s().c_str())] = hs::tier0_dll::s().c_str();
	g_Modules[fnv::hash(hs::gameoverlayrenderer_dll::s().c_str())] = hs::gameoverlayrenderer_dll::s().c_str();
	//g_Modules[fnv::hash(hs::d3dx9fix::s().c_str())] = hs::d3dx9fix::s().c_str();

	g_Patterns[fnv::hash(hs::client_state::s().c_str())] = hs::client_state_p::s().c_str();
	g_Patterns[fnv::hash(hs::move_helper::s().c_str())] = hs::move_helper_p::s().c_str();
#ifdef _DEBUG
	g_Patterns[fnv::hash(hs::reset::s().c_str())] = hs::reset_p::s().c_str();
#endif
	g_Patterns[fnv::hash(hs::input::s().c_str())] = hs::input_p::s().c_str();
	g_Patterns[fnv::hash(hs::glow_manager::s().c_str())] = hs::glow_manager_p::s().c_str();
	g_Patterns[fnv::hash(hs::beams::s().c_str())] = hs::beams_p::s().c_str();
	g_Patterns[fnv::hash(hs::update_clientside_anim::s().c_str())] = hs::update_clientside_anim_p::s().c_str();
	g_Patterns[fnv::hash(hs::debp::s().c_str())] = hs::debp_p::s().c_str();
	g_Patterns[fnv::hash(hs::standard_blending_rules::s().c_str())] = hs::standard_blending_rules_p::s().c_str();
	g_Patterns[fnv::hash(hs::should_skip_anim_frame::s().c_str())] = hs::should_skip_anim_frame_p::s().c_str();
	g_Patterns[fnv::hash(hs::get_foreign_fall_back_name::s().c_str())] = hs::get_foreign_fall_back_name_p::s().c_str();
	g_Patterns[fnv::hash(hs::setup_bones::s().c_str())] = hs::setup_bones_p::s().c_str();
	g_Patterns[fnv::hash(hs::build_transformations::s().c_str())] = hs::build_transformations_p::s().c_str();
	g_Patterns[fnv::hash(hs::effects::s().c_str())] = hs::effects_p::s().c_str();
	//g_Patterns[fnv::hash(hs::d3dx9fix_pattern::s().c_str())] = hs::d3dx9fix_pattern::s().c_str();


	static auto test = g_Patterns[fnv::hash(hs::effects::s().c_str())];
	test = test;

	interfaces.client = get_interface<IBaseClientDll>("client.dll", ("VClient0"));

	interfaces.ent_list = Interface< CEntityList >((
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::ent_list::s().c_str());

	interfaces.leaf_system = Interface< IClientLeafSystem >((
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::leaf_sys::s().c_str());

	interfaces.cvars = Interface< ICVar >(
		g_Modules[fnv::hash(hs::vstdlib_dll::s().c_str())].c_str(),
		hs::cvar::s().c_str());

	interfaces.engine = Interface< IEngineClient >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::engine_client::s().c_str());

	interfaces.trace = Interface< IEngineTrace >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::engine_trace::s().c_str());

	interfaces.inputsystem = Interface< IInputSystem >(
		g_Modules[fnv::hash(hs::input_system_dll::s().c_str())].c_str(),
		hs::input_sys::s().c_str());

	interfaces.surfaces = Interface< ISurface >(
		g_Modules[fnv::hash(hs::vgui_mat_surface_dll::s().c_str())].c_str(),
		hs::vgui_surface::s().c_str());

	interfaces.server = GetInterface<IServerGameDLL>(
		g_Modules[fnv::hash(hs::server_dll::s().c_str())].c_str(),
		hs::server_game_dll::s().c_str());

	interfaces.v_panel = Interface< IVPanel >(
		g_Modules[fnv::hash(hs::vgui2_dll::s().c_str())].c_str(),
		hs::vgui_panel::s().c_str());

	interfaces.models.model_render = Interface< IVModelRender >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::engine_model::s().c_str());

	interfaces.material_system = Interface< IMaterialSystem >(
		g_Modules[fnv::hash(hs::mat_sys_dll::s().c_str())].c_str(),
		hs::mat_sys::s().c_str());

	interfaces.models.model_info = Interface< IVModelInfo >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::model_info::s().c_str());

	interfaces.render_view = Interface< IVRenderView >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::render_view::s().c_str());

	interfaces.debug_overlay = Interface< IVDebugOverlay >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::debug_overlay::s().c_str());

	interfaces.event_manager = Interface< IGameEventManager2 >(((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str())),
		hs::events_manager::s().c_str());

	interfaces.studio_render = Interface< IStudioRender >(
		g_Modules[fnv::hash(hs::studio_render_dll::s().c_str())].c_str(),
		hs::studio_render::s().c_str());

	interfaces.phys_props = Interface< IPhysicsSurfaceProps >(
		g_Modules[fnv::hash(hs::physics_dll::s().c_str())].c_str(),
		hs::phys_props::s().c_str());

	interfaces.game_movement = Interface< CGameMovement >((
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::game_movement::s().c_str());

	interfaces.model_cache = Interface< IMDLCache >(
		g_Modules[fnv::hash(hs::data_cache_dll::s().c_str())].c_str(),
		hs::mdl_cache::s().c_str());

	interfaces.engine_vgui = Interface<IEngineVGui>((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
		hs::engine_vgui::s().c_str());

	interfaces.engine_sound = Interface< IEngineSound >((
		g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
		hs::engine_sound::s().c_str());

	interfaces.move_helper = **reinterpret_cast<IMoveHelper***>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		g_Patterns[fnv::hash(hs::move_helper::s().c_str())].c_str()) + 2);

	interfaces.prediction = Interface< CPrediction >((
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::client_prediction::s().c_str());

	interfaces.input = *reinterpret_cast<CInput**>(csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 1);

	interfaces.glow_manager = *reinterpret_cast<CGlowObjectManager**>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		g_Patterns[fnv::hash(hs::glow_manager::s().c_str())].c_str()) + 3);

	interfaces.beams = *reinterpret_cast<IViewRenderBeams**>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		g_Patterns[fnv::hash(hs::beams::s().c_str())].c_str()) + 1);

	interfaces.memalloc = *reinterpret_cast<IMemAlloc**>(GetProcAddress(GetModuleHandleA("tier0.dll"), "g_pMemAlloc"));

	interfaces.effects = Interface<CEffects>("engine.dll", "VEngineEffects001");

	interfaces.global_vars = **reinterpret_cast<global_vars_t***>((*(DWORD**)interfaces.client)[0] + 0x1F);
	interfaces.client_mode = **reinterpret_cast<IClientMode***>((*reinterpret_cast<uintptr_t**>(interfaces.client))[10] + 0x5);

	interfaces.net_string_container = Interface< CNetworkStringTableContainer >("engine.dll","VEngineClientStringTable001");

	CSignatures::Get().Initialize();
}

float F::KillDelayTime = NULL;
LineGoesThroughSmokeFn F::LineToSmoke = NULL;

bool F::Shooting()
{
	auto btime = []()
	{
		if (!csgo->weapon)
			return false;
		float flNextPrimaryAttack = csgo->weapon->NextPrimaryAttack();

		return flNextPrimaryAttack <= (float)csgo->local->GetTickBase() * interfaces.global_vars->interval_per_tick;
	};

	if (!csgo->weapon)
		return false;
	if (csgo->weapon->IsBomb())
		return false;
	else if (csgo->weapon->IsNade())
	{
		CBaseCSGrenade* csGrenade = (CBaseCSGrenade*)csgo->weapon;
		if (csGrenade->GetThrowTime() > 0.f)
		{
			return true;
		}
	}
	else if (csgo->cmd->buttons & IN_ATTACK && btime())
	{
		return true;
	}
	return false;
}

namespace VMT
{
	std::unique_ptr< VMTHook > g_pDirectXHook = nullptr;
	std::unique_ptr< VMTHook > g_pClientStateAdd = nullptr;
	std::unique_ptr< VMTHook > g_pPanelHook = nullptr;
	std::unique_ptr< VMTHook > g_pClientModeHook = nullptr;
	std::unique_ptr< VMTHook > g_pClientHook = nullptr;
	std::unique_ptr< VMTHook > g_pRenderViewHook = nullptr;
	std::unique_ptr< VMTHook > g_pStudioRenderHook = nullptr;
	//std::unique_ptr< VMTHook > g_pFireEventHook = nullptr;
	std::unique_ptr< VMTHook > g_pModelRenderHook = nullptr;
	std::unique_ptr< VMTHook > g_pPredictHook = nullptr;
	std::unique_ptr< VMTHook > g_pQueryHook = nullptr;
	std::unique_ptr< VMTHook > g_pGetBoolHook = nullptr;
	std::unique_ptr< VMTHook > g_pCameraHook = nullptr;
	std::unique_ptr< VMTHook > g_pEngineHook = nullptr;
	//std::unique_ptr< VMTHook > g_pPlayerHook = nullptr;
	std::unique_ptr< VMTHook > g_pLeafSystemHook = nullptr;
	std::unique_ptr< VMTHook > g_pVGUIHook = nullptr;
	std::unique_ptr< VMTHook > g_pTEFireBullets = nullptr;
	std::unique_ptr< VMTHook > g_pNetShowFragments = nullptr;
	std::unique_ptr< VMTHook > g_pShadow = nullptr;
	std::unique_ptr< VMTHook > g_pD3DX9 = nullptr;
	std::unique_ptr< VMTHook > g_SteamAPI = nullptr;
}
std::uint8_t* pattern_scan(const char* module_name, const char* signature) noexcept {
	const auto module_handle = GetModuleHandleA(module_name);

	if (!module_handle)
		return nullptr;

	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + std::strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;

				if (*current == '?')
					++current;

				bytes.push_back(-1);
			}
			else {
				bytes.push_back(std::strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
	auto nt_headers =
		reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(module_handle) + dos_header->e_lfanew);

	auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
	auto pattern_bytes = pattern_to_byte(signature);
	auto scan_bytes = reinterpret_cast<std::uint8_t*>(module_handle);

	auto s = pattern_bytes.size();
	auto d = pattern_bytes.data();

	std::stringstream write;

	for (auto i = 0ul; i < size_of_image - s; ++i) {
		bool found = true;

		for (auto j = 0ul; j < s; ++j) {
			if (scan_bytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found)
		{
			
			return &scan_bytes[i];
		}

	}
	
	return nullptr;
}

/*Vector* __fastcall get_eye_ang::hook(void* ecx, void* edx) {
	auto pl = (IBasePlayer*)ecx;
	//static Vector* o_fn;
	if (!csgo->cmd || !pl || !csgo->local || pl != csgo->local)
		return H::GetEyeAng(ecx, edx);

	static auto ret_to_thirdperson_pitch = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?");
	static auto ret_to_thirdperson_yaw = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "F3 0F 10 55 ? 51 8B 8E ? ? ? ?");

	if (_ReturnAddress() == (void*)ret_to_thirdperson_pitch || _ReturnAddress() == (void*)ret_to_thirdperson_yaw)
		return &csgo->cmd->viewangles;

	return H::GetEyeAng(ecx, edx);
}	*/







__declspec(naked) void __stdcall hooked_createmove_naked(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		push ebx
		push esp
		push dword ptr[esp + 20]
		push dword ptr[esp + 0Ch + 8]
		push dword ptr[esp + 10h + 4]
		call hooks_new_hooked_createmove
		pop ebx
		retn 0Ch
	}
}




void H::Hook()
{
	for (int i = 0; i < 256; i++) {
		csgo->key_pressed[i] = false;
		csgo->key_down[i] = false;
		csgo->key_pressedticks[i] = 0;
	}

	features->Visuals = new CVisuals();
	features->Bunnyhop = new CBunnyhop();
	features->BulletTracer = new CBulletTracer();
	features->Eventlog = new CEventlog();

	ClassId = new ClassIdManager();

	csgo->client_state = **reinterpret_cast<CClientState***>(
		csgo->Utils.FindPatternIDA(
			GetModuleHandleA(g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
			"A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);

	DWORD* dwUpdateClientSideAnimation = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		"8B 0D ? ? ? ? 53 56 57 8B 99 ? ? ? ? 85 DB 74 1C"));

	/*
	DWORD* dwDoExtraBoneProcessing = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		g_Patterns[fnv::hash(hs::debp::s().c_str())].c_str()));*/

	DWORD* dwStandardBlendingRules = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		"55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"));

	DWORD* dwShouldSkipAnimFrame = (DWORD*)(csgo->Utils.FindPatternIDA( GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()), "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"));


	DWORD* dwGetForeignFallbackFontName = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::vgui_mat_surface_dll::s().c_str())].c_str()),
		"80 3D ? ? ? ? ? 74 06 B8"));

	DWORD* dwSetupBones = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		"55 8B EC 83 E4 F0 B8 D8"));

	DWORD* dwBuildTransform = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		"55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 1C"));

	Effects = **reinterpret_cast<CClientEffectRegistration***>(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		g_Patterns[fnv::hash(hs::effects::s().c_str())].c_str()) + 2);

	typedef uint32_t SteamPipeHandle;
	typedef uint32_t SteamUserHandle;
	SteamUserHandle hSteamUser = ((SteamUserHandle(__cdecl*)(void))GetProcAddress(GetModuleHandle("steam_api.dll"), "SteamAPI_GetHSteamUser"))();
	SteamPipeHandle hSteamPipe = ((SteamPipeHandle(__cdecl*)(void))GetProcAddress(GetModuleHandle("steam_api.dll"), "SteamAPI_GetHSteamPipe"))();
	SteamClient = ((ISteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandle("steam_api.dll"), "SteamClient"))();
	SteamGameCoordinator = (ISteamGameCoordinator*)SteamClient->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamGameCoordinator001");
	SteamUser = (ISteamUser*)SteamClient->GetISteamUser(hSteamUser, hSteamPipe, "SteamUser019");
	SteamFriends = SteamClient->GetISteamFriends(hSteamUser, hSteamPipe, "SteamFriends015");
	SteamUtils = SteamClient->GetISteamUtils(hSteamPipe, "SteamUtils009");

	auto sv_cheats = interfaces.cvars->FindVar(hs::sv_cheats::s().c_str());
	auto camera = interfaces.cvars->FindVar(hs::camera::s().c_str());
	auto shadows = interfaces.cvars->FindVar(hs::shadows::s().c_str());

	g_pClientStateAdd = std::make_unique< VMTHook >();
	g_pEngineHook = std::make_unique< VMTHook >();
	g_pPanelHook = std::make_unique< VMTHook >();
	g_pClientModeHook = std::make_unique< VMTHook >();
	g_pShadow = std::make_unique< VMTHook >();
	g_pStudioRenderHook = std::make_unique< VMTHook >();
	//g_pFireEventHook = std::make_unique< VMTHook >();
	g_pModelRenderHook = std::make_unique< VMTHook >();
	g_pPredictHook = std::make_unique< VMTHook >();
	g_pQueryHook = std::make_unique< VMTHook >();
	g_pCameraHook = std::make_unique< VMTHook >();
	g_pGetBoolHook = std::make_unique< VMTHook >();
	g_pDirectXHook = std::make_unique< VMTHook >();
	g_pRenderViewHook = std::make_unique< VMTHook >();
	g_pLeafSystemHook = std::make_unique< VMTHook >();
	g_pVGUIHook = std::make_unique< VMTHook >();
	g_pClientHook = std::make_unique< VMTHook >();
	g_SteamAPI = std::make_unique< VMTHook >();
	
	
	g_pEngineHook->Setup(interfaces.engine);
	g_pPanelHook->Setup(interfaces.v_panel);
	g_pClientHook->Setup(interfaces.client);
	g_pClientModeHook->Setup(interfaces.client_mode);
	g_pShadow->Setup(shadows);
	g_pStudioRenderHook->Setup(interfaces.studio_render);
	g_pModelRenderHook->Setup(interfaces.models.model_render);
	g_pPredictHook->Setup(interfaces.prediction);
	g_pQueryHook->Setup(interfaces.engine->GetBSPTreeQuery());
	g_pCameraHook->Setup(camera);
	g_pGetBoolHook->Setup(sv_cheats);
	g_pRenderViewHook->Setup(interfaces.render_view);
	g_pLeafSystemHook->Setup(interfaces.leaf_system);
	g_pVGUIHook->Setup(interfaces.engine_vgui);
	g_pClientStateAdd->Setup((CClientState*)(uint32_t(csgo->client_state) + 0x8));
	g_SteamAPI->Setup(SteamGameCoordinator);

	for (auto e = Effects; e; e = e->next)
	{
		if (strstr(e->effectName, hs::impact::s().c_str()) && strlen(e->effectName) <= 8) {
			ImpactCallback = e->function;
			e->function = &Hooked_ImpactCallback;
			break;
		}
	}
	


	//auto get_eye_ang_target = (void*)csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "56 8B F1 85 F6 74 32");

	
	
	g_pClientHook->Hook(37, Hooked_FrameStageNotify);// fixed
	g_pClientHook->Hook(22, hooked_createmove_naked);// fixed
	
	


	g_pEngineHook->Hook(32, Hooked_IsBoxVisible); // ? just dont work hook lol
	g_pEngineHook->Hook(93, Hooked_IsHLTV);
	g_pEngineHook->Hook(101, Hooked_GetScreenAspectRatio);
	g_pShadow->Hook(13, Hooked_ShouldDrawShadow);
	g_pClientModeHook->Hook(24, Hooked_CreateMove);
	//g_pEngineHook->Hook(59, Hooked_FireEvent);
	g_pClientModeHook->Hook(17, Hooked_ShouldDrawFog);
	g_pClientModeHook->Hook(18, Hooked_OverrideView);
	g_pClientModeHook->Hook(35, Hooked_ViewModel);
	g_pStudioRenderHook->Hook(29, Hooked_DrawModel);
	g_pClientModeHook->Hook(44, Hooked_DoPostScreenEffects);
	g_pPanelHook->Hook(41, Hooked_PaintTraverse);
	g_pStudioRenderHook->Hook(9, Hooked_BeginFrame);
	//g_pModelRenderHook->Hook(Index::DrawModelExecute, Hooked_DrawModelExecute);
	g_pPredictHook->Hook(19, Hooked_RunCommand);
	g_pPredictHook->Hook(14, Hooked_InPrediction);
	g_pQueryHook->Hook(6, Hooked_ListLeavesInBox);
	g_pCameraHook->Hook(Index::InCamera, get_bool);
	g_pGetBoolHook->Hook(13, Hooked_GetBool);
	g_pRenderViewHook->Hook(9, Hooked_SceneEnd);
//	g_pClientStateAdd->Hook(5, Hooked_PacketStart);
//	g_pClientStateAdd->Hook(6, Hooked_PacketEnd);
	//MH_Initialize();
	//do our hooks
	//MH_CreateHook(get_eye_ang_target, &get_eye_ang::hook, (void**)&H::GetEyeAng);
	//MH_CreateHook(setup_bones_target, Hooks::SetupBones, (void**)&o_setup_bones);

	//MH_EnableHook(nullptr);

	H::UpdateClientSideAnimation = (UpdateClientSideAnimationFn)DetourFunction((PBYTE)dwUpdateClientSideAnimation, (PBYTE)Hooked_UpdateClientSideAnimation);
	//H::DoExtraBoneProcessing = (DoExtraBoneProcessingFn)DetourFunction((PBYTE)dwDoExtraBoneProcessing, (PBYTE)Hooked_DoExtraBoneProcessing);
	H::ShouldSkipAnimFrame = (ShouldSkipAnimFrameFn)DetourFunction((PBYTE)dwShouldSkipAnimFrame, (PBYTE)Hooked_ShouldSkipAnimFrame);
	//H::BuildTransformations = (BuildTransformationsFn)DetourFunction((PBYTE)dwBuildTransform, (PBYTE)Hooked_BuildTransformations);
	//H::SetupBones = (SetupBonesFn)DetourFunction((PBYTE)dwSetupBones, (PBYTE)Hooked_SetupBones);
	H::StandardBlendingRules = (StandardBlendingRulesFn)DetourFunction((PBYTE)dwStandardBlendingRules, (PBYTE)Hooked_StandardBlendingRules);
	H::GetForeignFallbackFontName = (GetForeignFallbackFontNameFn)DetourFunction((PBYTE)dwGetForeignFallbackFontName, (PBYTE)Hooked_GetForeignFallbackFontName);
	

	p_CL_Move = (uintptr_t)(csgo->Utils.FindPatternIDA(GetModuleHandleA("engine.dll"), "55 8B EC 81 EC ? ? ? ? 53 56 8A F9 F3 0F 11 45 ? 8B"));
	if (p_CL_Move)
		ClMove = (decltype(&CL_Move))DetourFunction(reinterpret_cast<BYTE*>(p_CL_Move), reinterpret_cast<BYTE*>(CL_Move));

	

	DWORD* dwCLSendMove = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA("engine.dll"),
		"55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98"));
	//if(dwCLSendMove) 
	//	H::oCLSendMove = (decltype(&CL_SendMove))DetourFunction(reinterpret_cast<BYTE*>(dwCLSendMove), reinterpret_cast<BYTE*>(CL_SendMove));

	DWORD* EyeAngles = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA("client.dll"),
		"56 8B F1 85 F6 74 32"));


	H::GetEyeAnglesO = (GetEyeAnglesFn)DetourFunction((PBYTE)EyeAngles, (PBYTE)hkGetEyeAngles);
	g_SteamAPI->Hook(0, Hooked_SendSteamMsg);
	g_SteamAPI->Hook(2, Hooked_RetrieveMsg);

	//H::SteamPresent = (steamoverlay_present)DetourFunction((PBYTE)present_pattern, (PBYTE)Hooked_Present);
	//H::SteamReset = (steamoverlay_reset)DetourFunction((PBYTE)reset_pattern, (PBYTE)Hooked_Reset);

	SteamPresent = *reinterpret_cast<decltype(SteamPresent)*>(CSignatures::Get().Present);
	*reinterpret_cast<decltype(::Hooked_Present)**>(CSignatures::Get().Present) = ::Hooked_Present;

	SteamReset = *reinterpret_cast<decltype(SteamReset)*>(CSignatures::Get().Reset);
	*reinterpret_cast<decltype(::Hooked_Reset)**>(CSignatures::Get().Reset) = ::Hooked_Reset;


	
	//g_Modules[fnv::hash(hs::vgui_mat_surface_dll::s().c_str())].c_str()),
	//g_Patterns[fnv::hash(hs::get_foreign_fall_back_name::s().c_str())].c_str())

	//static auto d3dx9 = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::d3dx9fix::s().c_str())].c_str()), g_Patterns[fnv::hash(hs::d3dx9fix_pattern::s().c_str())].c_str()) + 1;
	

	//static auto d3dx9 = **reinterpret_cast<uintptr_t**>(pattern_scan(("shaderapidx9.dll"), "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
	//g_pD3DX9 = std::make_unique<VMTHook>();
	//g_pD3DX9->Setup((VOID*)d3dx9);
	//g_pD3DX9->Hook(17, Hooked_Present);
	//g_pD3DX9->Hook(16, Hooked_Reset);
	





	if (csgo->Init.Window)
		csgo->Init.OldWindow = (WNDPROC)SetWindowLongPtr(csgo->Init.Window, GWL_WNDPROC, (LONG_PTR)Hooked_WndProc);

	RegListeners();

	/*g_Patterns.clear();
	g_Modules.clear();*/

	//F::LineToSmoke = (LineGoesThroughSmokeFn)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())]), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");

	const char* fart[]{ "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long amongus = 0x69690004C201B0;
	for (auto sex : fart) WriteProcessMemory(GetCurrentProcess(),
		(LPVOID)csgo->Utils.FindPatternIDA(GetModuleHandleA(sex), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &amongus, 7, 0);

	

}

void H::UnHook()
{
	if (interfaces.inputsystem)
		interfaces.inputsystem->EnableInput(true);

	if (g_Animfix->FakeAnimstate)
		interfaces.memalloc->Free(g_Animfix->FakeAnimstate);

	//DetourRemove((PBYTE)H::DoExtraBoneProcessing, (PBYTE)Hooked_DoExtraBoneProcessing);
	DetourRemove((PBYTE)H::UpdateClientSideAnimation, (PBYTE)Hooked_UpdateClientSideAnimation);
	DetourRemove((PBYTE)H::StandardBlendingRules, (PBYTE)Hooked_StandardBlendingRules);
	//DetourRemove((PBYTE)H::BuildTransformations, (PBYTE)Hooked_BuildTransformations);
	DetourRemove((PBYTE)H::ShouldSkipAnimFrame, (PBYTE)Hooked_ShouldSkipAnimFrame);
	//DetourRemove((PBYTE)H::SetupBones, (PBYTE)Hooked_SetupBones);
	DetourRemove((PBYTE)H::GetForeignFallbackFontName, (PBYTE)Hooked_GetForeignFallbackFontName);
	//DetourRemove((PBYTE)H::SteamPresent, (PBYTE)Hooked_Present);
	//DetourRemove((PBYTE)H::SteamReset, (PBYTE)Hooked_Reset);



	*reinterpret_cast<void**>(CSignatures::Get().Present) = SteamPresent;
	*reinterpret_cast<void**>(CSignatures::Get().Reset) = SteamReset;

	g_pVGUIHook->Unhook(14);
	g_pClientHook->Unhook(37);
	//g_pClientHook->Unhook(22);
	g_pClientHook->Unhook(24);
	//g_pDirectXHook->Unhook(Index::Present);
	//g_pDirectXHook->Unhook(Index::Reset);
	g_pPanelHook->Unhook(Index::PaintTraverse);
//	g_pEngineHook->Unhook(59);
	g_pEngineHook->Unhook(Index::IsBoxVisible);
	g_pEngineHook->Unhook(Index::IsHLTV);
	g_pEngineHook->Unhook(101);
	g_pShadow->Unhook(13);
	g_pClientModeHook->Unhook(35);
	g_pClientModeHook->Unhook(17);
	g_pClientModeHook->Unhook(Index::CreateMove);
	g_pClientModeHook->Unhook(Index::OverrideView);
	g_pClientModeHook->Unhook(Index::DoPostScreenEffects);
	g_pStudioRenderHook->Unhook(Index::BeginFrame);
	g_pStudioRenderHook->Unhook(29);
	g_pModelRenderHook->Unhook(Index::DrawModelExecute);
	//g_pPredictHook->Unhook(20);
	g_pPredictHook->Unhook(19);
	g_pPredictHook->Unhook(Index::InPrediction);
	g_pRenderViewHook->Unhook(Index::SceneEnd);
	g_pQueryHook->Unhook(Index::ListLeavesInBox);
	g_pCameraHook->Unhook(Index::InCamera);
	g_pGetBoolHook->Unhook(Index::GetBool);

	for (auto listener : g_pGameEventManager)
		interfaces.event_manager->RemoveListener(listener);

	for (auto head = Effects; head; head = head->next)
	{
		if (strstr(head->effectName, hs::impact::s().c_str()) && strlen(head->effectName) <= 8) {
			head->function = H::ImpactCallback;
			break;
		}
	}
}