#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "Hooks.h"

struct CChamsSettings
{
	int32_t m_iMainMaterial = 0;

	bool m_bRenderChams = false;
	bool m_aModifiers[4] = { false, false, false };

	color_t m_aModifiersColors[4] = { { 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 } };
	color_t m_Color = color_t(255, 255, 255, 255);
};

class CDrawModel : public Singleton<CDrawModel>
{
public:
	 void Run();
	 ITexture* GetTexture() { return this->m_PreviewTexture; };
	 void SetChamsSettings(CChamsSettings Settings) { this->m_ChamsSettings = Settings; };
	 void SetGlow(int glow) { this->m_iGlow = glow; };
	 void SetGlowColor(color_t glow) { this->m_GlowColor = glow; };
	 void Refresh() { this->m_PreviewTexture = nullptr; this->m_PreviewModel = nullptr; this->m_CubemapTexture = nullptr; };
	 float Rotation = -180;
private:
	ITexture* m_PreviewTexture = nullptr;
	ITexture* m_CubemapTexture = nullptr;
	CMergedMDL* m_PreviewModel = nullptr;

	CViewSetup m_ViewSetup = { };
	CChamsSettings m_ChamsSettings = { };

	int32_t m_iGlow = 0;
	color_t m_GlowColor = color_t(255, 255, 255, 255);
};

enum tab_t : int {
	undefined = -1,
	legit,
	rage,
	antiaim,
	esp,
	world,
	cfgsrp,
	misc
};
extern LPDIRECT3DTEXTURE9 ProfilePicture;
//extern LPDIRECT3DTEXTURE9 Logo;
class c_window;
class c_child;
class c_menu
{
public:
	void draw_indicators();
	c_child* weapon_cfg = nullptr;
	c_window* window = nullptr;
	bool initialized = false;
	void render();
	void update_binds();
	void keybinds();
	void watermark();
	bool should_reinit_weapon_cfg = false;
	bool should_reinit_chams = false;
	bool should_reinit_config = false;
	bool should_reinit_scripts = false;
	bool should_reinit_legit_cfg = false;
	void reinit_scripts();
	void reinit_chams();
	void reinit_config();
	void reinit_weapon_cfg();
	void reinit_legit_cfg();
};

extern c_menu *g_Menu;

extern float g_Menucolor_t[4];
