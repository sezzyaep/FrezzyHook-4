#include "Hooks.h"
#include "Menu.h"
#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_freetype.h"
#include <d3dx9tex.h>
#include "AntiAims.h"
#include <chrono>
#include "Bytesa.h"

void GUI_Init(IDirect3DDevice9* pDevice)
{
	if (!vars.menu.guiinited)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(csgo->Init.Window);
		ImGui_ImplDX9_Init(pDevice);

		auto& io = ImGui::GetIO();
		ImFontConfig m_config;
		m_config.OversampleH = m_config.OversampleV = 3;
		m_config.PixelSnapH = false;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x2000, 0x206F, // General Punctuation
			0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
			0x31F0, 0x31FF, // Katakana Phonetic Extensions
			0xFF00, 0xFFEF, // Half-width characters
			0x4e00, 0x9FAF, // CJK Ideograms
			0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
			0x2DE0, 0x2DFF, // Cyrillic Extended-A
			0xA640, 0xA69F, // Cyrillic Extended-B
			0,
		};
		ImFontConfig cfg;
		fonts::menu_main = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 22, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::Keybinds_ = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 18, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::Watermark2 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 22, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::Watermark = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/Nirmala.ttf", 21, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::menu_desc = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/Nirmala.ttf", 20, &m_config, ranges);
		fonts::icon = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/Nirmala.ttf", 23, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::NonBoldIcon = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 23, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::lby_indicator = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/calibrib.ttf", 29, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::ICM = io.Fonts->AddFontFromMemoryTTF((void*)icomenu, sizeof(icomenu), 25.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		fonts::Iconss = io.Fonts->AddFontFromMemoryTTF((void*)econs, sizeof(econs), 45.f, &m_config, ranges);
		fonts::Logo = io.Fonts->AddFontFromMemoryTTF((void*)econs, sizeof(econs), 22.f, &m_config, ranges);\
		fonts::MenuL = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 26, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::MoreIcons = io.Fonts->AddFontFromMemoryTTF((void*)Econs2, sizeof(Econs2), 25, &m_config, ranges);
		fonts::Econs = io.Fonts->AddFontFromMemoryTTF((void*)Econs3, sizeof(Econs3), 25, &m_config, ranges);
		fonts::Keybinds = io.Fonts->AddFontFromMemoryTTF((void*)Keybinds, sizeof(Keybinds), 25, &m_config, ranges);
		int old_fl = m_config.RasterizerFlags;
	
		m_config.RasterizerFlags = ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome;
		fonts::SmallPixelWatermark = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 13, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		fonts::AADebug = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 12, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		fonts::WTLG = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 13, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		m_config.RasterizerFlags = old_fl;
		fonts::ESPIcons2 = io.Fonts->AddFontFromMemoryTTF((void*)espiconsimgui, sizeof(espiconsimgui), 23.f, &m_config, ranges);
		// AdobeFanHeitiStdBold
		fonts::EventLog = io.Fonts->AddFontFromMemoryTTF((void*)cryl500, sizeof(cryl500), 18, &m_config, ranges);
		fonts::NewIconsNotify = io.Fonts->AddFontFromMemoryTTF((void*)NewIcons, sizeof(NewIcons), 18, &m_config, ranges);
		fonts::CloudIndi = io.Fonts->AddFontFromMemoryTTF((void*)NewIcons, sizeof(NewIcons), 24, &m_config, ranges);
		fonts::NewIconsNotify2 = io.Fonts->AddFontFromMemoryTTF((void*)NewIcons, sizeof(NewIcons), 30, &m_config, ranges);
		fonts::NotifyBold = io.Fonts->AddFontFromMemoryTTF((void*)BoldNotify, sizeof(BoldNotify), 18, &m_config, ranges);
		fonts::MenuIcons = io.Fonts->AddFontFromMemoryTTF((void*)WarningAndQuestion, sizeof(WarningAndQuestion), 20, &m_config, ranges);
		fonts::NotifyMore = io.Fonts->AddFontFromMemoryTTF((void*)WarningAndQuestion, sizeof(WarningAndQuestion), 30, &m_config, ranges);
		ImGuiFreeType::BuildFontAtlas(io.Fonts, 0x00

		/*ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome*/);


		vars.menu.guiinited = true;
	}
}

LPDIRECT3DTEXTURE9 img = nullptr;
LPD3DXSPRITE sprite = nullptr;

string UnixTimeToExpiryDate(long long unixTime) {
	string s, m, h;
	if (unixTime < 0) {
		csgo->DoUnload = true;
		return string();
	}

	s = std::to_string(unixTime % 60);
	unixTime /= 60;
	m = std::to_string(unixTime % 60);
	unixTime /= 60;
	h = std::to_string(unixTime % 24);
	unixTime /= 24;

	if (atoi(h.c_str()) > 1000)
		return "Never :)";

	return std::to_string(unixTime)
		+ "d:" + ("0" + h).substr(h.length() - 1)
		+ "h:" + ("0" + m).substr(m.length() - 1)
		+ "m:" + ("0" + s).substr(s.length() - 1)
		+ "s";
}
static HRESULT __stdcall Hooked_Present(IDirect3DDevice9Ex* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{
	//H::SteamPresent = g_pD3DX9->GetOriginal<Present_t>(17);
	IDirect3DStateBlock9* stateBlock = nullptr;
	IDirect3DVertexDeclaration9* vertDec = nullptr;

	device->GetVertexDeclaration(&vertDec);

	if (device->CreateStateBlock(D3DSBT_PIXELSTATE, &stateBlock))
		return H::SteamPresent(device, src, dest, windowOverride, dirtyRegion);

	GUI_Init(device);
	csgo->render_device = device;
	g_Render->SetupPresent(device);
	static void* dwReturnAddress = _ReturnAddress();

	if (dwReturnAddress == _ReturnAddress())
	{
		g_Render->PreRender(device);
		if (interfaces.engine->IsInGame() && interfaces.engine->IsConnected() && csgo->local)
		{
			if (csgo->local->isAlive()) {
				CGrenadePrediction::Get().Paint(device);
			}
		}
		features->Visuals->DrawLocalShit(device);

		g_Hitmarker->Paint();

		g_Render->PostRender(device);

		g_Menu->draw_indicators();


	//	if (vars.menu.open) {
	//		CDrawModel::Get().Run();
	//	}

		g_Menu->render();

		g_Render->EndPresent(device);
	}

	stateBlock->Apply();
	stateBlock->Release();
	device->SetVertexDeclaration(vertDec);

	return H::SteamPresent(device, src, dest, windowOverride, dirtyRegion);//return present here
}



static HRESULT D3DAPI Hooked_Reset(IDirect3DDevice9Ex* device, D3DPRESENT_PARAMETERS* params) noexcept
{
	//H::SteamReset = g_pD3DX9->GetOriginal<Reset_t>(16);

	if (csgo->DoUnload)
		return H::SteamReset(device, params);

	g_Render->InvalidateObjects();

	auto hr = H::SteamReset(device, params);

	if (hr >= 0)
		g_Render->CreateObjects(device);

	return hr;
}