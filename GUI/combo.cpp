#include "combo.h"
#include "child.h"
#include "../Menu.h"
bool c_combo::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	auto c = (c_child*)child;
	auto pos = c->get_cursor_position();
	pos.x -= 5;
	if (!c->hovered())
		return false;
	auto size = Vector2D(290, 24);
	if (label.size() > 0)
		pos.y += after_text_offset;
	return g_mouse.x >= pos.x && g_mouse.y >= pos.y
		&& g_mouse.x <= pos.x + size.x && g_mouse.y <= pos.y + size.y;
}
bool c_combo::update()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return false;
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false;
	auto pos = c->get_cursor_position();
	auto size = Vector2D(290, 24);
	pos.x -= 5;
	bool h = hovered();
	if (h) wnd->g_hovered_element = this;
	if (label.size() > 0)
		pos.y += after_text_offset;
	if (h && c->hovered() && wnd->is_click()) {
		open = !open;
		wnd->g_active_element = this;
		return true;
	}
	if (open) {
		if (open_animation < 1.f)
			open_animation += animation_speed;
		h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
			&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y * (elements.size() + 1);
		if (wnd->is_click() && !h) {
			open = false;
			wnd->g_active_element = nullptr;
			wnd->reset_mouse();
			return true;
		}
		if (open_animation == 1.f) {
			for (size_t i = 0; i < elements.size(); i++) {
				pos.y += 20;
				h = g_mouse.x >= pos.x && g_mouse.y >= pos.y
					&& g_mouse.x <= pos.x + size.x && g_mouse.y < pos.y + size.y;
				if (h) {
					if (h) wnd->g_hovered_element = this;
					if (animations[i] < 1.f) animations[i] += animation_speed;
				}
				else
				{
					if (animations[i] > 0.f) animations[i] -= animation_speed;
				}
				if (animations[i] > 1.f) animations[i] = 1.f;
				else if (animations[i] < 0.f) animations[i] = 0.f;
				if (wnd->is_click() && h) {
					*(int*)value = i;
					open = false;
					wnd->g_active_element = nullptr;
					wnd->reset_mouse();
					if (on_change_value)
						on_change_value(i);
					return true;
				}
			}
		}
		wnd->g_active_element = this;
	}
	else {
		if (wnd->g_active_element == this)
			wnd->g_active_element = nullptr;
		if (open_animation > 0.f)
			open_animation -= animation_speed;

	}
	// bullshit animations
	if (h || open) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	if (open_animation > 1.f) open_animation = 1.f;
	else if (open_animation < 0.f) open_animation = 0.f;
	animation = clamp(animation, 0.f, 1.f);

	*(int*)value = clamp(*(int*)value, 0, (int)elements.size() - 1);
	return false;
}
void c_combo::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	if (!c)
		return;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd)
		return;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;
	auto pos = c->get_cursor_position();
	auto alpha = (int)(wnd->get_transparency() * 2.55f);
	auto size = Vector2D(290, 20);

	bool h = hovered();
	pos.x -= 7;
	auto clr = color_t(40 + animation * 10.f, 40 + animation * 10.f, 40 + animation * 10.f, alpha);
	if (label.size() > 0) {
		if (this == wnd->g_active_element)
			c->lock_bounds();
		g_Render->DrawString(pos.x, pos.y, color_t(200 + 55.f * animation, 200 + 55.f * animation, 200 + 55.f * animation, alpha),
			render::none, fonts::menu_desc, label.c_str());
		pos.y += after_text_offset;
		if (this == wnd->g_active_element)
			c->unlock_bounds();
	}
	pos.x += 2;
	if (open) {
		auto new_alpha = clamp(alpha * open_animation, 0.f, 255.f);
		//g_Render->Rect(pos.x - 1, pos.y - 1, size.x + 2, (size.y * (elements.size() + 1)) + 2, color_t(0, 0, 0, new_alpha));
		g_Render->FilledRect(pos.x, pos.y, size.x, size.y,
			vars.movable.colormode == 2 ? color_t(35, 35, 35, alpha) : color_t(17, 10, 47, alpha));



		auto base = ImVec2(pos.x + size.x - 10, pos.y + size.y / 2);

		g_Render->_drawList->AddTriangleFilled(
			ImVec2(base.x - 3, base.y + 2), ImVec2(base.x + 3, base.y + 2), ImVec2(base.x, base.y - 4), color_t(255, 255, 255, new_alpha).u32());


	
		g_Render->DrawString(pos.x + 10, pos.y + size.y / 2, color_t(255, 255, 255, alpha), render::centered_y,
			fonts::menu_desc, elements[*(int*)value].c_str());

		for (size_t i = 0; i < elements.size(); i++) {
			pos.y += size.y;
			
			g_Render->FilledRect(pos.x, pos.y, size.x, size.y + 5,
				vars.movable.colormode == 2 ? color_t(35, 35, 35, alpha) : color_t(17, 10, 47, new_alpha));

			auto clr2 = color_t(200 + animations[i] * 55.f, 200 + animations[i] * 55.f, 200 + animations[i] * 55.f, new_alpha);
			g_Render->DrawString(pos.x + 10, pos.y + size.y / 2, *(int*)value == i
				? vars.movable.colormode == 0 ? color_t(210, 145, 255, alpha) : vars.movable.colormode == 3 ? color_t(170, 50, 255, alpha) : color_t(143, 150, 255, alpha) : clr2, render::centered_y,
				fonts::menu_desc, elements[i].c_str());
		}
		//g_Render->DrawLine(pos.x, pos.y + size.y, pos.x + size.x, pos.y + size.y, color_t(0, 0, 0, new_alpha));
	}
	else {
		auto new_alpha = clamp(alpha * (1.f - open_animation), 0.f, 255.f);
		auto clr2 = color_t(40 - animation * 10.f, 40 - animation * 10.f, 40 - animation * 10.f, new_alpha);
		
		g_Render->Rect(pos.x - 1, pos.y - 1, size.x + 2, size.y + 2, vars.movable.colormode == 2 ? color_t(35, 35, 35, alpha) : color_t(17, 10, 47, new_alpha));
		g_Render->FilledRect(pos.x, pos.y, size.x, size.y, vars.movable.colormode == 2 ? color_t(35, 35, 35, alpha) : color_t(17, 10, 47, alpha));
		g_Render->DrawString(pos.x + 10, pos.y + size.y / 2 - 2, color_t(200 + 55.f * animation, 200 + 55.f * animation, 200 + 55.f * animation, alpha),
			render::centered_y, fonts::menu_desc, elements[*(int*)value].c_str());

		auto base = ImVec2(pos.x + size.x - 10, pos.y + size.y / 2);
		g_Render->_drawList->AddTriangleFilled(
			ImVec2(base.x - 4, base.y - 2), ImVec2(base.x + 4, base.y - 2), ImVec2(base.x, base.y + 5), color_t(255, 255, 255, new_alpha).u32());
	}
}

int c_combo::get_total_offset() {
	if (should_render)
		if (!should_render())
			return 0;
	return 27 + 2 + (label.size() > 0) * after_text_offset;
};