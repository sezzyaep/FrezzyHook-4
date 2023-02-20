#include "render.h"
#include "Hooks.h"

ImGuiRendering* g_Render = new ImGuiRendering();

void __stdcall ImGuiRendering::CreateObjects(IDirect3DDevice9* pDevice)
{
	//if (csgo->Init.Window)
	ImGui_ImplDX9_CreateDeviceObjects();
	_drawList = ImGui::GetBackgroundDrawList();
}

void __stdcall ImGuiRendering::InvalidateObjects()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	_drawList = nullptr;
}

void __stdcall ImGuiRendering::SetupPresent(IDirect3DDevice9* device)
{
	_drawList = ImGui::GetBackgroundDrawList();
}

void __stdcall ImGuiRendering::EndPresent(IDirect3DDevice9* device)
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void __stdcall ImGuiRendering::PreRender(IDirect3DDevice9* device)
{
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->SetRenderState(D3DRS_LIGHTING, false);
	device->SetRenderState(D3DRS_FOGENABLE, false);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	device->SetRenderState(D3DRS_STENCILENABLE, false);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

	device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	m_pDevice = device;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	_drawData.Valid = false;
	_drawList->Clear();
	_drawList->PushClipRectFullScreen();
}

void __stdcall ImGuiRendering::PostRender(IDirect3DDevice9* device)
{
	if (!_drawList->VtxBuffer.empty()) {
		_drawData.Valid = true;
		_drawData.CmdLists = &_drawList;
		_drawData.CmdListsCount = 1;
		_drawData.TotalVtxCount = _drawList->VtxBuffer.Size;
		_drawData.TotalIdxCount = _drawList->IdxBuffer.Size;
	}

	ImGui_ImplDX9_RenderDrawData(&_drawData);
}

#define ZREZ Vector{0.f,0.f,0.f}
#define ZRE Vector{0.f,0.f}

void ImGuiRendering::DrawGradient3DCircle(const Vector origin, float radius, color_t color, color_t color2)
{
	
}

void ImGuiRendering::DrawString(float x, float y, color_t color, int flags, ImFont* font, const char* message, ...)
{
	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message, args);
	va_end(args);


	_drawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);
	auto coord = ImVec2(x, y);
	auto size = ImGui::CalcTextSize(output);
	auto coord_out = ImVec2{ coord.x + 1.f, coord.y + 1.f };
	color_t outline_clr = color_t(0, 0, 0, color.get_alpha());

	int width = 0, height = 0;
	
	if (!(flags & render::centered_x))
		size.x = 0;
	if (!(flags & render::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(coord.x - (size.x * .5), coord.y - (size.y * .5));

	if (flags & render::outline)
	{
		pos.y++;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x++;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.y--;
		_drawList->AddText(pos, outline_clr.u32(), output);
		pos.x--;
		_drawList->AddText(pos, outline_clr.u32(), output);
	}
	
	_drawList->AddText(pos, color.u32(), output);
	ImGui::PopFont();
}
void ImGuiRendering::DrawBlur(color_t Color, float Scale) {
	return;
	BlurProgram::instance().UpdateScreenSize(csgo->w, csgo->h);
	BlurProgram::instance().SetDevice(m_pDevice);
	BlurProgram::instance().SetupTextures(Scale);
	BlurProgram::instance().SetupShaders();

	if (!BlurProgram::instance().BluredTexture_One || !BlurProgram::instance().BluredTexture_Two)
		return;

	_drawList->AddCallback(&begin, nullptr);
	{
		_drawList->AddCallback(&first, nullptr);
		_drawList->AddImage(reinterpret_cast<ImTextureID>(BlurProgram::instance().BluredTexture_One), { -1.0f, -1.0f }, { 1.0f, 1.0f });
		_drawList->AddCallback(&second, nullptr);
		_drawList->AddImage(reinterpret_cast<ImTextureID>(BlurProgram::instance().BluredTexture_Two), { -1.0f, -1.0f }, { 1.0f, 1.0f });
	}
	_drawList->AddCallback(&end, nullptr);
	_drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	_drawList->AddImage(reinterpret_cast<ImTextureID>(BlurProgram::instance().BluredTexture_One), { 0.0f, 0.0f }, { (float)BlurProgram::instance().BufferWidth, (float)BlurProgram::instance().BufferHeight }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, Color.u32());
}
void ImGuiRendering::DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, color_t clr, float width)
{
	ImVec2 min = ImVec2(leftUpCorn.x, leftUpCorn.y);
	ImVec2 max = ImVec2(rightDownCorn.x, rightDownCorn.y);
	_drawList->AddRect(min, max, clr.u32(), 0.0F, -1, width);
}

void ImGuiRendering::DrawLine(float x1, float y1, float x2, float y2, color_t clr, float thickness)
{
	_drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), clr.u32(), thickness);
}

void ImGuiRendering::DrawLineGradient(float x1, float y1, float x2, float y2, color_t clr1, color_t cl2, float thickness)
{
	_drawList->AddRectFilledMulticolor_t(ImVec2(x1, y2), ImVec2(x2 + thickness, y2 + thickness),
		clr1.u32(), cl2.u32(), cl2.u32(), clr1.u32());
}

void ImGuiRendering::DrawSkeetLine(float x1, float y1, float x2, float y2, color_t clr,float thickness)
{
	color_t clr2 = color_t(clr[0], clr[1], clr[2], 0);
	_drawList->AddRectFilledMulticolor_t(ImVec2(x1, y2), ImVec2(x2 + thickness / 2, y2 + thickness / 2),
		clr2.u32(), clr2.u32(), clr.u32(), clr.u32());

	_drawList->AddRectFilledMulticolor_t(ImVec2(x1, y2), ImVec2(x2 + thickness / 2, y2 + thickness / 2),
		clr.u32(), clr.u32(), clr2.u32(), clr2.u32());
}

void ImGuiRendering::Rect(float x, float y, float w, float h, color_t clr, float rounding)
{
	_drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), clr.u32(), rounding);
}

void ImGuiRendering::Arc(float X, float Y, float radius, float Angle1, float Angle2, color_t color)
{
	_drawList->PathArcTo(ImVec2(X, Y), radius, Angle1, Angle2, 230);
	_drawList->PathStroke(color.u32(), false, 2.5f);
}

void ImGuiRendering::PArc(float X, float Y, float radius, float Angle1, float Angle2, float Thickness, color_t color)
{
	_drawList->PathArcTo(ImVec2(X, Y), radius, Angle1, Angle2, 130);
	_drawList->PathStroke(color.u32(), false, Thickness);
}

color_t blend(color_t col, color_t col2, float ammount)
{
	float R = (float)col.get_red() * ammount + (float)col.get_red() * (1.f - ammount);
	float G = (float)col.get_green() * ammount + (float)col.get_green() * (1.f - ammount);
	float B = (float)col.get_blue() * ammount + (float)col.get_blue() * (1.f - ammount);
	float A = (float)col.get_alpha() * ammount + (float)col.get_alpha() * (1.f - ammount);
	return color_t(R, G, B);
}

void ImGuiRendering::CircleFilledRadialGradient(float x1, float y1, float radius, color_t col, float dropoff, float differencial,int segments)
{
	for (float i = 0; i < radius; i += dropoff)
	{
		float r = std::clamp(i, 0.f, radius);
		color_t color = color_t(col.get_red(), col.get_green(), col.get_blue(), differencial);
		CircleFilled(x1, y1, r, color, segments);
	}
}

void ImGuiRendering::FilledRect(float x, float y, float w, float h, color_t clr, float rounding)
{
	_drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), clr.u32(), rounding);
}
void ImGuiRendering::filled_rect_gradient(float x, float y, float w, float h, color_t col_upr_left,
	color_t col_upr_right, color_t col_bot_right, color_t col_bot_left)
{
	_drawList->AddRectFilledMulticolor_t(ImVec2(x, y), ImVec2(x + w, y + h),
		col_upr_left.u32(), col_upr_right.u32(), col_bot_right.u32(), col_bot_left.u32());
}
void ImGuiRendering::Triangle(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr, float thickness)
{
	_drawList->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32(), thickness);
}

void ImGuiRendering::TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr)
{
	_drawList->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32());
}

void ImGuiRendering::CircleFilled(float x1, float y1, float radius, color_t col, int segments)
{
	_drawList->AddCircleFilled(ImVec2(x1, y1), radius, col.u32(), segments);
}

void ImGuiRendering::DrawWave(Vector loc, float radius, color_t color, float thickness)
{
	static float Step = PI * 3.0f / 40;
	Vector prev;
	for (float lat = 0; lat <= PI * 3.0f; lat += Step)
	{
		float sin1 = sin(lat);
		float cos1 = cos(lat);
		float sin3 = sin(0.0f);
		float cos3 = cos(0.0f);

		Vector point1;
		point1 = Vector(sin1 * cos3, cos1, sin1 * sin3) * radius;
		Vector point3 = loc;
		Vector Out;
		point3 += point1;

		if (Math::WorldToScreen(point3, Out))
		{
			if (lat > 0.000f)
				DrawLine(prev.x, prev.y, Out.x, Out.y, color, thickness);
		}
		prev = Out;
	}
}
color_t hsb_to_rgbcol(float _h, float s, float b, int alpha)
{
	float h = _h == 1.0f ? 0 : _h * 6.0f;
	float f = h - (int)h;
	float p = b * (1.0f - s);
	float q = b * (1.0f - s * f);
	float t = b * (1.0f - (s * (1.0f - f)));

	if (h < 1)
	{
		return color_t(
			(unsigned char)(b * 255),
			(unsigned char)(t * 255),
			(unsigned char)(p * 255)
		);
	}
	if (h < 2)
	{
		return color_t(
			(unsigned char)(q * 255),
			(unsigned char)(b * 255),
			(unsigned char)(p * 255)
		);
	}
	if (h < 3)
	{
		return color_t(
			(unsigned char)(p * 255),
			(unsigned char)(b * 255),
			(unsigned char)(t * 255)
		);
	}
	if (h < 4)
	{
		return color_t(
			(unsigned char)(p * 255),
			(unsigned char)(q * 255),
			(unsigned char)(b * 255)
		);
	}
	if (h < 5)
	{
		return color_t(
			(unsigned char)(t * 255),
			(unsigned char)(p * 255),
			(unsigned char)(b * 255)
		);
	}
	return color_t(
		(unsigned char)(b * 255),
		(unsigned char)(p * 255),
		(unsigned char)(q * 255)
	);

}

void ImGuiRendering::circle_filled_rainbow(IDirect3DDevice9* dev, Vector2D center, float radius, bool rotate, float speed) 
{
	static float base_hue = 0.f;
	if (rotate)
	{
		if (base_hue >= 1.f)
			base_hue = 0.f;

		base_hue += interfaces.global_vars->frametime * speed;
	}

	vertex vert[128 + 2] = {};

	float new_hue = base_hue;
	for (auto i = 1; i <= 128; i++)
	{
		if (new_hue >= 1.f)
			new_hue = 0.f;

		new_hue += 1.f / 128.f;
		new_hue = std::clamp(new_hue, 0.f, 1.f);

		auto temp = hsb_to_rgbcol(new_hue, 1.f, 1.f, 1.f);
		auto col = D3DCOLOR_RGBA(temp.get_red(), temp.get_green(), temp.get_blue(), (int)vars.visuals.innacuracyoverlay_alpha);

		vert[i] =
		{
			center.x + radius * cos(D3DX_PI * (i / (128 / 2.0f))),
			center.y + radius * sin(D3DX_PI * (i / (128 / 2.0f))),
			0.0f,
			1.0f,
			col
		};
	}

	vert[0] = { center.x, center.y, 0.0f, 1.0f, D3DCOLOR_RGBA(100, 100, 100, 10) };
	vert[128 + 1] = vert[1];

	dev->SetTexture(0, NULL);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 128, &vert, sizeof(vertex));
}

void ImGuiRendering::build_lookup_table()
{
	if (!lookup_table.empty())
		return;

	for (auto i = 0; i <= points; i++)
		lookup_table.emplace_back(
			std::cos(2.f * D3DX_PI * (i / static_cast<float>(points))),
			std::sin(2.f * D3DX_PI * (i / static_cast<float>(points)))
		);
}

void ImGuiRendering::circle_filled_radial_gradient(IDirect3DDevice9* dev, Vector2D center, float radius, color_t color1, color_t color2)
{
	const auto col1 = color1.u32();
	const auto col2 = color2.u32();
	build_lookup_table();

	vertex vert[points + 2] = {};

	for (auto i = 1; i <= points; i++)
		vert[i] =
	{
		center.x + radius * lookup_table[i].x,
		center.y - radius * lookup_table[i].y,
		0.0f,
		1.0f,
		col1
	};

	vert[0] = { center.x, center.y, 0.0f, 1.0f, col2 };
	vert[points + 1] = vert[1];

	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);
	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, &vert, sizeof vertex);
	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);
}

void ImGuiRendering::Render3DCircle(IDirect3DDevice9* dev, Vector center, float radius, color_t color) {
	const auto col1 = color_t(color[2], color[1], color[0], color[3]).u32();
	build_lookup_table();





	vertex vert[points + 2] = {};
	bool render = false;

	for (auto i = 1; i <= points; i++)
	{
		Vector Point3D = Vector(lookup_table[i].y, lookup_table[i].x, 0.f) * radius;
		Vector Point2D;
		if (Math::WorldToScreen(center + Point3D, Point2D)) {
			render = true;
		}
		vert[i] =
		{
			Point2D.x,
			Point2D.y,
			0.0f,
			1.0f,
			col1
		};
	}
	if (!render)
		return;
	Vector Point2D;
	Math::WorldToScreen(center, Point2D);

	vert[0] = { Point2D.x, Point2D.y, 0.0f, 1.0f, col1 };
	vert[points + 1] = vert[1];

	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);
	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, &vert, sizeof vertex);
	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);
}

void ImGuiRendering::Render3DCyTri(IDirect3DDevice9* dev, Vector center, float hieght, float radius, color_t color1, color_t color2) {
	const auto col1 = color_t(color1[2], color1[1], color1[0], color1[3]).u32();
	const auto col2 = color_t(color2[2], color2[1], color2[0], color2[3]).u32();
	build_lookup_table();





	vertex vert[points + 2] = {};
	bool render = false;

	for (auto i = 1; i <= points; i++)
	{
		trace_t WPoint;
		Vector Point3D = Vector(lookup_table[i].y, lookup_table[i].x, 0.f) * radius;
		Vector Point2D;
		if (Math::WorldToScreen(center + Point3D, Point2D)) {
			render = true;
		}
		vert[i] =
		{
			Point2D.x,
			Point2D.y,
			0.0f,
			1.0f,
			col1
		};
	}
	if (!render)
		return;
	Vector Point2D;
	Math::WorldToScreen(center - Vector(0,0,hieght), Point2D);

	vert[0] = { Point2D.x, Point2D.y, 0.0f, 1.0f, col2 };
	vert[points + 1] = vert[1];

	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);
	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, &vert, sizeof vertex);
	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);
	/*
	std::vector<Vector2D> Matrix;


	for (auto i = 0; i <= points; i++)
		Matrix.emplace_back(
			std::cos(2.f * D3DX_PI * (i / static_cast<float>(points))),
			std::sin(2.f * D3DX_PI * (i / static_cast<float>(points)))
		);*/
}

void ImGuiRendering::Gradient3DCircleOp(IDirect3DDevice9* dev, Vector center, float radius, color_t color1, color_t color2) {
	const auto col1 = color_t(color1[2],color1[1], color1[0], color1[3]).u32();
	const auto col2 = color_t(color2[2], color2[1], color2[0], color2[3]).u32();
	build_lookup_table();





	vertex vert[points + 2] = {};
	bool render = false;

	for (auto i = 1; i <= points; i++)
	{
		Vector Point3D = Vector(lookup_table[i].y, lookup_table[i].x, 0.f) * radius;
		Vector Point2D;
		if (Math::WorldToScreen(center + Point3D, Point2D)) {
			render = true;
		}
		vert[i] =
		{
			Point2D.x,
			Point2D.y,
			0.0f,
			1.0f,
			col1
		};
	}
	if (!render)
		return;
	Vector Point2D;
	Math::WorldToScreen(center, Point2D);
	
	vert[0] = { Point2D.x, Point2D.y, 0.0f, 1.0f, col2 };
	vert[points + 1] = vert[1];

	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);
	dev->SetTexture(0, nullptr);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, &vert, sizeof vertex);
	dev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);
	/*
	std::vector<Vector2D> Matrix;


	for (auto i = 0; i <= points; i++)
		Matrix.emplace_back(
			std::cos(2.f * D3DX_PI * (i / static_cast<float>(points))),
			std::sin(2.f * D3DX_PI * (i / static_cast<float>(points)))
		);*/
}