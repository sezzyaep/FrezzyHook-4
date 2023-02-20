#pragma once
#include <string>
#include <time.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <vector>
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_dx9.h"
#include "ImGui\imgui_impl_win32.h"
#include "imgui\imgui_freetype.h"
#include <vector>
#include "Vector.h"
#include "colors.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

struct Vertex_t;
class Vector;
class Vector2D;

struct vertex
{
	float x, y, z, rhw;
	uint32_t color;
};

namespace fonts
{
	extern unsigned long esp_name;
	extern unsigned long esp_info;
	extern unsigned long esp_icon;
	extern unsigned long esp_logs;
	extern ImFont* SmallPixelWatermark;
	extern ImFont* lby_indicator;
	extern ImFont* Econs;
	extern ImFont* menu_main;
	extern ImFont* Watermark2;
	extern ImFont* Weapons;
	extern ImFont* menu_desc;
	extern ImFont* Logo;
	extern ImFont* MenuL;
	extern ImFont* Keybinds_;
	extern ImFont* MoreIcons;
	extern ImFont* Iconss;
	extern ImFont* ICM;
	extern ImFont* Watermark;
	extern ImFont* Keybinds;
	extern ImFont* icon;
	extern ImFont* NonBoldIcon;
	extern ImFont* WTLG;
	extern ImFont* AADebug;
	extern ImFont* EventLog;
	extern ImFont* ESPIcons2;
	extern ImFont* NewIconsNotify;
	extern ImFont* NewIconsNotify2;
	extern ImFont* NotifyBold;
	extern ImFont* CloudIndi;
	extern ImFont* MenuIcons;
	extern ImFont* NotifyMore;
}
namespace render {
	enum e_textflags {
		none = 0,
		outline = 1 << 0,
		centered_x = 1 << 1,
		centered_y = 1 << 2,
	};
}

const BYTE blur_x[] =
{
	  0,   2, 255, 255, 254, 255,
	 44,   0,  67,  84,  65,  66,
	 28,   0,   0,   0, 131,   0,
	  0,   0,   0,   2, 255, 255,
	  2,   0,   0,   0,  28,   0,
	  0,   0,   0,   1,   0,   0,
	124,   0,   0,   0,  68,   0,
	  0,   0,   3,   0,   0,   0,
	  1,   0,   0,   0,  80,   0,
	  0,   0,   0,   0,   0,   0,
	 96,   0,   0,   0,   2,   0,
	  0,   0,   1,   0,   0,   0,
	108,   0,   0,   0,   0,   0,
	  0,   0, 116, 101, 120,  83,
	 97, 109, 112, 108, 101, 114,
	  0, 171,   4,   0,  12,   0,
	  1,   0,   1,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	116, 101, 120, 101, 108,  87,
	105, 100, 116, 104,   0, 171,
	  0,   0,   3,   0,   1,   0,
	  1,   0,   1,   0,   0,   0,
	  0,   0,   0,   0, 112, 115,
	 95,  50,  95,  48,   0,  77,
	105,  99, 114, 111, 115, 111,
	102, 116,  32,  40,  82,  41,
	 32,  72,  76,  83,  76,  32,
	 83, 104,  97, 100, 101, 114,
	 32,  67, 111, 109, 112, 105,
	108, 101, 114,  32,  49,  48,
	 46,  49,   0, 171,  81,   0,
	  0,   5,   1,   0,  15, 160,
	 20,  59, 177,  63,  24, 231,
	161,  62, 198, 121, 104,  62,
	236, 196,  78,  64,  81,   0,
	  0,   5,   2,   0,  15, 160,
	220, 233, 143,  61,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,  31,   0,
	  0,   2,   0,   0,   0, 128,
	  0,   0,   3, 176,  31,   0,
	  0,   2,   0,   0,   0, 144,
	  0,   8,  15, 160,   1,   0,
	  0,   2,   0,   0,   9, 128,
	  1,   0, 228, 160,   4,   0,
	  0,   4,   1,   0,   1, 128,
	  0,   0,   0, 160,   0,   0,
	  0, 128,   0,   0,   0, 176,
	  1,   0,   0,   2,   1,   0,
	  2, 128,   0,   0,  85, 176,
	  4,   0,   0,   4,   0,   0,
	  1, 128,   0,   0,   0, 160,
	  0,   0,   0, 129,   0,   0,
	  0, 176,   1,   0,   0,   2,
	  0,   0,   2, 128,   0,   0,
	 85, 176,   4,   0,   0,   4,
	  2,   0,   1, 128,   0,   0,
	  0, 160,   0,   0, 255, 129,
	  0,   0,   0, 176,   1,   0,
	  0,   2,   2,   0,   2, 128,
	  0,   0,  85, 176,   4,   0,
	  0,   4,   3,   0,   1, 128,
	  0,   0,   0, 160,   0,   0,
	255, 128,   0,   0,   0, 176,
	  1,   0,   0,   2,   3,   0,
	  2, 128,   0,   0,  85, 176,
	 66,   0,   0,   3,   1,   0,
	 15, 128,   1,   0, 228, 128,
	  0,   8, 228, 160,  66,   0,
	  0,   3,   0,   0,  15, 128,
	  0,   0, 228, 128,   0,   8,
	228, 160,  66,   0,   0,   3,
	  4,   0,  15, 128,   0,   0,
	228, 176,   0,   8, 228, 160,
	 66,   0,   0,   3,   2,   0,
	 15, 128,   2,   0, 228, 128,
	  0,   8, 228, 160,  66,   0,
	  0,   3,   3,   0,  15, 128,
	  3,   0, 228, 128,   0,   8,
	228, 160,   5,   0,   0,   3,
	  0,   0,   7, 128,   0,   0,
	228, 128,   1,   0,  85, 160,
	  4,   0,   0,   4,   0,   0,
	  7, 128,   4,   0, 228, 128,
	  1,   0, 170, 160,   0,   0,
	228, 128,   4,   0,   0,   4,
	  0,   0,   7, 128,   1,   0,
	228, 128,   1,   0,  85, 160,
	  0,   0, 228, 128,   4,   0,
	  0,   4,   0,   0,   7, 128,
	  2,   0, 228, 128,   2,   0,
	  0, 160,   0,   0, 228, 128,
	  4,   0,   0,   4,   4,   0,
	  7, 128,   3,   0, 228, 128,
	  2,   0,   0, 160,   0,   0,
	228, 128,   1,   0,   0,   2,
	  0,   8,  15, 128,   4,   0,
	228, 128, 255, 255,   0,   0
};

const BYTE blur_y[] =
{
	  0,   2, 255, 255, 254, 255,
	 44,   0,  67,  84,  65,  66,
	 28,   0,   0,   0, 131,   0,
	  0,   0,   0,   2, 255, 255,
	  2,   0,   0,   0,  28,   0,
	  0,   0,   0,   1,   0,   0,
	124,   0,   0,   0,  68,   0,
	  0,   0,   3,   0,   0,   0,
	  1,   0,   0,   0,  80,   0,
	  0,   0,   0,   0,   0,   0,
	 96,   0,   0,   0,   2,   0,
	  0,   0,   1,   0,   0,   0,
	108,   0,   0,   0,   0,   0,
	  0,   0, 116, 101, 120,  83,
	 97, 109, 112, 108, 101, 114,
	  0, 171,   4,   0,  12,   0,
	  1,   0,   1,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	116, 101, 120, 101, 108,  72,
	101, 105, 103, 104, 116,   0,
	  0,   0,   3,   0,   1,   0,
	  1,   0,   1,   0,   0,   0,
	  0,   0,   0,   0, 112, 115,
	 95,  50,  95,  48,   0,  77,
	105,  99, 114, 111, 115, 111,
	102, 116,  32,  40,  82,  41,
	 32,  72,  76,  83,  76,  32,
	 83, 104,  97, 100, 101, 114,
	 32,  67, 111, 109, 112, 105,
	108, 101, 114,  32,  49,  48,
	 46,  49,   0, 171,  81,   0,
	  0,   5,   1,   0,  15, 160,
	 20,  59, 177,  63,  24, 231,
	161,  62, 198, 121, 104,  62,
	236, 196,  78,  64,  81,   0,
	  0,   5,   2,   0,  15, 160,
	220, 233, 143,  61,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,  31,   0,
	  0,   2,   0,   0,   0, 128,
	  0,   0,   3, 176,  31,   0,
	  0,   2,   0,   0,   0, 144,
	  0,   8,  15, 160,   1,   0,
	  0,   2,   0,   0,   9, 128,
	  1,   0, 228, 160,   4,   0,
	  0,   4,   1,   0,   2, 128,
	  0,   0,   0, 160,   0,   0,
	  0, 128,   0,   0,  85, 176,
	  1,   0,   0,   2,   1,   0,
	  1, 128,   0,   0,   0, 176,
	  4,   0,   0,   4,   0,   0,
	  2, 128,   0,   0,   0, 160,
	  0,   0,   0, 129,   0,   0,
	 85, 176,   1,   0,   0,   2,
	  0,   0,   1, 128,   0,   0,
	  0, 176,   4,   0,   0,   4,
	  2,   0,   2, 128,   0,   0,
	  0, 160,   0,   0, 255, 129,
	  0,   0,  85, 176,   1,   0,
	  0,   2,   2,   0,   1, 128,
	  0,   0,   0, 176,   4,   0,
	  0,   4,   3,   0,   2, 128,
	  0,   0,   0, 160,   0,   0,
	255, 128,   0,   0,  85, 176,
	  1,   0,   0,   2,   3,   0,
	  1, 128,   0,   0,   0, 176,
	 66,   0,   0,   3,   1,   0,
	 15, 128,   1,   0, 228, 128,
	  0,   8, 228, 160,  66,   0,
	  0,   3,   0,   0,  15, 128,
	  0,   0, 228, 128,   0,   8,
	228, 160,  66,   0,   0,   3,
	  4,   0,  15, 128,   0,   0,
	228, 176,   0,   8, 228, 160,
	 66,   0,   0,   3,   2,   0,
	 15, 128,   2,   0, 228, 128,
	  0,   8, 228, 160,  66,   0,
	  0,   3,   3,   0,  15, 128,
	  3,   0, 228, 128,   0,   8,
	228, 160,   5,   0,   0,   3,
	  0,   0,   7, 128,   0,   0,
	228, 128,   1,   0,  85, 160,
	  4,   0,   0,   4,   0,   0,
	  7, 128,   4,   0, 228, 128,
	  1,   0, 170, 160,   0,   0,
	228, 128,   4,   0,   0,   4,
	  0,   0,   7, 128,   1,   0,
	228, 128,   1,   0,  85, 160,
	  0,   0, 228, 128,   4,   0,
	  0,   4,   0,   0,   7, 128,
	  2,   0, 228, 128,   2,   0,
	  0, 160,   0,   0, 228, 128,
	  4,   0,   0,   4,   4,   0,
	  7, 128,   3,   0, 228, 128,
	  2,   0,   0, 160,   0,   0,
	228, 128,   1,   0,   0,   2,
	  0,   8,  15, 128,   4,   0,
	228, 128, 255, 255,   0,   0
};

class ShaderProgram
{
public:
	void SetDevice(IDirect3DDevice9* pDevice) {
		device = pDevice;
	}
	void Use(float uniform, int location)
	{
		device->SetPixelShader(pixelShader);
		const float params[4] = { uniform };
		device->SetPixelShaderConstantF(location, params, 1);
	}

	void Initialize(const BYTE* pixelShaderSrc)
	{
		if (initialized)
			return;
		initialized = true;

		device->CreatePixelShader(reinterpret_cast<const DWORD*>(pixelShaderSrc), &pixelShader);
	}

private:
	IDirect3DDevice9* device;
	IDirect3DPixelShader9* pixelShader;
	bool initialized = false;
};

class BlurProgram {
public:
	static BlurProgram& instance()
	{
		static BlurProgram blurEffect;
		return blurEffect;
	}
	void SetDevice(IDirect3DDevice9* pDevice) {
		m_pDevice = pDevice;
		BlurShaderX.SetDevice(pDevice);
		BlurShaderY.SetDevice(pDevice);
	}
	IDirect3DTexture9* CreateTexture(int width, int height)
	{
		IDirect3DTexture9* texture;
		m_pDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr);
		return texture;
	}

	void CopyBackgroundToTexture(IDirect3DTexture9* texture, D3DTEXTUREFILTERTYPE filtering)
	{
		if (IDirect3DSurface9* backBuffer; m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) == D3D_OK)
		{
			if (IDirect3DSurface9* surface; texture->GetSurfaceLevel(0, &surface) == D3D_OK)
				m_pDevice->StretchRect(backBuffer, nullptr, surface, nullptr, filtering);
		}
	}
	void SetRenderTarget(IDirect3DTexture9* rtTexture)
	{
		if (IDirect3DSurface9* surface; rtTexture->GetSurfaceLevel(0, &surface) == D3D_OK)
			m_pDevice->SetRenderTarget(0, surface);
	}
	void SetupTextures(float scale)
	{
		if (Scale != scale) {
			Scale = scale;
			ClearTextures();
		}

		if (!BluredTexture_One)
			BluredTexture_One = CreateTexture(BufferWidth / Scale, BufferHeight / Scale);
		if (!BluredTexture_Two)
			BluredTexture_Two = CreateTexture(BufferWidth / Scale, BufferHeight / Scale);
	}
	void ClearTextures() {
		if (BluredTexture_One) {
			BluredTexture_One->Release();
			BluredTexture_One = nullptr;
		}
		if (BluredTexture_Two) {
			BluredTexture_Two->Release();
			BluredTexture_Two = nullptr;
		}
	}
	void SetupShaders()
	{
		BlurShaderX.Initialize(blur_x);
		BlurShaderY.Initialize(blur_y);
	}

	void UpdateScreenSize(int w, int y) {
		if (BufferWidth != w || BufferHeight != y) {
			ClearTextures();
			BufferWidth = static_cast<int>(w);
			BufferHeight = static_cast<int>(y);
		}
	}
	void Second()
	{
		BlurShaderY.Use(1.0f / (BufferHeight / Scale), 0);
		SetRenderTarget(BluredTexture_One);
	}

	void End()
	{
		m_pDevice->SetRenderTarget(0, Backup);
		Backup->Release();

		m_pDevice->SetPixelShader(nullptr);
		//	m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	}
	void First()
	{
		BlurShaderX.Use(1.0f / (BufferWidth / Scale), 0);
		SetRenderTarget(BluredTexture_Two);
	}
	void Begin()
	{
		m_pDevice->GetRenderTarget(0, &Backup);

		CopyBackgroundToTexture(BluredTexture_One, D3DTEXF_LINEAR);

		//	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		//	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		//	m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		const D3DMATRIX projection
		{ {{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f / (BufferWidth / Scale), 1.0f / (BufferHeight / Scale), 0.0f, 1.0f
		}} };
		m_pDevice->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
	}
	float Scale = 8.f;
	int BufferWidth = 0;
	int BufferHeight = 0;
	IDirect3DSurface9* Backup = nullptr;
	IDirect3DTexture9* BluredTexture_One = nullptr;
	IDirect3DTexture9* BluredTexture_Two = nullptr;
	IDirect3DDevice9* m_pDevice;
	ShaderProgram BlurShaderX;
	ShaderProgram BlurShaderY;

};
class ImGuiRendering {
public:


	void __stdcall CreateObjects(IDirect3DDevice9* pDevice);
	void __stdcall InvalidateObjects();
	void __stdcall PreRender(IDirect3DDevice9* device);
	void __stdcall PostRender(IDirect3DDevice9* deivce);
	void __stdcall EndPresent(IDirect3DDevice9* device);
	void __stdcall SetupPresent(IDirect3DDevice9* device);

	void DrawBlur(color_t Color, float Scale);
	void DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, color_t clr, float width);
	void DrawLine(float x1, float y1, float x2, float y2, color_t clr, float thickness = 1.f);
	void DrawLineGradient(float x1, float y1, float x2, float y2, color_t clr1, color_t cl2, float thickness = 1.f);
	void DrawSkeetLine(float x1, float y1, float x2, float y2, color_t clr,float thickness = 1.f);
	void Rect(float x, float y, float w, float h, color_t clr, float rounding = 0.f);
	void FilledRect(float x, float y, float w, float h, color_t clr, float rounding = 0.f);
	void Arc(float X, float Y, float radius, float Angle1, float Angle2, color_t color);
	void PArc(float X, float Y, float radius, float Angle1, float Angle2, float Thickness, color_t color);
	void Triangle(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr, float thickness = 1.f);
	void TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr);
	void CircleFilled(float x1, float y1, float radius, color_t col, int segments);
	void CircleFilledRadialGradient(float x1, float y1, float radius, color_t col, float dropoff, float differencial, int segments);
	void DrawWave(Vector loc, float radius, color_t clr, float thickness = 1.f);
	void DrawString(float x, float y, color_t color, int flags, ImFont* font, const char* message, ...);
	void DrawGradient3DCircle(const Vector origin, float radius, color_t color, color_t color2);
	void circle_filled_radial_gradient(IDirect3DDevice9* dev, Vector2D center, float radius, color_t color1, color_t color2);
	void Gradient3DCircleOp(IDirect3DDevice9* dev, Vector center, float radius, color_t color1, color_t color2);
	void Render3DCircle(IDirect3DDevice9* dev, Vector center, float radius, color_t color);
	void Render3DCyTri(IDirect3DDevice9* dev, Vector center, float hieght, float radius, color_t color1, color_t color2);
	void circle_filled_rainbow(IDirect3DDevice9* dev, Vector2D center, float radius, bool rotate, float speed);
	void filled_rect_gradient(float x, float y, float w, float h, color_t col_upr_left,
		color_t col_upr_right, color_t col_bot_right, color_t col_bot_left);
	static void begin(const ImDrawList*, const ImDrawCmd*) { BlurProgram::instance().Begin(); }
	static void first(const ImDrawList*, const ImDrawCmd*) { BlurProgram::instance().First(); }
	static void second(const ImDrawList*, const ImDrawCmd*) { BlurProgram::instance().Second(); }
	static void end(const ImDrawList*, const ImDrawCmd*) { BlurProgram::instance().End(); }
	void build_lookup_table();
	IDirect3DDevice9* GetDevice() {
		return m_pDevice;
	}
	ImDrawList* _drawList;
private:
	IDirect3DDevice9* m_pDevice;
	static constexpr auto points = 64;
	std::vector<Vector2D> lookup_table;
	ImDrawData _drawData;

	IDirect3DTexture9* _texture;
	ImFontAtlas _fonts;
	DWORD dwOld_D3DRS_COLORWRITEENABLE;
}; extern ImGuiRendering* g_Render;

namespace Drawing
{
	extern void CreateFonts();
	extern void LimitDrawingArea(int x, int y, int w, int h);
	extern void GetDrawingArea(int& x, int& y, int& w, int& h);
	extern void DrawString(unsigned long font, int x, int y, color_t color, DWORD alignment, const char* msg, ...);
	extern void DrawStringFont(unsigned long font, int x, int y, color_t clrcolor_t, bool bCenter, const char* szText, ...);
	extern void DrawStringUnicode(unsigned long font, int x, int y, color_t color, bool bCenter, const wchar_t* msg, ...);
	extern void DrawRect(int x, int y, int w, int h, color_t col);
	extern void Rectangle(float x, float y, float w, float h, float px, color_t col);
	extern void Border(int x, int y, int w, int h, int line, color_t col);
	extern void DrawRectRainbow(int x, int y, int w, int h, float flSpeed, float& flRainbow);
	extern void DrawRectGradientVertical(int x, int y, int w, int h, color_t color1, color_t color2);
	extern void DrawRectGradientHorizontal(int x, int y, int w, int h, color_t color1, color_t color2);
	extern void DrawPixel(int x, int y, color_t col);
	extern void DrawOutlinedRect(int x, int y, int w, int h, color_t col);
	extern void DrawOutlinedCircle(int x, int y, int r, color_t col);
	extern void DrawLine(int x0, int y0, int x1, int y1, color_t col);
	extern void DrawCorner(int iX, int iY, int iWidth, int iHeight, bool bRight, bool bDown, color_t colDraw);
	extern void DrawRoundedBox(int x, int y, int w, int h, int r, int v, color_t col);
	extern void Triangle(Vector ldcorner, Vector rucorner, color_t col);
	extern void DrawPolygon(int count, Vertex_t* Vertexs, color_t color);
	extern void DrawBox(int x, int y, int w, int h, color_t color);
	extern RECT GetViewport();
	extern int	GetStringWidth(unsigned long font, const char* msg, ...);
	extern RECT GetTextSize(unsigned long font, const char* text);
	extern void Draw3DBox(Vector* boxVectors, color_t color);
	extern void rotate_point(Vector2D& point, Vector2D origin, bool clockwise, float angle);
	extern void DrawFilledCircle(int x, int y, int radius, int segments, color_t color);
	extern void TexturedPolygon(int n, std::vector<Vertex_t> vertice, color_t color);
	extern void filled_tilted_triangle(Vector2D position, Vector2D size, Vector2D origin, bool clockwise, float angle, color_t color, bool rotate = true);
	extern void DrawCircle(float x, float y, float r, float s, color_t color);

	extern const char* LastFontName;
}
