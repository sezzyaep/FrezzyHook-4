#include "tab.h"
#include "child.h"
#include "../Variables.h"
void c_tab::draw(Vector2D pos, Vector2D size, int gay) {
	c_child* c = (c_child*)child;
	auto alpha = (int)(c->get_transparency() * 2.55f);
	bool hovered = g_mouse.x > pos.x + gay / 2 - 10 && g_mouse.y > pos.y - 5
		&& g_mouse.x < pos.x + size.x + gay / 2 + 10 && g_mouse.y < pos.y + size.y + 5;
	if (hovered && !c->g_active_element && !c->is_holding_menu()) {
		c->g_hovered_element = this;
		if (c->is_click()) {
			c->set_active_tab_index(this->index);
			c->reset_mouse();
		}

		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}

	animation = clamp(animation, 0.f, 1.f);

	auto clr = color_t(18 + animation * 20.f, 18 + animation * 20.f, 18 + animation * 20.f, alpha);
	auto clr2 = color_t(18 - animation * 20.f, 18 - animation * 20.f, 18 - animation * 20.f, alpha);


	auto t_clr = vars.movable.colormode == 3 ? color_t(160 - animation * 120.f, 160 - animation * 120.f, 160 - animation * 120.f, alpha) : color_t(200 + animation * 20.f, 200 + animation * 20.f, 200 + animation * 20.f, alpha);
	if (c->get_active_tab_index() == this->index)
	{
		vars.movable.colormode == 3 ? t_clr = color_t(0, 0, 0, alpha) : t_clr = color_t(255, 255, 255, alpha);
	}
	else
	{
		//clamp bugged
		int intermediate;
		if (vars.movable.colormode == 3)
		{
			intermediate = 160 - animation * 120;
			if (intermediate > 160)
				intermediate = 160;
			else if (intermediate < 0)
				intermediate = 0;

		}
		else
		{
			intermediate = 150 + animation * 100;
			if (intermediate > 255)
				intermediate = 255;
			else if (intermediate < 150)
				intermediate = 150;
		}
		t_clr = color_t(intermediate, intermediate, intermediate, alpha);
	}
	ImFont* UsedFont;
	if (c->get_active_tab_index() == this->index)
	{
		UsedFont = fonts::NonBoldIcon;
	}
	else
	{
		//g_Render->FilledRect(pos.x + 41, pos.y + 1, size.x - 3, size.y - 2, clr);
		UsedFont = fonts::icon;
	}
	//auto t_clr = color_t(255, 255, 255, alpha);
	std::string info = "";
	if (get_title() == std::string("P"))
	{
		info = "Legit";
	}
	if (get_title() == std::string("T"))
	{
		info = "Rage";
	}
	if (get_title() == std::string("Q"))
	{
		info = "AntiAim";
	}
	if (get_title() == std::string("N"))
	{
		info = "Players";
	}
	if (get_title() == std::string("S"))
	{
		info = "World";
	}
	if (get_title() == std::string("B"))
	{
		info = "User";
	}
	if (get_title() == std::string("U"))
	{
		info = "Misc";
	}

	//g_Render->DrawString(pos.x + size.x / 2 + 39, pos.y + size.y / 2 - 13, t_clr, render::centered_x, fonts::Iconss, get_title().c_str());
	g_Render->DrawString(pos.x + size.x / 2 + gay / 2, pos.y + size.y / 2, t_clr, render::centered_x, UsedFont, info.c_str());
}
void c_tab::special_render() {

}
void c_tab::render() {

}