#include "slider.h"
#include "window.h"
#include "child.h"
#include "../Menu.h"
bool c_slider::hovered()
{
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto pos = c->get_cursor_position();
	if (label.size() > 0)
		pos.y += after_text_offset;
	pos.x -= 5;
	auto this_size = Vector2D(250, 12);
	return g_mouse.x > pos.x && g_mouse.y > pos.y
		&& g_mouse.x < pos.x + this_size.x && g_mouse.y < pos.y + this_size.y;
}

bool c_slider::update()
{
	if (should_render)
		if (!should_render()) {
	
			this->openanim = 0.f;
			return false;
		}
	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if ((int)(wnd->get_transparency() * 2.55f) <= 20) {
		this->openanim = 0.f;
	}
	if (!wnd) return false;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0) {
		this->openanim = 0.f;
		return false;
	}
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return false;
	auto pos = c->get_cursor_position();
	auto alpha = (int)(c->get_transparency() * 2.55f);
	auto size = Vector2D(250, 12);
	pos.x -= 5;
	bool h = hovered();
	if (h) wnd->g_hovered_element = this;
	if (label.size() > 0)
		pos.y += after_text_offset;

	if (wnd->is_holding() && h && c->hovered()) {
		dragging = true;
	}
	else {
		if (!wnd->is_holding())
			dragging = false;
		else if (wnd->is_holding() && !h && dragging)
			dragging = true;
		else
			dragging = false;
	}

	if (dragging) {
		float newpos = g_mouse.x - pos.x;
		if (newpos < 0)
			newpos = 0;
		if (newpos > size.x)
			newpos = size.x;
		float ratio = newpos / size.x;
		if (round)
			*(int*)value = (int)(min + (max - min) * ratio);
		else
			*(float*)value = min + (max - min) * ratio;
		wnd->g_active_element = this;
	} else {
		wnd->g_active_element = nullptr;
	}
	if (round) {
		if (*(int*)value > max)
			*(int*)value = max;
		else if (*(int*)value < min)
			*(int*)value = min;
	}
	else
	{
		if (*(float*)value > max)
			*(float*)value = max;
		else if (*(float*)value < min)
			*(float*)value = min;
	}

	// bullshit animations
	if (h || dragging) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = clamp(animation, 0.f, 1.f);
	float secondanimationspeed = animation_speed * 2.f;
	secondanimationspeed *= clamp(1.f - this->openanim, 0.0008f,1.f);
	this->openanim += secondanimationspeed ;
	this->openanim = clamp(this->openanim, 0.f, 1.f);
	//this->openalphaanim += animation_speed * 1.2f;
	//this->openalphaanim = clamp(this->openalphaanim, 0.f, 1.f);
	this->openalphaanim = 1.f;
	return dragging;
}

void c_slider::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();
	pos.x -= 7;
	auto size = Vector2D(250, 12);
	bool h = hovered();
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0)
		return;
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * openalphaanim;

	auto clr = color_t(210, 145, 255, alpha);
	auto clr2 = color_t(210, 145, 255, alpha);
	if (label.size() > 0) {
		g_Render->DrawString(pos.x, pos.y, vars.movable.colormode == 3 ? color_t(0, 0, 0, alpha) : color_t(255, 255, 255, alpha),
			render::none, fonts::menu_desc, label.c_str());
		pos.y += after_text_offset;
	}

	pos.x += 2;
	float val = round ? *(int*)value : *(float*)value;
	float ratio = (float)(val - this->min) / (float)(this->max - this->min);
	float location = ratio * size.x;
	location *= this->openanim;
	g_Render->FilledRect(pos.x, pos.y + 2, size.x, size.y - 6, vars.movable.colormode == 3 ? color_t(182, 151, 196, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 58, alpha) : vars.movable.colormode == 2 ? color_t(28, 28, 28, alpha) : color_t(23, 16, 72, alpha), 5.f);
	//g_Render->filled_rect_gradient(pos.x, pos.y, location, size.y - 2, clr, clr, clr2, clr2);
	if (hovered()) {
		floatnewanim = clamp(floatnewanim + 20.f, 0.f, 255.f);

	}
	else {
		floatnewanim = clamp(floatnewanim - 20.f, 0.f, 255.f);
	}
	if (dragging) {
		floatnewanim = 255.f;
	}

	if (floatnewanim > 0.f) {
		float pnalpha = (alpha / 255) * floatnewanim;
		g_Render->CircleFilled(pos.x + location, pos.y + size.y / 2, 13, vars.movable.colormode == 3 ? color_t(170, 50, 255, (pnalpha / 255) * 70) : color_t(143, 150, 255, (pnalpha / 255) * 70), 70);
	}
	g_Render->CircleFilled(pos.x + location, pos.y + size.y / 2, 7, vars.movable.colormode == 0 ? color_t(210, 145, 255, alpha) : vars.movable.colormode == 3 ? color_t(170, 50, 255, alpha) : color_t(143, 150, 255, alpha), 70);
	//g_Render->Rect(pos.x - 1, pos.y - 1, size.x + 2, size.y, color_t(210, 145, 255, alpha));
	if (this == wnd->g_active_element)
		c->lock_bounds();

	g_Render->DrawString(pos.x + 282, pos.y + size.y / 2 - 3, vars.movable.colormode == 3 ? color_t(0, 0, 0, alpha) : color_t(255, 255, 255, alpha),
		render::centered_x | render::centered_y | render::outline, fonts::menu_desc, format.c_str(), val);
	if (this == wnd->g_active_element)
		c->unlock_bounds();
}