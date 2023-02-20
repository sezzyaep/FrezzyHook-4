#include "tab.h"
#include "child.h"
#include "../Menu.h"

#define logo_width 0

float animation_speed = 0.f;
void c_window::set_title(string title) {
	this->title = title;
}
void c_window::set_position(Vector2D pos) {
	this->pos = pos;
}
void c_window::set_size(Vector2D size) {
	this->size = size;
}
string c_window::get_title() {
	return this->title;
}
std::vector<c_tab*> c_window::get_tabs() {
	return this->tabs;
}
Vector2D c_window::get_position() {
	return this->pos;
}
Vector2D c_window::get_size() {
	return this->size;
}
void c_window::add_element(c_element* e) {
	this->elements.push_back(e);
}
void c_window::add_tab(c_tab* t) {
	this->tabs.push_back(t);
}
void c_window::set_cursor_position(Vector2D cursor) {
	this->cursor = cursor;
}
Vector2D c_window::get_cursor_position() {
	return this->cursor;
}
void c_window::apply_element(int offset) {
	this->cursor.y += offset;
}
float c_window::get_transparency() {
	transparency = clamp(transparency, 0.f, 100.f);
	return transparency;
}
void c_window::set_transparency(float transparency) {
	transparency = clamp(transparency, 0.f, 100.f);
}
void c_window::increase_transparency(float add) {
	transparency += add;
	transparency = clamp(transparency, 0.f, 100.f);
}
void c_window::decrease_transparency(float subtract)
{
	transparency -= subtract;
	transparency = clamp(transparency, 0.f, 100.f);
}
bool c_window::is_click() {
	return is_pressed;
}
bool c_window::left_click() {
	return is_pressed_left;
}
bool c_window::reset_mouse() {
	is_pressed = false;
	holding = false;
	needs_reset = true;
	return true;
}
void c_window::set_active_tab_index(int tab) {
	this->active_tab_index = tab;
}
int c_window::get_active_tab_index() {
	return this->active_tab_index;
}
bool c_window::is_holding() {
	return holding;
}
bool c_window::get_clicked()
{
	if (hovered()) {
		if (!holding) {
			drag.x = g_mouse.x - pos.x;
			drag.y = g_mouse.y - pos.y;
		}
		return true;
	}
	return false;
}
void c_window::lock_bounds() {
	auto pos = this->get_position();
	auto size = this->get_size();
	auto m = Vector2D(pos.x + size.x, pos.y + size.y);
	ImGui::PushClipRect(ImVec2(pos.x, pos.y), ImVec2(m.x, m.y), true);
}
void c_window::unlock_bounds() {
	ImGui::PopClipRect();
}
bool c_window::hovered() {
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}
bool c_window::key_updated(int key)
{
	return (keystates[key] && !oldstates[key]);
}
bool c_window::update() {
	if (ImGui::GetIO().MouseDownDuration[0] >= 0.f && needs_reset)
		return false;
	else if (needs_reset)
		needs_reset = false;

	is_pressed = ImGui::GetIO().MouseDownDuration[0] == 0.f;
	holding = ImGui::GetIO().MouseDownDuration[0] > 0.f;

	is_pressed_left = ImGui::GetIO().MouseDownDuration[1] == 0.f;
	holding_left = ImGui::GetIO().MouseDownDuration[1] > 0.f;
	if (hovered()) {
		if (b_switch) {
			if (pulsating >= 0.f)
				pulsating += animation_speed / 2.f;
			if (pulsating >= 1.f)
				b_switch = false;
		}
		else {
			if (pulsating <= 1.f)
				pulsating -= animation_speed / 2.f;
			if (pulsating <= 0.f)
				b_switch = true;
		}
	}
	else {
		if (pulsating > 0)
			pulsating -= animation_speed / 2.f;
		b_switch = false;
	}
	pulsating = clamp(pulsating, 0.f, 1.f);
	g_hovered_element = nullptr;
	return false;
}
bool c_window::is_holding_menu()
{
	return holding_menu && is_holding();
}
c_element* c_window::get_parent() {
	return nullptr;
}
void c_window::render_tabs() {
	int t_size = tabs.size();
	Vector2D tab_size = Vector2D(45, 30);
	Vector2D pos_render = Vector2D(pos.x + 70, pos.y + 2);
	auto alpha = (int)(get_transparency() * 2.55f);
	int Zz = 75;
	for (auto& t : tabs) {
		t->draw(pos_render, tab_size, Zz);
		pos_render.x += Zz;
	}
	pos_render.x -= Zz * tabs.size();
	bool g_hovered = !g_active_element && g_mouse.x > pos.x - 30.f && g_mouse.y > pos.y + 20.f && g_mouse.x < pos.x + 100 && g_mouse.y < pos.y + 55.f;
	if (g_hovered) {
		animation228 += animation_speed;
		if (left_click()) {
			active_tab_index = -1;
			//reset_mouse();
		}
	}
	else
		animation228 -= animation_speed;
	animation228 = clamp(animation228, 0.f, 1.f);
}
void c_window::update_animation() {
	if (last_time_updated == -1.f)
		last_time_updated = csgo->get_absolute_time();
	animation_speed = fabsf(last_time_updated - csgo->get_absolute_time()) * 6.f;
	last_time_updated = csgo->get_absolute_time();
}
void c_window::update_keystates() {
	std::copy(keystates, keystates + 255, oldstates);
	for (auto i = 0; i < 255; i++)
		keystates[i] = GetAsyncKeyState(i);
}
void c_window::render() {
	if (transparency <= 0.f)
		return;

	int a = clamp((int)(transparency * 2.55f - 50.f + 25.f * pulsating), 0, 255);
	static bool FinishedLoading = false;
	static int FinishedLoading_T = 0;
	static float Alpha2 = 255;
	auto alpha = (int)(get_transparency() * 2.55f);
	auto draw = g_Render;
	int width, height;
	interfaces.engine->GetScreenSize(width, height);
	//draw->FilledRect(0, 0, 4000, 4000, color_t(10, 10, 10, alpha - (alpha / 5)));//background
	if(vars.menu.outline)
	{ 
		int FAA = alpha;
		if (alpha < 255)
		{
			FAA = alpha - 140;
		}
		int FA = clamp(FAA, 0, 255);
		//draw->filled_rect_gradient(pos.x - 2, pos.y - 12, size.x + 40 + 4, size.y + 60 + 14, color_t(60, 125, 245, FA), color_t(0, 255, 255, FA), color_t(60, 125, 245, FA), color_t(245, 60, 60, FA));
	}
	
	
	//draw->FilledRect(0, 0, width, height, color_t(0, 0, 34, clamp((alpha - 200), 0, 255)));
	
	static int IDK3 = 600 + rand() % 400;
	if (FinishedLoading_T < 600 + IDK3) {
		FinishedLoading_T++;
	}
	else if (Alpha2 > 0) {

		Alpha2 -= (animation_speed * 60.f);

	}
	else if (!FinishedLoading) {
		
		FinishedLoading = true;
	}
	//auto highalpha = vars.movable.colormode == 0 ? color_t(210, 145, 255, (alpha / 255) * 80.f) : vars.movable.colormode == 3 ? color_t(170, 50, 255, (alpha / 255) * 80.f) : color_t(143, 150, 255, (alpha / 255) * 80.f);
	//auto dhighalpha = vars.movable.colormode == 0 ? color_t(210, 145, 255, (alpha / 255) * 50.f) : vars.movable.colormode == 3 ? color_t(170, 50, 255, (alpha / 255) * 50.f) : color_t(143, 150, 255, (alpha / 255) * 50.f);
	//auto lowalpha = vars.movable.colormode == 0 ? color_t(210, 145, 255, 0) : vars.movable.colormode == 3 ? color_t(170, 50, 255, 0) : color_t(143, 150, 255, 0);
	//draw->filled_rect_gradient(pos.x - 7, pos.y - 10, 7, size.y + 60 + 10, lowalpha, highalpha, highalpha, lowalpha); //left


	//draw->filled_rect_gradient(pos.x - 10 / 2, pos.y - 10 - 10 / 2, 10 / 2, 10 / 2,lowalpha,dhighalpha, dhighalpha, dhighalpha); //topleft


//	draw->filled_rect_gradient(pos.x + size.x + 40, pos.y - 10, 7, size.y + 60 + 10, highalpha, lowalpha, lowalpha, highalpha); //right
//	draw->filled_rect_gradient(pos.x + size.x + 40, pos.y - 10 - 10 / 2, 10 / 2, 10 / 2, dhighalpha, lowalpha, dhighalpha, dhighalpha); //top right

	//draw->filled_rect_gradient(pos.x,pos.y - 10 - 7, size.x + 40, 7,lowalpha,lowalpha,highalpha,highalpha); //top
//	draw->filled_rect_gradient(pos.x + size.x + 40, pos.y - 10 + size.y + 60 + 10, 10 / 2, 10 / 2, dhighalpha, dhighalpha, lowalpha, dhighalpha); //bottom right
//	draw->filled_rect_gradient(pos.x, pos.y - 10 + size.y + 60 + 10, size.x + 40, 7, highalpha, highalpha, lowalpha, lowalpha); //bottom
//	draw->filled_rect_gradient(pos.x - 10 / 2, pos.y - 10 + size.y + 60 + 10, 10 / 2, 10 / 2, dhighalpha, dhighalpha, dhighalpha, lowalpha); //bottom left
	draw->FilledRect(pos.x, pos.y - 10, size.x + 40, size.y + 60 + 10, vars.movable.colormode == 3 ? color_t(245, 245, 245, alpha) : vars.movable.colormode == 2 ? color_t(4, 4, 4, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 22, alpha) : color_t(0, 0, 34, alpha));
	

	//draw->FilledRect(pos.x, pos.y + 21, 100 - 2, size.y + 38, color_t(0, 0, 34, alpha));
	ImGui::PushFont(fonts::NonBoldIcon);
	ImVec2 TZ = ImGui::CalcTextSize(csgo->PUsername.c_str());
	ImGui::PopFont();

	ImGui::PushFont(fonts::icon);
	ImVec2 TZ2 = ImGui::CalcTextSize((std::to_string(csgo->PDaysLeft) + " days" ).c_str());
	ImGui::PopFont();
	TZ.x += 7;
	TZ2.x += 7;

	if (TZ2.x > TZ.x) {
		TZ.x = TZ2.x;
	}

	g_Render->_drawList->AddImageRounded((void*)ProfilePicture, ImVec2(pos.x + size.x - 40 - TZ.x, pos.y), ImVec2(pos.x + size.x + 40 - 20 - TZ.x, pos.y + 60), ImVec2(0, 0), ImVec2(1, 1), color_t(255, 255, 255, alpha).u32(), 15.f,ImDrawCornerFlags_All);
	//g_Render->CircleFilled(pos.x + size.x + 40 - 20 - TZ.x - 10 - 3, pos.y + 50, 10, vars.movable.colormode == 2 ? color_t(4, 4, 4, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 22, alpha) : color_t(0, 0, 34, alpha), 18);
	//g_Render->CircleFilled(pos.x + size.x + 40 - 20 - TZ.x - 10 - 3, pos.y + 50, 7, csgo->ConnectedToInternet ? color_t(66, 245, 123, alpha) : color_t(255, 82, 82, alpha), 18);
	g_Render->DrawString(pos.x + size.x + 40 - 20 - TZ.x - 10 - 3 + 7 / (1 + 1), pos.y + 50 + 7 / (1 + 1), vars.movable.colormode == 2 ? color_t(4, 4, 4, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 22, alpha) : color_t(0, 0, 34, alpha), render::centered_x | render::centered_y, fonts::CloudIndi, "E");
	g_Render->DrawString(pos.x + size.x + 40 - 20 - TZ.x - 10 - 3 + 7 / (1 + 1), pos.y + 50 + 7 / (1 + 1), csgo->ConnectedToInternet ? color_t(66, 245, 123, alpha) : color_t(255, 82, 82, alpha), render::centered_x | render::centered_y, fonts::NewIconsNotify, "E");
	g_Render->DrawString(pos.x + size.x + 50 - TZ.x + 7 - 30,pos.y + 5,color_t(255,255,255,alpha), render::none, fonts::NonBoldIcon, csgo->PUsername.c_str());
	g_Render->DrawString(pos.x + size.x + 50 - TZ.x + 9 - 30, pos.y + 26, color_t(148, 148, 148, alpha), render::none, fonts::icon, (std::to_string(csgo->PDaysLeft) + " days").c_str());
	
	
	//draw->FilledRect(pos.x, pos.y - 10, size.x + 40, 31, color_t(0, 0, 34, alpha));

	//
	
	//draw->DrawString(10, height - 15, vars.movable.colormode == 3 ? color_t(0, 0, 0, alpha) : color_t(255,255,255, alpha), render::centered_x, fonts::MenuL, "INFINITE");
	//draw->DrawString(10, height - 15, vars.movable.colormode == 0 ? color_t(210, 145, 255, alpha) : vars.movable.colormode == 3 ? color_t(170, 50, 255, alpha) : color_t(143, 150, 255, alpha), render::centered_x, fonts::MenuL, ".TECH");
	
	
	

	if (alpha > 0) {

		update();

		if (tabs.size() > 0) {
			render_tabs();
			cursor = Vector2D(pos.x + 15, pos.y + 65);
		}
		else
			cursor = Vector2D(pos.x + 15, pos.y + 25);
		Vector2D backup_cursor_pos = cursor;
		update_elements();
		cursor = backup_cursor_pos;
		render_elements();
	}
	if (g_active_element) {
		if (g_active_element->type != c_elementtype::child)
			((c_child*)active_element_parent)->set_cursor_position(g_active_element_pos);
		g_active_element->render();
	}

	if (!holding)
		holding_menu = !g_hovered_element && get_clicked();

	if (!FinishedLoading && Alpha2 > 0) {
		vars.menu.open = true;
		static float A3 = 0;
		static float Angle1;
		static float Angle2;
		static float Angle3 = 3.14;
		draw->FilledRect(pos.x, pos.y - 10, size.x + 40, size.y + 60 + 10, vars.movable.colormode == 3 ? color_t(245, 245, 245, Alpha2) : vars.movable.colormode == 2 ? color_t(4, 4, 4, Alpha2) : vars.movable.colormode == 1 ? color_t(0, 0, 22, Alpha2) : color_t(0, 0, 34, Alpha2));
		draw->DrawString(pos.x + (size.x + 40) / 2, pos.y - 10 + (size.y + 60 + 10) / 2 + 70, color_t(255,255,255, (A3 / 255) * Alpha2), render::centered_x | render::centered_y,fonts::EventLog, "INITIALIZING FREZZYHOOK API");
		draw->PArc(pos.x + (size.x + 40) / 2, pos.y - 10 + (size.y + 60 + 10) / 2 - (30), 50, Angle1, Angle2, 5.f, color_t(161,161,255,Alpha2));
		static bool InitAngle;
		float newanimspeed = animation_speed * 25.f;
		if (Angle2 < Angle3) {
			Angle2 += newanimspeed * 0.09f;
			Angle1 += newanimspeed * 0.045;
		}
		else {
			Angle2 += newanimspeed * 0.069;
			Angle1 += newanimspeed * 0.09;
			if (!InitAngle) {
				InitAngle = true;
				Angle3 += 3.14;

				if (Angle3 >= 6.28) {
					Angle3 -= 6.28;
				}
			}
		}
/*
		if (Angle1 > Angle2) {
			int AA = Angle2;
			Angle1 = Angle2;
			Angle2 = AA;
		}*/

		if (Angle2 >= 6.28) {
			Angle2 -= 6.28;
			Angle1 -=6.28;
		}

		if (Angle1 >= 6.28) {
			Angle1 -= 6.28;
			Angle2 -= 6.28;
		}

		
		
		if (A3 < 255 && FinishedLoading_T > 100) {
			A3 += animation_speed * 60.f;
		}
		if (A3 > 255) {
			A3 = 255;
		}
		
	}

	if (holding_menu && FinishedLoading)
		this->pos = Vector2D(g_mouse.x - drag.x, g_mouse.y - drag.y);

	

}

void c_window::update_elements()
{
	for (auto e : elements) {
		if (e->tab != active_tab_index) {
			e->update();
			continue;
		}
		e->update();
		if (e->type != c_elementtype::child) {
			int offset = e->get_total_offset();
			if (offset > 0)
				apply_element(offset + 3);
		}
	}
}
void c_window::render_elements()
{
	for (auto e : elements) {
		if (e->tab != active_tab_index)
			continue;
		e->render();
		if (e->type != c_elementtype::child) {
			int offset = e->get_total_offset();
			if (offset > 0)
				apply_element(offset + 3);
		}
	}
};

const vector<string> KeyStrings = {
	"-", "lm", "rm", "control+break", "mm", "m4", "m5",
	"unk", "backspace", "tab", "unk", "unk", "unk", "enter", "unk", "unk", "shift", "ctrl", "alt", "pause",
	"caps lock", "unk", "unk", "unk", "unk", "unk", "unk", "esc", "unk", "unk", "unk", "unk", "space",
	"pg up", "pg down", "end", "home", "left", "up", "right", "down", "unk", "print", "unk", "print screen", "insert",
	"delete", "unk", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
	"y", "z", "left windows", "right windows", "unk", "unk", "unk", "num 0", "num 1", "num 2", "num 3", "num 4", "num 5", "num 6",
	"num 7", "num 8", "num 9", "*", "+", "_", "-", ".", "/", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
	"f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "num lock", "scroll lock", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "lshift", "rshift", "lcontrol", "rcontrol", "lmenu", "rmenu", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "next track", "previous track", "stop", "play/pause", "unk", "unk",
	"unk", "unk", "unk", "unk", ";", "+", ",", "-", ".", "/?", "~", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "[{", "\\|", "}]", "'\"", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
	"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk","unk", "unk", "unk"
};