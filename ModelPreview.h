#pragma once
#include "Hooks.h"
enum ClearFlags_t
{
	VIEW_CLEAR_COLOR = 0x1,
	VIEW_CLEAR_DEPTH = 0x2,
	VIEW_CLEAR_FULL_TARGET = 0x4,
	VIEW_NO_DRAW = 0x8,
	VIEW_CLEAR_OBEY_STENCIL = 0x10, // Draws a quad allowing stencil test to clear through portals
	VIEW_CLEAR_STENCIL = 0x20,
};
struct PModel
{
	bool m_bShotChams = false;
	bool m_bDrawModel = false;
	float_t m_flModelRotation = -180.0f;
};
class C_DrawModel
{
public:
	virtual void Instance();
	virtual ITexture* GetTexture() { return this->m_PreviewTexture; };
	virtual void SetChamsSettings(int Settings) { this->MaterialIndex = Settings; };
	virtual void SetGlow(int glow) { this->m_iGlow = glow; };
	virtual void SetGlowColor(color_t glow) { this->m_GlowColor = glow; };
private:
	ITexture* m_PreviewTexture = nullptr;
	ITexture* m_CubemapTexture = nullptr;
	C_MergedMDL* m_PreviewModel = nullptr;

	CViewSetup m_ViewSetup = { };
	int MaterialIndex;
	PModel m_model;
	int32_t m_iGlow = 0;
	color_t m_GlowColor = color_t(255, 255, 255, 255);
};

inline C_DrawModel* g_DrawModel = new C_DrawModel();