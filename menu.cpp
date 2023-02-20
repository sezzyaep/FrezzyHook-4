#include "Menu.h"
#include "GUI/gui.h"
#include "Skinchanger.h"
#include <thread>
#include "steam_sdk/isteamfriends.h"
#include "steam_sdk/steam_api.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#define FCVAR_HIDDEN			(1<<4)	// Hidden. Doesn't appear in find or 
#define FCVAR_UNREGISTERED		(1<<0)	// If this is set, don't add to linked list, etc.
#define FCVAR_DEVELOPMENTONLY	(1<<1)	// Hidden in released products. Flag is removed 

vector<string> ConfigList;
vector<string> ScriptsList;
typedef void(*LPSEARCHFUNC)(LPCTSTR lpszFileName);
static const char* player_model_index_all[] =
{
	"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
	"models/player/custom_player/legacy/tm_leet_varianth.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
	"models/player/custom_player/legacy/tm_leet_varianti.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_variantf.mdl",
	"models/player/custom_player/legacy/ctm_st6_variante.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",//14
	"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantb.mdl"
};
std::array < Vector4D, 6 > aWhiteArray =
{
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
};

std::array < float_t, 3 > aColorModulation =
{
	1.0f,
	1.0f,
	1.0f
};

class CNStudioRender
{
private:
	template <typename T, typename ... args_t>
	constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
	{
		using VirtualFnxz = T(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<VirtualFnxz**>(thisptr))[nIndex](thisptr, argList...);
	}
public:
	void SetAmbientLightColors(const Vector4D* pAmbientOnlyColors)
	{
		CallVFunc<void>(this, 20, pAmbientOnlyColors);
	}
	void SetAmbientLightColors(const Vector* pAmbientOnlyColors)
	{
		CallVFunc<void>(this, 21, pAmbientOnlyColors);
	}
	void SetLocalLights(int nLights, const LightDesc_t* pLights)
	{
		CallVFunc<void>(this, 22, nLights, pLights);
	}

	void SetColorModulation(float const* arrColor)
	{
		CallVFunc<void>(this, 27, arrColor);
	}

	void SetAlphaModulation(float flAlpha)
	{
		CallVFunc<void>(this, 28, flAlpha);
	}

	void ForcedMaterialOverride(IMaterial* pMaterial, OverrideType_t nOverrideType = 0, int nOverrides = 0)
	{
		CallVFunc<void>(this, 33, pMaterial, nOverrideType, nOverrides);
	}
};
CNStudioRender* m_StudioRender = new CNStudioRender();

void CDrawModel::Run()
{
	if (!vars.menu.open)
		return;

	if (!m_PreviewTexture)
	{

		interfaces.material_system->BeginRenderTargetAllocation();

		m_PreviewTexture = interfaces.material_system->CreateNamedRenderTargetTextureEx(
			("Preview"),
			350, 575,
			RT_SIZE_NO_CHANGE,
			interfaces.material_system->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SHARED,
			4 | 8,
			0x00000001
		);

		interfaces.material_system->FinishRenderTargetAllocation();
	}

	if (!m_CubemapTexture)
		m_CubemapTexture = interfaces.material_system->FindTexture(("editor/cubemap.hdr"), ("CubeMap textures"));
	static auto m_CreateModel = csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08");
	auto CreateModel = reinterpret_cast<void(__thiscall*)(void*)>(m_CreateModel);
	if (!m_PreviewModel)
	{
		m_PreviewModel = static_cast<CMergedMDL*>(interfaces.memalloc->Alloc(0x75C));
		CreateModel(m_PreviewModel);
		m_PreviewModel->SetMDL(("models/player/custom_player/uiplayer/animset_uiplayer.mdl"));
		m_PreviewModel->SetMergedMDL((player_model_index_all[14]));
		m_PreviewModel->SetMergedMDL(("models/weapons/w_rif_ak47.mdl"));
		m_PreviewModel->SetSequence(32, false);
		m_PreviewModel->SetupBonesForAttachmentQueries();
	}

	m_PreviewModel->RootMDL.flTime += interfaces.global_vars->frametime / 2.0f;

	m_ViewSetup.x = 0;
	m_ViewSetup.y = 0;
	m_ViewSetup.width = 350;
	m_ViewSetup.height = 575;
	m_ViewSetup.m_bOrtho = false;
	m_ViewSetup.fov = 70.f;
	m_ViewSetup.origin = Vector(-65.0f, 2.0f, 50);
	m_ViewSetup.angles = Vector(0, 0, 0);
	m_ViewSetup.flNearZ = 7.0f;
	m_ViewSetup.flFarZ = 1000.f;
	m_ViewSetup.bDoBloomAndToneMapping = true;

	CMatRenderContextPtr pRenderContext(interfaces.material_system);

	pRenderContext->PushRenderTargetAndViewport();
	pRenderContext->SetRenderTarget(m_PreviewTexture);

	pRenderContext->BindLocalCubemap(m_CubemapTexture);
	pRenderContext->SetLightingOrigin(-65.0f, 2.0f, 50.0f);

	Frustum_t dummyFrustum;
	enum ahdf
	{
		VIEW_CLEAR_COLOR = 0x1,
		VIEW_CLEAR_DEPTH = 0x2,
		VIEW_CLEAR_FULL_TARGET = 0x4,
		VIEW_NO_DRAW = 0x8,
		VIEW_CLEAR_OBEY_STENCIL = 0x10,
		VIEW_CLEAR_STENCIL = 0x20,
	};
	interfaces.render_view->Push3DView(pRenderContext, m_ViewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL, m_PreviewTexture, dummyFrustum);

	pRenderContext->ClearColor4ub(false, false, false, false);
	pRenderContext->ClearBuffers(true, true, true);
	pRenderContext->SetAmbientLightCube(aWhiteArray.data());

	m_StudioRender->SetAmbientLightColors(aWhiteArray.data());
	m_StudioRender->SetLocalLights(0, nullptr);

	matrix matPlayerView = { };
	Math::AngleMatrix(Vector(0, Rotation, 0), Vector(0, 0, 0), matPlayerView);

	
	interfaces.models.model_render->SuppressEngineLighting(true);


	m_PreviewModel->Draw(matPlayerView);



	interfaces.models.model_render->SuppressEngineLighting(false);

	interfaces.render_view->PopView(pRenderContext, dummyFrustum);
	pRenderContext->BindLocalCubemap(nullptr);

	pRenderContext->PopRenderTargetAndViewport();
	pRenderContext->Release();
}
BOOL SearchFiles(LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders = FALSE)
{
	LPTSTR part;
	char tmp[MAX_PATH];
	char name[MAX_PATH];

	HANDLE hSearch = NULL;
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(WIN32_FIND_DATA));

	if (bInnerFolders)
	{
		if (GetFullPathName(lpszFileName, MAX_PATH, tmp, &part) == 0) return FALSE;
		strcpy(name, part);
		strcpy(part, "*.*");
		wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		if (!((hSearch = FindFirstFile(tmp, &wfd)) == INVALID_HANDLE_VALUE))
			do
			{
				if (!strncmp(wfd.cFileName, ".", 1) || !strncmp(wfd.cFileName, "..", 2))
					continue;

				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char next[MAX_PATH];
					if (GetFullPathName(lpszFileName, MAX_PATH, next, &part) == 0) return FALSE;
					strcpy(part, wfd.cFileName);
					strcat(next, "\\");
					strcat(next, name);

					SearchFiles(next, lpSearchFunc, TRUE);
				}
			} while (FindNextFile(hSearch, &wfd));
			FindClose(hSearch);
	}

	if ((hSearch = FindFirstFile(lpszFileName, &wfd)) == INVALID_HANDLE_VALUE)
		return TRUE;
	do
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file[MAX_PATH];
			if (GetFullPathName(lpszFileName, MAX_PATH, file, &part) == 0) return FALSE;
			strcpy(part, wfd.cFileName);

			lpSearchFunc(wfd.cFileName);
		}
	while (FindNextFile(hSearch, &wfd));
	FindClose(hSearch);
	return TRUE;
}
void ReadConfigs(LPCTSTR lpszFileName)
{
	ConfigList.push_back(lpszFileName);
}

void RefreshConfigs()
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
	{
		ConfigList.clear();
		string ConfigDir = std::string(path) + "\\.FrezzyHook\\*.json";
		SearchFiles(ConfigDir.c_str(), ReadConfigs, FALSE);
	}

}

bool Rename(const char* NName) noexcept
{


	interfaces.cvars->FindVar("name")->SetValue(NName);
	return(true);
}

void EnableHiddenCVars()
{
	
	auto p = **reinterpret_cast<ConCommandBase***>(interfaces.cvars + 0x34);
	for (auto c = p->m_pNext; c != nullptr; c = c->m_pNext)
	{
		ConCommandBase* cmd = c;
		cmd->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
		cmd->m_nFlags &= ~FCVAR_HIDDEN;
	}
	PNotify("CS:GO", "Unlocked Cvars", 2);
}

void rename()
{
	Rename(vars.menu.serverIP.c_str());
}

void fakevote()
{
	string outr = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" + vars.menu.fakevote + "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	Rename(outr.c_str());
}

void removename()
{
	Rename("\n\n\n\n\n\n\n\n\n\n");
}

Vector2D g_mouse;
color_t main_color = color_t(255, 0, 0, 255);

bool enable_rage() { return vars.ragebot.enable; };
bool enable_legit() { return vars.legitbot.enable; };
bool enable_antiaim() { return vars.antiaim.enable; };
bool enable_customAA() { return vars.ragebot.customAA; };
bool enable_aa() { return vars.antiaim.preset; };
bool enable_esp() { return vars.visuals.enable; };
bool enable_cusJit() { return vars.antiaim.enable_cusjit; }
bool enable_cusRan() { return vars.antiaim.customRandom; }
bool enable_legacy() { return vars.menu.remove; }
bool enable_D()
{
	if (vars.antiaim.customAAT == 6 && enable_antiaim)
		return true;
	else
		return false;
}

bool enable_base_direction() {
	if (vars.antiaim.baseangle == 0 && (vars.checkbox.yawbase || vars.keybind.yawbase->type > 0))
		return true;
	else
		return false;
}

bool enable_base_static() {
	if (vars.antiaim.baseangle == 1 && (vars.checkbox.yawbase || vars.keybind.yawbase->type > 0))
		return true;
	else
		return false;
}
bool enable_FD() {
	if (vars.antiaim.FD == 3)
		return(true);
	else
		return(false);
}

void c_menu::draw_indicators()
{/*
	constexpr int wnd_size = 300;
	static c_window* wnd = nullptr;
	if (!wnd) {
		wnd = new c_window();
		wnd->set_size(Vector2D(282, wnd_size));
		wnd->set_active_tab_index(-1);
		auto child = new c_child("", -1, wnd);
		child->set_position(Vector2D(0, -8));
		child->set_size(Vector2D(250, wnd_size - 32));
		child->add_element(new c_bar("desync", &csgo->desync_angle, 0.f, 58.f, []() {
			return csgo->desync_angle > 0.f;
		}, []() {
			return csgo->desync_angle == -1.f;
		}));
		child->initialize_elements();
		wnd->add_element(child);
		wnd->set_position(Vector2D(100, csgo->h - 250));
	}
	else {
		wnd->render();
	}
	*/
}
bool need_warning() {
	return (csgo->expire_date - time(0)) / 3600 < 48;
};

LPDIRECT3DTEXTURE9 ProfilePicture;


void LoadStr(string* pointer, const char* name, json_t json) {
	if (json.isMember(name)) *pointer = json[name].asString();
}


void LoadInt(int* pointer, const char* name, json_t json) {
	if (json.isMember(name)) *pointer = json[name].asInt();
}
#define _NOUSER
#define _STEAM
void c_menu::render() {
	if (window) {
		window->update_keystates();
		window->update_animation();
		update_binds();
	}

	static bool once = false;
	if (!once) {
		Config.ResetToDefault(false);
		vars.menu.open = true;
		std::string server_folder;
		std::string Uinfo;
		json_t UinfoT;
		static TCHAR path[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
		{
			server_folder = std::string(path) + ("\\.FrezzyHook\\Server");
			Uinfo = std::string(path) + ("\\.FrezzyHook\\Server\\UserInfo.json");
		}
		std::string ProfilePictureLocation = server_folder + "\\ProfilePicture.png";
		std::string InfiniteLogoLocation = server_folder + "\\.FrezzyHook\\logo.png";
		std::ifstream FIN(Uinfo);
		//string in;
		if (FIN.good())
			FIN >> UinfoT;

		if (!UinfoT.isMember("User")) {
			static int Counter = 0;
#ifndef _NOUSER

			if (csgo->NoUser) {
				if (Counter == 30) {
					std::this_thread::sleep_for(std::chrono::seconds(3));
					PNotify("Critical Alert", "Failed to Initialize Cheat", 5);
				}
				if (Counter >= 600) {
					H::UnHook();
				}
				Counter++;
			}
			if (!csgo->NoUser) {
				
				PNotify("Connection Established", "Connected to FrezzyHook.xyz", 4);
	
				
			
				
				csgo->NoUser = true;
				return;
			}
			
			return;
#else

			std::this_thread::sleep_for(std::chrono::seconds(3));
		//	PNotify("Critical Alert", "No User Detected", 5);
			PNotify("Connection Established", "Connected to FrezzyHook.xyz", 4);
	
#endif	
		}

#ifndef _NOUSER
		auto& UserJson = UinfoT["User"];

		auto& InfoJson = UserJson["Info"];



		std::string UName;
		std::string UnameP;


		LoadStr(&UName, "Username", InfoJson);
		LoadInt(&csgo->PDaysLeft, "DaysLeft", InfoJson);

		if (UName.length() > 14) {
			transform(UName.begin(), UName.end(), UName.begin(), ::tolower);
			for (int i = 0; i < 11; i++) {
				if (UName.at(i) == 'm' && i % 2 == 1) {
					continue;
				}
				UnameP += UName.at(i);
			}
			UnameP += "...";
		}
		else
		{
			UnameP = UName;
		}
		std::this_thread::sleep_for(std::chrono::seconds(3));
		ImGui::PushFont(fonts::WTLG);
		ImVec2 TAIZ = ImGui::CalcTextSize(UName.c_str());
		ImGui::PopFont();
		csgo->FullUsernameLength = TAIZ.x;
		csgo->PUsername = UnameP;
		csgo->FullUsername = UName;
		PNotify("FREZZYHOOK SYSTEM", "Welcome back " + UnameP, 1);
		D3DXCreateTextureFromFileA(csgo->render_device, ProfilePictureLocation.c_str(), &ProfilePicture);
		D3DXCreateTextureFromFileA(csgo->render_device, InfiniteLogoLocation.c_str(), &csgo->Logo);
#else
		std::string UName = SteamFriends->GetPersonaName();
		std::string UnameP;

		if (UName.length() > 14) {
			transform(UName.begin(), UName.end(), UName.begin(), ::tolower);
			for (int i = 0; i < 11; i++) {
				if (UName.at(i) == 'm' && i % 2 == 1) {
					continue;
				}
				UnameP += UName.at(i);
			}
			UnameP += "...";
		}
		else
		{
			UnameP = UName;
		}
		ImGui::PushFont(fonts::WTLG);
		ImVec2 TAIZ = ImGui::CalcTextSize(UName.c_str());
		ImGui::PopFont();
		csgo->FullUsernameLength = TAIZ.x;
		csgo->PUsername = UnameP;
		csgo->FullUsername = UName;
		ProfilePicture = GetImage(SteamUser->GetSteamID());
		D3DXCreateTextureFromFileA(csgo->render_device, InfiniteLogoLocation.c_str(), &csgo->Logo);
		PNotify("FREZZYHOOK SYSTEM", "Welcome back FrezzyHook", 1);
#endif
		vars.visuals.watermark = true;
		once = true;
	}

	if (initialized) {
		if (window->get_transparency() < 100.f && vars.menu.open)
			window->increase_transparency(animation_speed * 160.f);
	}
	else {
		window = new c_window();
		window->set_transparency(0.0f);
		window->set_size(Vector2D(800, 560));
		window->set_position(Vector2D(1920 / 2 - 400, 1080 / 2 - 280));
		window->add_tab(new c_tab("P", tab_t::legit, window));
		{
			auto legitbot_general = new c_child("General", tab_t::legit, window);
			legitbot_general->set_size(Vector2D(350, 470));
			legitbot_general->set_position(Vector2D(17, 36)); {

				legitbot_general->add_element(new c_checkbox("Enable",
					&vars.legitbot.enable));

				legitbot_general->add_element(new c_checkbox("Backward Tracking",
					&vars.legitbot.backtrack, []() { return vars.legitbot.enable; }));

				legitbot_general->add_element(new c_checkbox("Forward Tracking",
					&vars.legitbot.fronttrack, []() { return vars.legitbot.enable; }));

				legitbot_general->add_element(new c_checkbox("Anti-Aim Correction",
					&vars.legitbot.resolver, []() { return vars.legitbot.enable; }));



	

				legitbot_general->initialize_elements();
			}
			window->add_element(legitbot_general);

			reinit_legit_cfg();
		}
		window->add_tab(new c_tab("T", tab_t::rage, window)); {
			auto main_child = new c_child("Main", tab_t::rage, window);
			main_child->set_size(Vector2D(350, 470));
			main_child->set_position(Vector2D(17, 36)); {

				main_child->add_element(new c_checkbox("Enable",
					&vars.ragebot.enable));
				main_child->add_element(new c_checkbox("Rage Backward Tracking",
					&vars.ragebot.posadj, enable_rage));

				main_child->add_element(new c_checkbox("Rage Forward Tracking",
					&vars.ragebot.fronttrack, []() { return vars.ragebot.enable; }));
				main_child->add_element(new c_checkbox("Auto Scope",
					&vars.ragebot.autoscope, enable_rage));

				main_child->add_element(new c_checkbox("Auto Shoot",
					&vars.ragebot.autoshoot, enable_rage));

				main_child->add_element(new c_checkbox("Prefer On Shot", &vars.ragebot.backshoot_bt,
					[]() { return vars.ragebot.enable && vars.ragebot.posadj; }, vars.keybind.onshot));






				main_child->add_element(new c_checkbox("Delay Shot", &vars.ragebot.delayshot,
					enable_rage));

				//main_child->add_element(new c_checkbox("Enable Anti Aim Correction", &vars.ragebot.resolver,
				//	enable_rage));

				main_child->add_element(new c_combo("Anti Aim Correction",
					&vars.ragebot.resolver,
					{
						"Default",
						"Legacy",
						"Disabled"

					}, enable_rage));

				main_child->add_element(new c_checkbox("Legacy resolver", &vars.ragebot.random_resolver,
					enable_legacy));

				main_child->add_element(new c_checkbox("Advanced Hitchance Prediction",
					&vars.ragebot.hitchance_consider_hitbox, enable_rage));


				main_child->add_element(new c_keybind("Force Body Aim", vars.ragebot.force_body, enable_rage));

				main_child->add_element(new c_keybind("Force Safe Point", vars.keybind.safepoint, enable_rage));

				main_child->add_element(new c_colorpicker(&vars.ragebot.shot_clr,
					color_t(255, 255, 255, 255), [] { return vars.ragebot.enable && vars.ragebot.shotrecord; }));

				//main_child->add_element(new c_checkbox("Hitchams",
				//	&vars.ragebot.shotrecord, enable_rage));

				main_child->add_element(new c_checkbox("Visualize Aimbot",
					&vars.ragebot.NOnsilent, enable_rage));


				main_child->add_element(new c_keybind("Overide Minimum Damage",
					vars.ragebot.override_dmg, enable_rage));

				main_child->add_element(new c_checkbox("Anti Exploit",
					&vars.ragebot.FullTP, enable_rage));

				main_child->add_element(new c_keybind("Hideshot", vars.ragebot.hideshots, []() {
					return vars.ragebot.enable;
					}));

				main_child->add_element(new c_combo("Hideshot Mode",
					&vars.ragebot.hideshotmode, {
						"Offensive",
						"Defensive"
					}, []() { return enable_rage() && (vars.ragebot.hideshots->active || vars.ragebot.hideshots->type > 0); }));

				
		

				main_child->add_element(new c_keybind("Rapid Fire", vars.ragebot.double_tap, []() {
					return vars.ragebot.enable;
					}));

				main_child->add_element(new c_combo("Doubletap Mode",
					&vars.ragebot.LagExploitS, {
						"Offensive",
						"Defensive",
						"Lag Peek",
						"Adaptive"
					}, []() { return enable_rage() && (vars.ragebot.double_tap->active || vars.ragebot.double_tap->type > 0); }));


				main_child->add_element(new c_combo("Doubletap Speed",
					&vars.ragebot.BDT,
					{
						"Default",
						"Fast",
						"Instant"
					}, []() { return enable_rage() && (vars.ragebot.double_tap->active || vars.ragebot.double_tap->type > 0); }));


				main_child->add_element(new c_checkbox("Doubletap Prefer Speed",
					&vars.ragebot.double_tap_modes, []() { return enable_rage() && (vars.ragebot.double_tap->active || vars.ragebot.double_tap->type > 0); }));

				main_child->add_element(new c_checkbox("Full Teleport",
					&vars.ragebot.FullTP, enable_rage));


					main_child->add_element(new c_combo("Doubletap Recharge Mode",
						&vars.antiaim.DTT, {
							"Normal",
							"Prefer Recharge",
							"Trippletap"
						}, enable_rage));

				main_child->add_element(new c_combo("Doubletap Movement",
					&vars.ragebot.DoubletapFlags,
					{
						"Default",
						"Teleport"
					}, []() { return enable_rage() && (vars.ragebot.double_tap->active || vars.ragebot.double_tap->type > 0); }));
				/*
				main_child->add_element(new c_combo("Doubletap Quality",
					&vars.antiaim.DTT,
					{
						"Prefer Quality",
						"Prefer Speed"
					}, enable_rage));
					*/

				//main_child->add_element(new c_checkbox("Doubletap Recharge Indicator", &vars.visuals.DTindi, enable_rage));

				main_child->initialize_elements();
			}
			window->add_element(main_child);

			reinit_weapon_cfg();
		}
		window->set_transparency(100.f);
		window->add_tab(new c_tab("Q", tab_t::antiaim, window));
		{
			auto antiaim_main = new c_child("Main", tab_t::antiaim, window);
			antiaim_main->set_size(Vector2D(350, 470));
			antiaim_main->set_position(Vector2D(17, 36)); {

				antiaim_main->add_element(new c_checkbox("Enable", &vars.antiaim.enable));

				antiaim_main->add_element(new c_checkbox("Pitch", &vars.checkbox.pitch, enable_antiaim, vars.keybind.pitch));

				antiaim_main->add_element(new c_combo("Pitch", &vars.antiaim.pitch,
					{
						"Down",
						"Zero",
						"Up",
						"Jitter",
						"Fake",
						"Fake Jitter",
						"None"
					},
					[]() { return enable_antiaim() && (vars.checkbox.pitch || vars.keybind.pitch->type > 0); }));





				antiaim_main->add_element(new c_checkbox("Custom Antiaim", &vars.ragebot.customAA, enable_antiaim));


				antiaim_main->add_element(new c_combo("Yaw Modifier",
					&vars.antiaim.yaw_mod_type, {
						"None",
						"Static",
						"Jitter",
						"Random",
						"Towards Enemies"
					}, enable_customAA));

				antiaim_main->add_element(new c_slider("Yaw Modifier Ammount", &vars.antiaim.yaw_mod_val, 0, 90, "%.0f ",
					enable_customAA));


				antiaim_main->add_element(new c_checkbox("Yaw Base",
					&vars.checkbox.yawbase, enable_antiaim, vars.keybind.yawbase));

				antiaim_main->add_element(new c_combo("Yaw Base",
					&vars.antiaim.baseangle, {
						"Direction",
						"Static",
						"At Targets"
					}, []() { return enable_antiaim() && (vars.checkbox.yawbase || vars.keybind.yawbase->type > 0); }));

				antiaim_main->add_element(new c_combo("Yaw Direction",
					&vars.antiaim.baseangle_direction, {
						"Back",
						"Left",
						"Right"
					}, []() { return enable_antiaim() && enable_base_direction(); }));

				antiaim_main->add_element(new c_slider("Yaw Angle", &vars.antiaim.yaw_angle_start, -90, 90, "%.0f ",
					[]() { return enable_antiaim() && (enable_base_static() || vars.antiaim.baseangle == 2); }));
				antiaim_main->add_element(new c_checkbox("Yaw modifier",
					&vars.checkbox.yawmodifier, enable_antiaim, vars.keybind.yawmodifier));

				antiaim_main->add_element(new c_combo("Yaw modifier",
					&vars.antiaim.yawmodifier, {
						"Jitter",
						"Half Jitter",
						"Directional",
						"Spin",
						"Random"
					}, []() { return enable_antiaim() && (vars.checkbox.yawmodifier || vars.keybind.yawmodifier > 0); }));

				antiaim_main->add_element(new c_slider("Yaw modifier Offset", &vars.antiaim.yawmodifier_value, 0, 90, "%.0f ",
					[]() { return enable_antiaim() && (vars.checkbox.yawmodifier || vars.keybind.yawmodifier->type > 0); }));

				antiaim_main->add_element(new c_combo("Desync Direction",
					&vars.antiaim.customAAT, {
						"None",
						"Static",
						"Jitter",
						"Dynamic"
						//"Dynamic"
					}, enable_antiaim));

				antiaim_main->add_element(new c_combo("Lower Body Target",
					&vars.antiaim.LBYT, {
						"Offset",
						"Extend",
						"Preserve"
					}, enable_antiaim));



				antiaim_main->add_element(new c_slider("Desync Delta", &vars.antiaim.DesyncAMM, 0, 60, "%.0f ", enable_antiaim));

			

				antiaim_main->add_element(new c_checkbox("Fakelag",
					&vars.checkbox.fakelag, enable_antiaim, vars.keybind.fakelag));

				antiaim_main->add_element(new c_slider("Fakelag Min", &vars.antiaim.fakelag_min, 1, 16, "%.0f ",
					[]() { return enable_antiaim() && vars.checkbox.fakelag; }));
				antiaim_main->add_element(new c_slider("Fakelag Max", &vars.antiaim.fakelagfactor, 1, 16, "%.0f ",
					[]() { return enable_antiaim() && vars.checkbox.fakelag; }));
				antiaim_main->add_element(new c_slider("Fakelag Jitter", &vars.antiaim.fakelagjitter, 1, 100, "%.0f ",
					[]() { return enable_antiaim() && vars.checkbox.fakelag; }));

				antiaim_main->add_element(new c_combo("Fakelag On Weapon Fire",
					&vars.antiaim.fakelagonshot, {
						"Disable Fakelag",
						"Enable Fakelag"
					}, []() { return enable_antiaim() && vars.checkbox.fakelag; }));

				antiaim_main->add_element(new c_multicombo("Fakelag Triggers",
					&vars.checkbox.fakelagflags, {
						"Peek", //1
						"In Air", //2
						"Slow-walk",//4
						"Moving",//8
						"Weapon Fire",//16
						"LBY Update"//32
					}, []() { return enable_antiaim() && vars.checkbox.fakelag; }));

				antiaim_main->add_element(new c_slider("Fakelag Trigger Limit", &vars.checkbox.fakelagaa, 1, 16, "%.0f ",
					[]() { return enable_antiaim() && vars.checkbox.fakelag; }));

				antiaim_main->add_element(new c_checkbox("AntiAim Indicator", &vars.antiaim.indicatorr, enable_antiaim));


				antiaim_main->add_element(new c_keybind("Invert Desync Direction", vars.antiaim.inverter,
					[]() { return enable_antiaim(); }));

				antiaim_main->initialize_elements();


			}
			window->add_element(antiaim_main);

			auto antiaim_other = new c_child("Extra", tab_t::antiaim, window);
			antiaim_other->set_size(Vector2D(350, 470));
			antiaim_other->set_position(Vector2D(380, 36));
			{
				/*
				antiaim_other->add_element(new c_checkbox("Freestand",
					&vars.checkbox.freestand, enable_antiaim, vars.keybind.freestand));
				antiaim_other->add_element(new c_combo("Freestand mode",
					&vars.movable.freestand, {
						"Freestand",
						"Anti-Freestand"
						//"Spam"
					}, []() { return enable_antiaim() && (vars.checkbox.freestand || vars.keybind.freestand->type > 0); })); */


				antiaim_other->add_element(new c_checkbox("Hide Anti-Aim",
					&vars.movable.HideAAM, enable_antiaim, vars.movable.HideAA));

				antiaim_other->add_element(new c_checkbox("On Shot Anti-Aim",
					&vars.movable.OnshotAA, enable_antiaim, vars.movable.OnshotAAA));

				antiaim_other->add_element(new c_checkbox("On Peek Anti-Aim",
					&vars.movable.OnPeekAAM, enable_antiaim, vars.movable.OnPeekAA));



				antiaim_other->add_element(new c_checkbox("Edge Anti-Aim",
					&vars.movable.autodir, enable_antiaim, vars.keybind.autodir));

				antiaim_other->add_element(new c_keybind("Wall Anti-Aim", vars.antiaim.manual.edgeyaw,
					[]() { return enable_antiaim(); }));

				antiaim_other->add_element(new c_keybind("Manual Left", vars.antiaim.manual.left,
					[]() { return enable_antiaim(); }));

				antiaim_other->add_element(new c_keybind("Manual Right", vars.antiaim.manual.right,
					[]() { return enable_antiaim(); }));

				antiaim_other->add_element(new c_combo("Desync Peek Direction",
					&vars.movable.freestand, {
						"Disabled",
						"Hide Head",
						"Peek Head"
						//"Spam"
					}, enable_antiaim));

				antiaim_other->add_element(new c_keybind("Fake Peek", vars.antiaim.fakepeek,
					[]() { return enable_antiaim(); }));

				antiaim_other->add_element(new c_colorpicker(&vars.misc.AutoPeekCol,
					color_t(255, 255, 255, 255)));


				antiaim_other->add_element(new c_keybind("Quick Peek Assist", vars.misc.AutoPeek));

				antiaim_other->add_element(new c_keybind("Duck Peek Assist", vars.antiaim.fakeduck,
					[]() { return enable_antiaim(); }));

				antiaim_other->add_element(new c_combo("Duck Peek Assist Mode",
					&vars.antiaim.FD, {
						"Default",
						"Prefer Duck",
						"Valve"
						//"Spam"
					}, enable_antiaim));



				antiaim_other->add_element(new c_keybind("Slow Walk", vars.antiaim.slowwalk,
					[]() { return enable_antiaim(); }));


				antiaim_other->add_element(new c_slider("Fake Ping", &vars.misc.fakeping, 0, 200, "%.0f ms"));


				antiaim_other->add_element(new c_checkbox("Jitter Desync Direction",
					&vars.ragebot.autoinvert, enable_antiaim));

				antiaim_other->add_element(new c_checkbox("Custom Jitter Desync Direction Speed",
					&vars.ragebot.autospeed, enable_antiaim));

				antiaim_other->add_element(new c_slider("Jitter Desync Direction Speed", &vars.ragebot.invertspeed, 1, 200, "%.0f ",
					enable_antiaim));


					
				antiaim_other->initialize_elements();
			}
			window->add_element(antiaim_other);
			reinit_weapon_cfg();
		}
		window->add_tab(new c_tab("N", tab_t::esp, window)); {

			auto esp_main = new c_child("ESP", tab_t::esp, window);
			esp_main->set_size(Vector2D(350, 470));
			esp_main->set_position(Vector2D(17, 36)); {


				esp_main->add_element(new c_checkbox("Enable",
					&vars.visuals.enable));





				esp_main->add_element(new c_checkbox("Dormant ESP",
					&vars.visuals.dormant, enable_esp));



				esp_main->add_element(new c_checkbox("Player Name",
					&vars.visuals.name, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.glow_color,
					color_t(255, 0, 255, 150), []()
					{ return vars.visuals.glow; }));

				esp_main->add_element(new c_checkbox("Glow", &vars.visuals.glow, enable_esp));

				esp_main->add_element(new c_combo("Glow Mode", &vars.visuals.glowtype, {
					"Outline",
					"Pulsating",
					}, []() { return vars.visuals.glow && enable_esp;  }));

				esp_main->add_element(new c_colorpicker(&vars.visuals.box_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.box; }));

				esp_main->add_element(new c_checkbox("Bounding Box",
					&vars.visuals.box, enable_esp));

				esp_main->add_element(new c_checkbox("Health Bar",
					&vars.visuals.healthbar, enable_esp));
				esp_main->add_element(new c_colorpicker(&vars.visuals.hp_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.override_hp; }));
				esp_main->add_element(new c_checkbox("Override Health Bar Color",
					&vars.visuals.override_hp, [] { return enable_esp() && vars.visuals.healthbar; }));


				esp_main->add_element(new c_colorpicker(&vars.visuals.skeleton_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.skeleton; }));

				esp_main->add_element(new c_checkbox("Player Skeleton",
					&vars.visuals.skeleton, enable_esp));







				esp_main->add_element(new c_colorpicker(&vars.visuals.weapon_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.weapon; }));

				esp_main->add_element(new c_checkbox("Weapon Name",
					&vars.visuals.weapon, enable_esp));


				esp_main->add_element(new c_checkbox("Weapon Icon",
					&vars.visuals.weapon_icon, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.ammo_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.ammo; }));

				esp_main->add_element(new c_checkbox("Weapon Ammo",
					&vars.visuals.ammo, enable_esp));

				esp_main->add_element(new c_colorpicker(&vars.visuals.flags_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.flags > 0; }));

				esp_main->add_element(new c_multicombo("Player Flags",
					&vars.visuals.flags, {
						"Armor",
						"Scoped",
						"Flashed",
						"Vulnerable",
						"Resolver Mode",
						"Choke"
					}, enable_esp));

				esp_main->add_element(new c_checkbox("Thirdperson",
					&vars.misc.thirdperson, enable_esp, vars.misc.thirdperson_bind));

				esp_main->add_element(new c_slider("", &vars.visuals.thirdperson_dist, 0, 300, "%.0f", enable_esp));

				esp_main->add_element(new c_slider("Aspect Ratio", &vars.visuals.aspect_ratio, 0, 250, "%.0f", enable_esp));



				esp_main->add_element(new c_colorpicker(&vars.visuals.aimbot_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.shot_multipoint && enable_esp; }));
				esp_main->add_element(new c_checkbox("Visualize Aimbot", &vars.visuals.shot_multipoint, enable_esp));


				esp_main->add_element(new c_checkbox("Visualize Multipoint", &vars.visuals.galaxy, []() { return enable_esp() && vars.visuals.shot_multipoint; }));


				esp_main->add_element(new c_colorpicker(&vars.visuals.hitmarker_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.hitmarker > 0 && enable_esp; }));

				esp_main->add_element(new c_combo("Hitmarker",
					&vars.visuals.hitmarker, { "Disabled", "3D" }, enable_esp));



				esp_main->add_element(new c_colorpicker(&vars.visuals.out_of_fov_color,
					color_t(255, 255, 255, 255), []() { return enable_esp() && vars.visuals.out_of_fov; }));

				esp_main->add_element(new c_checkbox("Out Of FOV Arrows",
					&vars.visuals.out_of_fov, enable_esp));

				esp_main->add_element(new c_slider("Size", &vars.visuals.out_of_fov_size, 10, 35, "%.0f px",
					[]() { return enable_esp() && vars.visuals.out_of_fov; }));
				esp_main->add_element(new c_slider("Distance", &vars.visuals.out_of_fov_distance, 5, 50, "%.0f",
					[]() { return enable_esp() && vars.visuals.out_of_fov; }));

				esp_main->add_element(new c_multicombo("FOV Arrows Flags",
					&vars.visuals.FOVArrowsFlags, { "Dormant", "Health", "Name", "Pulsating" }, []() { return enable_esp() && vars.visuals.out_of_fov; }));
				esp_main->initialize_elements();






				//move to world






				esp_main->initialize_elements();
			}
			window->add_element(esp_main);

			reinit_chams();
		}
		window->add_tab(new c_tab("S", tab_t::world, window)); {

			auto world_child = new c_child("Miscellaneous", tab_t::world, window);
			world_child->set_size(Vector2D(350, 470));
			world_child->set_position(Vector2D(380, 36));
			{


				world_child->add_element(new c_colorpicker(&vars.visuals.ScopeColor,
					color_t(136, 122, 255, 255), []() { return vars.visuals.ScopeTHing == 2; }));


				world_child->add_element(new c_combo("Scope Overlay",
					&vars.visuals.ScopeTHing, {
						"Default",
						"Lines",
						"Custom"
					}));

				world_child->add_element(new c_colorpicker(&vars.visuals.innacuracyoverlay_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.innacuracyoverlay; }));

				world_child->add_element(new c_checkbox("Spread Circle",
					&vars.visuals.innacuracyoverlay));



				world_child->add_element(new c_checkbox("Preserve Killfeed",
					&vars.visuals.PreserveKills));

				world_child->add_element(new c_colorpicker(&vars.visuals.eventlog_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.eventlog; }));

				world_child->add_element(new c_checkbox("Event Log",
					&vars.visuals.eventlog));

				world_child->add_element(new c_multicombo("Indicators",
					&vars.visuals.indicators, {
						"Antiaim",
						"Lag Compensation",
						"Fake Duck",
						"Damage Override",
						"Doubletap",
						"DA",
						"Moving Antiaim Debug",
						"Doubletap Recharge"
					}));

				world_child->add_element(new c_colorpicker(&vars.visuals.bullet_tracer_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.bullet_tracer; }));

				world_child->add_element(new c_checkbox("Buller Tracer",
					&vars.visuals.bullet_tracer));

				world_child->add_element(new c_colorpicker(&vars.visuals.bullet_tracer_local_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.bullet_tracer_local; }));

				world_child->add_element(new c_checkbox("Local Bullet Tracer",
					&vars.visuals.bullet_tracer_local));

				world_child->add_element(new c_combo("Bullet Tracer Type", &vars.visuals.bullet_tracer_type,
					{
						"Line",
						"Beam",
						"Bubble",
						"Glow"
					},
					[]() { return vars.visuals.bullet_tracer; }));
				
				world_child->add_element(new c_checkbox("Enable Optimization",
					&vars.visuals.optimize));

				world_child->add_element(new c_multicombo("Features to Optimize",
					&vars.visuals.To_Op, {
						"ESP",
						"Chams",
						"Glow"
					}, []() {return vars.visuals.optimize; }));

				world_child->add_element(new c_combo("Optimization Strength",
					&vars.visuals.op_strength, {
						"None",
						"Low",
						"Medium",
						"Max"
					}, []() {return vars.visuals.optimize; }));

				






				world_child->add_element(new c_checkbox("Enable Impacts",
					&vars.visuals.bullet_impact));


				world_child->add_element(new c_colorpicker(&vars.visuals.bullet_impact_color,
					color_t(255, 0, 0, 255), []() { return vars.visuals.bullet_impact; }));

				world_child->add_element(new c_text("Server Impact", []() { return vars.visuals.bullet_impact; }));

				world_child->add_element(new c_colorpicker(&vars.visuals.client_impact_color,
					color_t(255, 0, 0, 255), []() { return vars.visuals.bullet_impact; }));

				world_child->add_element(new c_text("Client Impact", []() { return vars.visuals.bullet_impact; }));


				world_child->initialize_elements();
			}
			window->add_element(world_child);

			auto effects_child = new c_child("Effects", tab_t::world, window);
			effects_child->set_size(Vector2D(350, 470));
			effects_child->set_position(Vector2D(17, 36)); {



				effects_child->add_element(new c_checkbox("Night Mode", &vars.visuals.nightmode));
				{
					effects_child->add_element(new c_colorpicker(&vars.visuals.nightmode_color,
						color_t(101, 97, 107, 255), []() { return vars.visuals.nightmode; }));

					effects_child->add_element(new c_text("World Color", []() { return vars.visuals.nightmode; }));


					effects_child->add_element(new c_colorpicker(&vars.visuals.nightmode_prop_color,
						color_t(255, 255, 255, 255), []() { return vars.visuals.nightmode; }));

					effects_child->add_element(new c_text("Prop Color", []() { return vars.visuals.nightmode; }));

					effects_child->add_element(new c_combo("Skybox",
						&vars.misc.skyboxx, {
							"Galaxy",
							"Tibet",
							"Daylight",
							"Night",
							"Night 2",
							"Night Flat",
							"Cloudy Night",
							"Cloudy",
							"Rainy"
						}, []() { return vars.visuals.nightmode; }));

					effects_child->add_element(new c_colorpicker(&vars.visuals.nightmode_skybox_color,
						color_t(194, 101, 35, 255), []() { return vars.visuals.nightmode; }));

					effects_child->add_element(new c_text("Skybox Color", []() { return vars.visuals.nightmode; }));
				}

				effects_child->add_element(new c_colorpicker(&vars.visuals.ambient_color,
					color_t(101, 97, 107, 255)));

				effects_child->add_element(new c_combo("Ambient Light", &vars.visuals.ambientlight, {
					"Disable",
					"Fullbright",
					"Custom"
					}));

				effects_child->add_element(new c_combo("Model Lights", &vars.visuals.modellight, {
					"Disable",
					"Fullbright"
					}));

				effects_child->add_element(new c_colorpicker(&vars.visuals.BombColor,
					color_t(101, 97, 107, 255)));

				effects_child->add_element(new c_multicombo("Bomb",
					&vars.visuals.BombT, {
						"Timer",
						"Position",
						"Out of FOV",
						"Gradient",
						"Indicator",
						"Defuse Timer"
					}));


				effects_child->add_element(new c_colorpicker(&vars.visuals.GrenadeColor,
					color_t(101, 97, 107, 255)));
				effects_child->add_element(new c_multicombo("Grenade Proximity Warning",
					&vars.visuals.GrenadeProximityWarning, {
						"End Location",
						"Trail",
						"Glow"
					}));
				effects_child->add_element(new c_colorpicker(&vars.visuals.MollyColor,
					color_t(101, 97, 107, 255)));
				effects_child->add_element(new c_multicombo("Fire Proximity Warning",
					&vars.visuals.MollyWarning, {
						"Fire Spread",
						"Fire Timer",
						"Fire Colour"
					}));

				effects_child->add_element(new c_colorpicker(&vars.visuals.localmemecolor,
					color_t(101, 97, 107, 255)));

				effects_child->add_element(new c_multicombo("Effects",
					&vars.visuals.localmemes, {
						"Local Glow",
						"Enemy Glow"
					}));

			

				effects_child->add_element(new c_colorpicker(&vars.visuals.nadepred_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.nadepred; }));

				effects_child->add_element(new c_checkbox("Grenade Prediction",
					&vars.visuals.nadepred));

				effects_child->add_element(new c_checkbox("Force Crosshair",
					&vars.visuals.force_crosshair));

				effects_child->add_element(new c_checkbox("Kill Effect",
					&vars.visuals.kill_effect));

				effects_child->add_element(new c_slider("World FOV", &vars.misc.worldfov, 90, 145, "%.0f"));

				effects_child->add_element(new c_text("Skybox Color", []() { return vars.visuals.nightmode; }));




				effects_child->add_element(new c_multicombo("Removals",
					&vars.visuals.remove, {
						"Visual Recoil",
						"Smoke",
						"Flash",
						"Scope",
						"Zoom",
						"Post Processing",
						"Fog",
						"Shadows"
					}));
				effects_child->initialize_elements();


			}
			window->add_element(effects_child);
		}

		window->add_tab(new c_tab("U", tab_t::misc, window));
		{
			auto main_child = new c_child("Main", tab_t::misc, window);
			main_child->set_size(Vector2D(350, 470));
			main_child->set_position(Vector2D(17, 36));
			{
				main_child->add_element(new c_checkbox("Anti-Untrusted",
					&vars.misc.antiuntrusted));



				main_child->add_element(new c_checkbox("Watermark",
					&vars.visuals.watermark));

				main_child->add_element(new c_checkbox("Keybinds",
					&vars.menu.keybinds));

				main_child->add_element(new c_slider("Keybind X", &vars.menu.keybindX, 0, 1920, "%.0f", []() {return vars.menu.keybinds; }));

				main_child->add_element(new c_slider("Keybind Y", &vars.menu.keybindY, 0, 1080, "%.0f", []() {return vars.menu.keybinds; }));

				main_child->add_element(new c_checkbox("Auto Bunnyhop",
					&vars.misc.bunnyhop));


				main_child->add_element(new c_checkbox("Clantag",
					&vars.visuals.clantagspammer));



				main_child->add_element(new c_combo("Chat Spam",
					&vars.misc.chatspam, {
						"Disabled",
						"TrashTalk Cheat",
						"TrashTalk"

					}));
				
				main_child->add_element(new c_checkbox("Killsay", &vars.misc.killsaysandshit.enablekillsay));
				{
					main_child->add_element(new c_input_text("Killsay Text",&vars.misc.killsaysandshit.killsay,false, []() {return vars.misc.killsaysandshit.enablekillsay; }));
					main_child->add_element(new c_multicombo("Killsay Flags", &vars.misc.killsaysandshit.killsayflags, {
						"Include Player name at Start",
						"Include Player name at End"

						}, []() {return vars.misc.killsaysandshit.enablekillsay; }));
				}

				main_child->add_element(new c_checkbox("Deathsay", &vars.misc.killsaysandshit.enabledeathsay));
				
				main_child->add_element(new c_input_text("Deathsay Text", &vars.misc.killsaysandshit.deathsay, false, []() {return vars.misc.killsaysandshit.enabledeathsay; }));
				
				


				main_child->add_element(new c_combo("Hitsound",
					&vars.visuals.hitmarker_sound, {
						"None",
						"Arena Switch",
						"Flick",
						"Hit"
					}));

				main_child->add_element(new c_checkbox("Knife Bot",
					&vars.misc.knifebot));

				main_child->add_element(new c_combo("Leg Movement",
					&vars.antiaim.Legs, {
						"Walking",
						"Sliding",
						"Static"
				//		"Walking and Sliding"
					}));

				main_child->add_element(new c_combo("Air Movement",
					&vars.antiaim.airl, {
						"Normal",
						"Static"
					}));









				main_child->add_element(new c_checkbox("Outline Accent",
					&vars.menu.outline));


				main_child->add_element(new c_checkbox("Hold Firing Animation",
					&vars.misc.hold_firinganims));


				//main_child->add_element(new c_slider("Indicator Scalling", &vars.visuals.disclay, 0, 15, "%.0f"));
				//main_child->add_element(new c_slider("Indicator Offset", &vars.visuals.indicatoroffset, 0, 430, "%.0f"));

				/*
				main_child->add_element(new c_input_text("New Name", &vars.menu.serverIP, false));

				main_child->add_element(new c_button("Rename", rename));

				main_child->add_element(new c_input_text("Fake Vote text", &vars.menu.fakevote, false));

				main_child->add_element(new c_button("Fake Vote", fakevote));

				main_child->add_element(new c_button("Remove Name", removename)); */

				//main_child->add_element(new c_button("full update", []() { csgo->client_state->ForceFullUpdate(); }));
				//main_child->add_element(new c_button("unlock hidden cvars", EnableHiddenCVars));
				
				main_child->initialize_elements();
			}
			window->add_element(main_child);

			auto main_2 = new c_child("Extras", tab_t::misc, window);
			main_2->set_size(Vector2D(350, 470));
			main_2->set_position(Vector2D(380, 36));
			{
				main_2->add_element(new c_checkbox("Legacy",
					&vars.movable.leagcy));

				main_2->add_element(new c_combo("Menu Color",
					&vars.movable.colormode, {
						"Purple",
						"Blue",
						"Dark",
						"Light"
					}));

				main_2->add_element(new c_slider("Animation Speed", &vars.CMenu.animationspeed, 50.f, 175.f, "%.0f"));

				main_2->add_element(new c_colorpicker(&vars.movable.PCol,
					color_t(143, 150, 255, 255), []() { return vars.movable.leagcy; }));
				main_2->add_element(new c_text("Keybinds Theme", []() { return vars.movable.leagcy; }));

				main_2->add_element(new c_checkbox("Rage Auto Buy",
					&vars.misc.autobuy.enable));

				main_2->add_element(new c_combo("Primary",
					&vars.misc.autobuy.main, {
						"Auto-Sniper",
						"Scout",
						"AWP"
					}, []() { return vars.misc.autobuy.enable; }));

				main_2->add_element(new c_combo("Secondary",
					&vars.misc.autobuy.pistol, {
						"Dual Beretta",
						"Deagle / Revolver"
					}, []() { return vars.misc.autobuy.enable; }));

				main_2->add_element(new c_multicombo("Other",
					&vars.misc.autobuy.misc, {
						"Helmet",
						"Armour",
						"Explosive Grenade",
						"Molotov",
						"Smoke",
						"Taser",
						"Defuse Kit",
					}, []() { return vars.misc.autobuy.enable; }));

				main_2->add_element(new c_text("Viewmodel"));

				main_2->add_element(new c_slider("", &vars.misc.viewmodel_x, -50.f, 50.f, "x: %.0f"));

				main_2->add_element(new c_slider("", &vars.misc.viewmodel_y, -50.f, 50.f, "y: %.0f"));

				main_2->add_element(new c_slider("", &vars.misc.viewmodel_z, -50.f, 50.f, "z: %.0f"));

				main_2->add_element(new c_slider("Viewmodel FOV", &vars.misc.viewmodelfov, 68, 145, "%.0f*"));
				//main_2->add_element(new c_button("Unlock Cvars", EnableHiddenCVars));
				main_2->initialize_elements();
			}
			window->add_element(main_2);
		}
		window->add_tab(new c_tab("B", tab_t::cfgsrp, window)); {
			//reinit_scripts();
			reinit_config();
		}

		window->set_active_tab_index(tab_t::rage);
		initialized = true;
	}

	if (!vars.menu.open) {
		if (window->get_transparency() > 0.f)
			window->decrease_transparency(animation_speed * 160.f);
	}

	ImGui::GetIO().MouseDrawCursor = window->get_transparency() > 0;
	POINT mp;
	GetCursorPos(&mp);
	ScreenToClient(GetForegroundWindow(), &mp);
	g_mouse.x = mp.x;
	g_mouse.y = mp.y;
	if (should_reinit_weapon_cfg) {
		reinit_weapon_cfg();
		should_reinit_weapon_cfg = false;
	}
	if (should_reinit_legit_cfg) {
		reinit_legit_cfg();
		should_reinit_legit_cfg = false;
	}
	if (should_reinit_chams) {
		reinit_chams();
		should_reinit_chams = false;
	}
	if (should_reinit_config) {
		reinit_config();
		should_reinit_config = false;
	}
	//if (should_reinit_scripts) {
	//	reinit_scripts();
	//	should_reinit_scripts = false;
	//}
	window->render();
	if (window->g_hovered_element) {
		if (window->g_hovered_element->type == c_elementtype::input_text)
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
	}
	csgo->scroll_amount = 0;
}



void c_menu::update_binds()
{
	for (auto e : window->elements) {
		if (e->type == c_elementtype::child) {
			for (auto el : ((c_child*)e)->elements) {
				if (el->type == c_elementtype::checkbox) {
					auto c = (c_checkbox*)el;
					auto binder = c->bind;
					if (binder) {
						binder->key = std::clamp<unsigned int>(binder->key, 0, 255);

						if (binder->type == 2 && binder->key > 0) {
							if (window->key_updated(binder->key)) {
								*(bool*)c->get_ptr() = !(*(bool*)c->get_ptr());
							}
						}
						else if (binder->type == 1 && binder->key > 0) {
							*(bool*)c->get_ptr() = csgo->key_pressed[binder->key];
						}
						else if (binder->type == 3 && binder->key > 0) {
							*(bool*)c->get_ptr() = !csgo->key_pressed[binder->key];
						}
						binder->active = *(bool*)c->get_ptr();
					}
				}
				else if (el->type == c_elementtype::keybind) {
					auto c = (c_keybind*)el;
					auto binder = ((c_keybind*)el)->bind;
					if (binder) {
						binder->key = std::clamp<unsigned int>(binder->key, 0, 255);

						if (binder->type == 2 && binder->key > 0) {
							if (window->key_updated(binder->key)) {
								binder->active = !binder->active;
							}
						}
						else if (binder->type == 1 && binder->key > 0) {
							if (binder->key == VK_MENU) {
								binder->active = (GetAsyncKeyState(binder->key) & (1 << 16));

							}
							else {
								binder->active = csgo->key_pressed[binder->key];
							}
						}
						else if (binder->type == 3 && binder->key > 0) {
							binder->active = !csgo->key_pressed[binder->key];
						}
						else if (binder->type == 0)
							binder->active = false;
						else if (binder->type == 4)
							binder->active = true;
					}
				}
			}
		}
	}
}

bool override_default() {
	return vars.ragebot.enable && (vars.ragebot.active_index == 0 || vars.ragebot.weapon[vars.ragebot.active_index].enable);
}

bool legit_override() {
	return vars.legitbot.enable && (vars.legitbot.CurrentWeaponConfig == 0 || vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].enable);
}

void c_menu::reinit_legit_cfg()
{
	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "Aimbot") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}
	auto cfg_child = new c_child("Aimbot", tab_t::legit, window);
	cfg_child->set_size(Vector2D(350, 470));
	cfg_child->set_position(Vector2D(350 + 30, 36)); {
		cfg_child->add_element(new c_combo("Current Weapon", &vars.legitbot.CurrentWeaponConfig, {
			"Default",
			"Pistols",
			"Rifles",
			"Heavy Pistols",
			"Snipers",
			"AWP",
			"SMG",
			"Shotguns",
			"Heavy"
			}, enable_legit, [](int) { g_Menu->should_reinit_legit_cfg = true; }));

		cfg_child->add_element(new c_checkbox("Override Default", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].enable,
			[]() { return enable_legit() && vars.legitbot.CurrentWeaponConfig > 0; }));


		cfg_child->add_element(new c_combo("Hitchance", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].hitchancetype, {
		"Dynamic",
		"Custom"
			}, legit_override));

		cfg_child->add_element(new c_slider("Custom Hitchance", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].hitchance, 0, 100, "%.0f%%",
			[]() {return legit_override() && vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].hitchance == 1; }));

		cfg_child->add_element(new c_combo("Targetted Damage", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].mindamagetype, {
			"Dynamic",
			"Custom"
			}, legit_override));

		cfg_child->add_element(new c_slider("Custom Targetted Damage", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].mindamage, 0, 100, "%.0f% HP",
			[]() {return legit_override() && vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].mindamagetype == 1; }));

		cfg_child->add_element(new c_multicombo("Targetted Hitboxes", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].Hitscan, {
			"Head",
			"Chest",
			"Stomach",
			"Legs"
			}, legit_override));

		cfg_child->add_element(new c_slider("Smoothing", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].smoothing, 0, 100, "%.0f%%",
			legit_override));


		cfg_child->add_element(new c_slider("Maximum FOV", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].fov, 0, 180, "%.0f FOV",
			legit_override));

		cfg_child->add_element(new c_slider("Reaction Time", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].reactiontime, 0, 180, "%.0f ms",
			legit_override));
/*

		cfg_child->add_element(new c_checkbox("Silent Aimbot", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].enable,
			[]() {return	legit_override(); }));

		cfg_child->add_element(new c_multicombo("Silent Targetted Hitboxes", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].silentHitscan, {
			"Head",
			"Chest",
			"Stomach",
			"Legs"
			}, []() {return	legit_override() && vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].silentenable; }));
		
		cfg_child->add_element(new c_slider("Silent Maximum FOV", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].silentfov, 0, 180, "%.0f*",
			[]() {return	legit_override() && vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].silentenable; }));

		cfg_child->add_element(new c_slider("Silent Reaction Time", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].silentreactiontime, 0, 180, "%.0f ms",
			[]() {return	legit_override() && vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].silentenable; }));

			*/
		cfg_child->add_element(new c_checkbox("Auto Stop", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].autostop,
			legit_override));

		cfg_child->add_element(new c_slider("Auto Stop Reaction Time", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].reactiontime, 0, 180, "%.0f ms",
			[]() {return	legit_override() && vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].autostop; }));

		cfg_child->add_element(new c_checkbox("Visible Only", &vars.legitbot.weapons[vars.legitbot.CurrentWeaponConfig].visibleonly,
			legit_override));

		cfg_child->initialize_elements();
	}
	window->add_element(cfg_child);

}
void c_menu::reinit_weapon_cfg()
{
	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "Targetting") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}
	auto cfg_child = new c_child("Targetting", tab_t::rage, window);
	cfg_child->set_size(Vector2D(350, 470));
	cfg_child->set_position(Vector2D(350 + 30, 36)); {
		cfg_child->add_element(new c_combo("Current Weapon", &vars.ragebot.active_index, {
			"Default",
			"Scar",
			"Scout",
			"AWP",
			"Rifle",
			"Pistol",
			"Heavy Pistol"
			}, enable_rage, [](int) { g_Menu->should_reinit_weapon_cfg = true; }));

		cfg_child->add_element(new c_checkbox("Override Default", &vars.ragebot.weapon[vars.ragebot.active_index].enable,
			[]() { return enable_rage() && vars.ragebot.active_index > 0; }));

		cfg_child->add_element(new c_multicombo("Targetted Hitboxes", &vars.ragebot.weapon[vars.ragebot.active_index].hitscan, {
			"Head",
			"Neck",
			"Upper Chest",
			"Lower Chest",
			"Stomach",
			"Pelvis",
			"Legs",
			"Feet"
			}, override_default));

		cfg_child->add_element(new c_multicombo("Targetted Body Hitboxes", &vars.ragebot.weapon[vars.ragebot.active_index].hitscan_baim, {
		"Chest",
		"Stomach",
		"Pelvis",
		"Legs",
		"Feet",
			},
			[]() { return
			override_default()
			&& (vars.ragebot.weapon[vars.ragebot.active_index].baim > 0 ||
				vars.ragebot.weapon[vars.ragebot.active_index].baim_under_hp > 0
				|| vars.ragebot.force_body->key > 0);
			}));



		cfg_child->add_element(new c_slider("Hitchance", &vars.ragebot.weapon[vars.ragebot.active_index].hitchance, 0, 100, "%.0f%%",
			override_default));

		/*
		cfg_child->add_element(new c_combo("Hitchance Type", &vars.ragebot.weapon[vars.ragebot.active_index].hitchancetype, {
			"Default",
			"Faster",
			}, []() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].hitchance > 0; }));*/

		cfg_child->add_element(new c_slider("Targetted Damage", &vars.ragebot.weapon[vars.ragebot.active_index].mindamage, 0, 120, "%.0f hp",
			override_default));

		cfg_child->add_element(new c_slider("Overriden Targetted Damage", &vars.ragebot.weapon[vars.ragebot.active_index].mindamage_override, 0, 120, "%.0f hp",
			[]() { return override_default() && vars.ragebot.override_dmg->key > 0; }));

		cfg_child->add_element(new c_checkbox("Scale Targetted Damage", &vars.ragebot.weapon[vars.ragebot.active_index].dynamicdamage,
			[]() { return override_default(); }));


		cfg_child->add_element(new c_checkbox("Enable Static Multipoint", &vars.ragebot.weapon[vars.ragebot.active_index].multipoint, override_default));

		cfg_child->add_element(new c_slider("Head Multipoint Scale", &vars.ragebot.weapon[vars.ragebot.active_index].pointscale_head,
			0, 100, "%.0f%%", []() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].multipoint; }));

		cfg_child->add_element(new c_slider("Body Multipoint Scale", &vars.ragebot.weapon[vars.ragebot.active_index].pointscale_body,
			0, 100, "%.0f%%", []() { return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].multipoint; }));


		cfg_child->add_element(new c_checkbox("Auto Stop", &vars.ragebot.weapon[vars.ragebot.active_index].quickstop,
			[]() { return override_default(); }));

		cfg_child->add_element(new c_multicombo("Auto Stop Modifier", &vars.ragebot.weapon[vars.ragebot.active_index].quickstop_options, {
			"Slow Walk", //1
			"Between Shots", //2
			"Lethal Only", // 4
			"Early" //8
			},
			[]() {
				return override_default() && vars.ragebot.weapon[vars.ragebot.active_index].quickstop;
			}));









		

		cfg_child->add_element(new c_multicombo("Body Aim Conditions", &vars.ragebot.weapon[vars.ragebot.active_index].baim, {
	"In Air",
	"Unresolved",
	"Lethal"
			},
			override_default));

	
		cfg_child->add_element(new c_checkbox("Prefer Body Aim", &vars.ragebot.weapon[vars.ragebot.active_index].adaptive_baim,
			[]() {
				return override_default();

			}));

	
		cfg_child->initialize_elements();
	}
	window->add_element(cfg_child);
}


void c_menu::reinit_config() {
	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "Lists") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}

	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "Actions") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}
	RefreshConfigs();
	auto cfg = new c_child("Lists", tab_t::cfgsrp, window);
	cfg->set_size(Vector2D(350, 470));
	cfg->set_position(Vector2D(17, 36));
	{
		if (ConfigList.size() != 0) {
			cfg->add_element(new c_text("Current Config: " + vars.menu.LoadedConfig));
			cfg->add_element(new c_combo("Configs List", &vars.menu.active_config_index, ConfigList));
		}



		//cfg->add_element(new c_multicombo("Scripts", &vars.menu.active_scripts_index, {"CloudRadar.lua"}));
		cfg->initialize_elements();
	}
	window->add_element(cfg);
	auto config_child = new c_child("Actions", tab_t::cfgsrp, window);
	config_child->set_size(Vector2D(350, 470));
	config_child->set_position(Vector2D(380, 36)); {
		//config_child->add_element(new c_listbox("Configs", &vars.menu.active_config_index, ConfigList, 150.f));




		config_child->add_element(new c_input_text("New Config Name", &vars.menu.active_config_name, false));

		config_child->add_element(new c_button("Create Config", []() {
			string add;
			if (vars.menu.active_config_name.find(".json") == -1)
				add = ".json";
			Config.Save(vars.menu.active_config_name + add);
			g_Menu->should_reinit_config = true;
			vars.menu.active_config_name.clear();
			}));

		config_child->add_element(new c_button("Load Config", []() {
			Config.Load(ConfigList[vars.menu.active_config_index]);
			g_Menu->should_reinit_config = true;
			}, []() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		config_child->add_element(new c_button("Save Config", []() {
			Config.Save(ConfigList[vars.menu.active_config_index]);
			}, []() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));





		config_child->add_element(new c_button("Reset to Default", []() { Config.ResetToDefault(true); },
			[]() { return ConfigList.size() > 0 && vars.menu.active_config_index >= 0; }));

		config_child->add_element(new c_button("Refresh Configs List", []() { g_Menu->should_reinit_config = true; }));


		config_child->initialize_elements();
	}
	window->add_element(config_child);
}

void c_menu::reinit_scripts() {
	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "configs") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}

	/*auto scripts_fucn = new c_child("scripts func", tab_t::cfgsrp, window);
	scripts_fucn->set_size(Vector2D(230, 460));
	scripts_fucn->set_position(Vector2D(120, -50));
	{
		scripts_fucn->initialize_elements();
	}
	window->add_element(scripts_fucn);*/

	auto scripts = new c_child("Scripting", tab_t::cfgsrp, window);
	scripts->set_size(Vector2D(350, 470));
	scripts->set_position(Vector2D(17, 36));
	{
		scripts->add_element(new c_text("Under Contruction"));
		/*
		static auto js_files = c_js_loader::refresh_scripts(js_funcs.get_js_files());
		scripts->add_element(new c_button("refresh scripts", []()
			{
				js_files = c_js_loader::refresh_scripts(js_funcs.get_js_files());
				RefreshScripts();
				RefreshConfigs();
				g_Menu->should_reinit_scripts = true;
				g_Menu->should_reinit_config = true;
			}));

		for (auto& js_script : c_js_loader::scripts)
		{
			std::string new_cb_name = js_script.name;
			std::string new_combo_name = "Hooked:" + js_script.name;
			scripts->add_element(new c_checkbox(new_cb_name.c_str(), &js_script.is_enabled));

			scripts->add_element(new c_combo(new_combo_name.c_str(), &js_script.execution_location,
				{
					"disabled","create move","frame stage notify","paint traverse"
				}));
		}
		*/
		scripts->initialize_elements();
	}
	window->add_element(scripts);
}

void c_menu::reinit_chams() {
	for (int i = 0; i < window->elements.size(); i++) {
		auto& e = window->elements[i];
		if (((c_child*)e)->get_title() == "Textured Models") {
			window->elements.erase(window->elements.begin() + i);
			break;
		}
	}
	auto cfg_child = new c_child("Textured Models", tab_t::esp, window);
	cfg_child->set_size(Vector2D(350, 470));
	cfg_child->set_position(Vector2D(380, 36)); {
		cfg_child->add_element(new c_combo("Selected Type", &vars.visuals.active_chams_index, {
				"Opponents",
				"Backtrack",
				"Local Player",
				"Local Desync",
				"Local Arms",
				"Weapons"
			}, nullptr, [](int) { g_Menu->should_reinit_chams = true; }));

		switch (vars.visuals.active_chams_index)
		{
		case 0: { // enemy
			cfg_child->add_element(new c_colorpicker(&vars.visuals.chamscolor,
				color_t(255, 0, 0, 255), []() { return vars.visuals.chams; }));
			cfg_child->add_element(new c_checkbox("Enable Visible Chams", &vars.visuals.chams));

			cfg_child->add_element(new c_combo("Chams Material", &vars.visuals.chamstype, {
					"Normal",
					"Flat",
					"Bubble"

				}, []() { return vars.visuals.chams || vars.visuals.chamsxqz; }));





			cfg_child->add_element(new c_combo("Visible Chams Effect", &vars.visuals.overlay, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
				}, []() { return vars.visuals.chams; }));

			cfg_child->add_element(new c_colorpicker(&vars.visuals.glow_col,
				color_t(255, 255, 255, 0), []() { return vars.visuals.chams && vars.visuals.overlay > 0; }));
			cfg_child->add_element(new c_text("Visible Chams Effect Color", []() { return vars.visuals.chams && vars.visuals.overlay > 0; }));

			cfg_child->add_element(new c_colorpicker(&vars.visuals.chamscolor_xqz,
				color_t(0, 100, 255, 255), []() { return vars.visuals.chamsxqz; }));
			cfg_child->add_element(new c_checkbox("Enable XQZ Chams", &vars.visuals.chamsxqz));

			cfg_child->add_element(new c_combo("XQZ Chams Effect", &vars.visuals.overlay_xqz, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
				}, []() { return vars.visuals.chamsxqz; }));

			cfg_child->add_element(new c_colorpicker(&vars.visuals.glow_col_xqz,
				color_t(255, 255, 255, 0), []() { return vars.visuals.chamsxqz && vars.visuals.overlay_xqz > 0; }));
			cfg_child->add_element(new c_text("XYZ Chams Effect Color", []() { return vars.visuals.chamsxqz && vars.visuals.overlay_xqz > 0; }));






		}
			  break;
		case 1: // history
		{
			cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[history].clr,
				color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[history].enable; }));
			cfg_child->add_element(new c_checkbox("Enable", &vars.visuals.misc_chams[history].enable));
			cfg_child->add_element(new c_combo("Chams Material", &vars.visuals.misc_chams[history].material, {
					"Normal",
					"Flat",
					"Bubble"

				}, []() { return vars.visuals.misc_chams[history].enable; }));



			{
				cfg_child->add_element(new c_combo("Chams Effect", &vars.visuals.misc_chams[history].overlay, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
					}, []() { return vars.visuals.misc_chams[history].enable; }));

				cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[history].glow_clr,
					color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[history].enable && vars.visuals.misc_chams[history].overlay > 0; }));
				cfg_child->add_element(new c_text("Chams Effect Color", []() { return vars.visuals.misc_chams[history].enable && vars.visuals.misc_chams[history].overlay > 0; }));
			}



		}
		break;
		case 2: // local
		{
			cfg_child->add_element(new c_colorpicker(&vars.visuals.localchams_color,
				color_t(255, 255, 255, 255), []() { return vars.visuals.localchams; }));
			cfg_child->add_element(new c_checkbox("Enable", &vars.visuals.localchams));

			cfg_child->add_element(new c_combo("Chams Material", &vars.visuals.localchamstype, {
					"Normal",
					"Flat",
					"Bubble"

				}, []() { return vars.visuals.localchams; }));





			{
				cfg_child->add_element(new c_combo("Chams Effect", &vars.visuals.local_chams.overlay, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
					}));

				cfg_child->add_element(new c_colorpicker(&vars.visuals.local_glow_color,
					color_t(255, 255, 255, 255), []() { return vars.visuals.local_chams.overlay > 0; }));
				cfg_child->add_element(new c_text("Chams Effect Color", []() { return vars.visuals.local_chams.overlay > 0; }));
			}



		cfg_child->add_element(new c_slider("Transparency in Scope", &vars.visuals.trnsparency, 0.f, 100.f, "%.0f%%"));


		}
		break;
		case 3: // desync
		{
			cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[desync].clr,
				color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[desync].enable; }));
			cfg_child->add_element(new c_checkbox("Enable", &vars.visuals.misc_chams[desync].enable));

			cfg_child->add_element(new c_combo("Chams Material", &vars.visuals.misc_chams[desync].material, {
					"Normal",
					"Flat",
					"Bubble"

				}, []() { return vars.visuals.misc_chams[desync].enable; }));



			{
				cfg_child->add_element(new c_combo("Chams Effect", &vars.visuals.misc_chams[desync].overlay, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
					}, []() { return vars.visuals.misc_chams[desync].enable; }));

				cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[desync].glow_clr,
					color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[desync].enable && vars.visuals.misc_chams[desync].overlay > 0; }));
				cfg_child->add_element(new c_text("Chams Effect Color", []() { return vars.visuals.misc_chams[desync].enable && vars.visuals.misc_chams[desync].overlay > 0; }));
			}



		}
		break;
		case 4: // arms
		{
			cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[arms].clr,
				color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[arms].enable; }));
			cfg_child->add_element(new c_checkbox("Enable", &vars.visuals.misc_chams[arms].enable));

			cfg_child->add_element(new c_combo("Chams Material", &vars.visuals.misc_chams[arms].material, {
					"Normal",
					"Flat",
					"Bubble"
				}, []() { return vars.visuals.misc_chams[arms].enable; }));



			{
				cfg_child->add_element(new c_combo("Chams Effect", &vars.visuals.misc_chams[arms].overlay, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
					}, []() { return vars.visuals.misc_chams[arms].enable; }));

				cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[arms].glow_clr,
					color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[arms].enable && vars.visuals.misc_chams[arms].overlay > 0; }));
				cfg_child->add_element(new c_text("Chams Effect Color", []() { return vars.visuals.misc_chams[arms].enable && vars.visuals.misc_chams[arms].overlay > 0; }));
			}


		}
		break;
		case 5: // weapon
		{
			cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[weapon].clr,
				color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[weapon].enable; }));
			cfg_child->add_element(new c_checkbox("Enable", &vars.visuals.misc_chams[weapon].enable));

			cfg_child->add_element(new c_combo("Chams Material", &vars.visuals.misc_chams[weapon].material, {
					"Normal",
					"Flat",
					"Bubble"

				}, []() { return vars.visuals.misc_chams[weapon].enable; }));



			{
				cfg_child->add_element(new c_combo("Chams Effect", &vars.visuals.misc_chams[weapon].overlay, {
					"Off",
					"Outline",
					"Glow",
					"Glass"
					}, []() { return vars.visuals.misc_chams[weapon].enable; }));

				cfg_child->add_element(new c_colorpicker(&vars.visuals.misc_chams[weapon].glow_clr,
					color_t(255, 255, 255, 255), []() { return vars.visuals.misc_chams[weapon].enable && vars.visuals.misc_chams[weapon].overlay > 0; }));
				cfg_child->add_element(new c_text("Chams Effect Color", []() { return vars.visuals.misc_chams[weapon].enable && vars.visuals.misc_chams[weapon].overlay > 0; }));
			}


		}
		break;


		}
	}
	cfg_child->initialize_elements();
	window->add_element(cfg_child);
}

c_menu* g_Menu = new c_menu();