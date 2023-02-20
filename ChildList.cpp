#include "ChildList.h"
#include "..\weave\GUI\window.h"
#include "../weave/Menu.h"
#include "..\weave\GUI\child.h"

bool c_childlist::update()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false;
	auto pos = c->get_cursor_position();

	auto alpha = (int)(c->get_transparency() * 2.55f);
	auto size = Vector2D(g_size + 145, height);
	bool h = hovered();
	should_draw_scroll = total_elements_size > size.y;
	if (h) {
		wnd->g_hovered_element = this;
		while (csgo->scroll_amount > 0) {
			scroll -= 10;
			csgo->scroll_amount--;
		}
		while (csgo->scroll_amount < 0) {
			scroll += 10;
			csgo->scroll_amount++;
		}
	}

	if (should_draw_scroll) {
		scroll = clamp(scroll, 0.f, clamp(total_elements_size - size.y, 0.f, FLT_MAX));
	}
	else
		scroll = 0.f;
	if (should_draw_scroll)
		pos.y -= scroll;
	size = Vector2D(g_size + 145, 24.f);
	for (int i = 0; i < elements.size(); i++) {

		bool hvrd = g_mouse.x > pos.x && g_mouse.y >= pos.y
			&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
		if (hvrd) {
			if (animations[i] < 1.f) animations[i] += animation_speed;
		}
		else {
			if (animations[i] > 0.f) animations[i] -= animation_speed;
		}
		if (hvrd && c->hovered() && wnd->is_click()) {
			*(unsigned int*)value = i;
			return true;
		}
		animations[i] = clamp(animations[i], 0.f, 1.f);
		pos.y += 24;

	}




	if (press_animation > 0.f)
		press_animation -= animation_speed;
	if (h) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}

	animation = clamp(animation, 0.f, 1.f);
	press_animation = clamp(press_animation, 0.f, 1.f);
	return false;
}
bool c_childlist::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto pos = c->get_cursor_position();
	pos.x -= 20;
	auto size = Vector2D(g_size + 145, height);
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + size.x && g_mouse.y < pos.y + size.y;
}
void c_childlist::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();
	pos.x -= 20;

	auto size = Vector2D(g_size, height);

	size.x += 145;


	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;

	auto alpha = (int)(wnd->get_transparency() * 2.55f);
	auto clr = color_t(40 + animation * 10.f, 40 + animation * 10.f, 40 + animation * 10.f, alpha * (1 - press_animation));
	auto clr2 = color_t(40 - animation * 10.f, 40 - animation * 10.f, 40 - animation * 10.f, alpha * (1 - press_animation));

	g_Render->Rect(pos.x - 1, pos.y - 1, size.x + 2, size.y + 2, color_t(0, 0, 0, alpha), 7.f);
	g_Render->_drawList->PushClipRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), true);
	g_Render->FilledRect(pos.x, pos.y, size.x, size.y,
		vars.movable.colormode == 2 ? color_t(35, 35, 35, alpha) : color_t(17, 10, 47, alpha), 7.f);
	if (should_draw_scroll)
		pos.y -= scroll;
	for (int i = 0; i < elements.size(); i++) {
		if (animations[i] > 0)
		{
			g_Render->FilledRect(pos.x, pos.y, size.x, 24,
				vars.movable.colormode == 2 ? color_t(35, 35, 35, alpha) : color_t(17 + 10.f * animations[i], 10 + 10.f * animations[i], 47 + 10.f * animations[i], alpha));
		}
		g_Render->DrawString(pos.x + 10, pos.y + 10,
			*(unsigned int*)value == i ? vars.movable.colormode == 0 ? color_t(210, 145, 255, alpha) : vars.movable.colormode == 3 ? color_t(170, 50, 255, alpha) : color_t(143, 150, 255, alpha) :
			color_t(200 + 55 * animations[i], 200 + 55 * animations[i], 200 + 55 * animations[i], alpha), render::centered_y,

			fonts::menu_desc, elements[i].c_str());
		pos.y += 24;
	}
	if (should_draw_scroll)
		pos.y += scroll;
	pos.y -= 24 * (elements.size());

	g_Render->_drawList->PopClipRect();

}