#include "checkbox.h"
#include "window.h"
#include "child.h"
#include "../Menu.h"
bool c_checkbox::update()
{

	if (should_render)
		if (!should_render()) {
			this->openalphanimation = 0.f;
			this->openanimation = 0.f;
			return false;
		}
	c_child* c = (c_child*)child;
	if (!c) return false;
	auto wnd = (c_window*)c->get_parent();
	if (!wnd) return false;
	if ((int)(wnd->get_transparency() * 2.55f) <= 20) {
		this->openalphanimation = 0.f;
		this->openanimation = 0.f;
	}
	if (wnd->g_active_element != this && wnd->g_active_element != nullptr) return true;
	auto pos = c->get_cursor_position();
	bool h = hovered();

	if (wnd->get_active_tab_index() != this->tab && wnd->get_tabs().size() > 0) {
		this->openalphanimation = 0.f;
		this->openanimation = 0.f;
		return false;
	}
	if (h) wnd->g_hovered_element = this;
	if (bind != nullptr) {
		if (binder.active) {
			for (auto i = 0; i < 256; ++i) {
				if (wnd->key_updated(i)) {
					if (i == VK_ESCAPE) {
						bind->key = 0;
						binder.active = false;
						wnd->g_active_element = nullptr;
						return true;
					}
					else {
						bind->key = i;
						binder.active = false;
						wnd->g_active_element = nullptr;
						return true;
					}
				}
			}
			return true;
		}
	}

	// bullshit animations
	if (h) {
		if (animation < 1.f) animation += animation_speed;
	}
	else {
		if (animation > 0.f) animation -= animation_speed;
	}
	animation = clamp(animation, 0.f, 1.f);

	if (h && c->hovered() && wnd->is_click()) {
		
		*(bool*)value = !(*(bool*)value);
		press_animation = 1.f;
		return true;
	}
	else if (h && c->hovered() && wnd->left_click() && bind != nullptr) {
		binder.open = true;
		wnd->g_active_element = this;
		return true;
	}
	int AE = 270;
	if (bind != nullptr) {
		if (binder.open) {
			auto size = Vector2D(g_size, 20);
			pos.y += after_text_offset;
			
			h = g_mouse.x >= pos.x - 70 + AE && g_mouse.y >= pos.y
				&& g_mouse.x <= pos.x - 70 + AE + size.x && g_mouse.y < pos.y + size.y;
			if (wnd->is_click() && h) {
				binder.open = false;
				wnd->g_active_element = nullptr;
				wnd->reset_mouse();
				return true;
			}
			h = g_mouse.x >= pos.x -70 + AE && g_mouse.y >= pos.y
				&& g_mouse.x <= pos.x - 70 + AE + size.x && g_mouse.y < pos.y + size.y * (binder.elements.size() + 1);
			if ((wnd->is_click() || wnd->left_click()) && !h) {
				binder.open = false;
				wnd->g_active_element = nullptr;
				wnd->reset_mouse();
				return true;
			}
			for (size_t i = 0; i < binder.elements.size(); i++) {
				pos.y += 20;
				h = g_mouse.x >= pos.x + AE && g_mouse.y >= pos.y
					&& g_mouse.x <= pos.x + AE + size.x && g_mouse.y < pos.y + size.y;
				if (h) {
					if (binder.animations[i] < 1.f) binder.animations[i] += animation_speed;
				}
				else
				{
					if (binder.animations[i] > 0.f) binder.animations[i] -= animation_speed;
				}
				if (binder.animations[i] > 1.f) binder.animations[i] = 1.f;
				else if (binder.animations[i] < 0.f) binder.animations[i] = 0.f;
				if (wnd->is_click() && h) {
					bind->type = i;
					binder.open = false;
					wnd->g_active_element = nullptr;
					wnd->reset_mouse();
					return true;
				}
			}
			wnd->g_active_element = this;
		}
		else {
			auto label_size = ImGui::CalcTextSize(label.c_str());
			auto txt_size = ImGui::CalcTextSize(string("[ " + string(KeyStrings[bind->key]) + " ]").c_str());
			auto size = Vector2D(txt_size.x, txt_size.y);
			h = g_mouse.x >= pos.x - 70 + AE && g_mouse.y >= pos.y
				&& g_mouse.x <= pos.x - 70 + AE + label_size.x + 15 + size.x && g_mouse.y < pos.y + size.y;
			if (h && c->hovered() && wnd->is_click() && bind->type > 0) {
				binder.active = true;
				wnd->g_active_element = this;
				return true;
			}
			if (h) {
				if (binder.animation < 1.f) binder.animation += animation_speed;
			}
			else {
				if (binder.animation > 0.f) binder.animation -= animation_speed;
			}
			if (binder.animation > 1.f) binder.animation = 1.f;
			else if (binder.animation < 0.f) binder.animation = 0.f;
		}
	}
	if (*(bool*)value && animationch < 20.f)
	{
		animationch += animation_speed * 40;
		animationch = clamp(animationch, 0.0f, 20.0f);
	}
	else if (!(*(bool*)value) && animationch > 0.f)
	{
		animationch -= animation_speed * 40;
		animationch = clamp(animationch, 0.0f, 20.0f);
	}
	if (press_animation > 0.f)
		press_animation -= animation_speed;

	float secondanimationspeed = animation_speed * 2.f;
	secondanimationspeed *= clamp(1.f - this->openanimation, 0.0008f, 1.f);
	this->openanimation += secondanimationspeed;
	//this->openalphanimation += animation_speed * 1.2f;
	//this->openalphanimation = clamp(this->openalphanimation, 0.f, 1.f);
//	this->openanimation = 1.f;
	this->openanimation = clamp(this->openanimation, 0.f, 1.f);
	this->openalphanimation = 1.f;
	press_animation = clamp(press_animation, 0.f, 1.f);
	return true;
}

bool c_checkbox::hovered()
{
	int AE = 270;
	if (should_render)
		if (!should_render())
			return false;
	c_child* c = (c_child*)child;
	if (!c->hovered())
		return false;
	auto pos = c->get_cursor_position();
	Vector2D size = { 14, 14 };
	return g_mouse.x > pos.x + AE - 1 - 1 - 12 && g_mouse.y > pos.y + 3 - 1
		&& g_mouse.x < pos.x + AE + 14 + 2 + 12 && g_mouse.y < pos.y + size.y + 2 + 2;
}

void c_checkbox::render() {
	if (should_render)
		if (!should_render())
			return;
	c_child* c = (c_child*)child;
	auto pos = c->get_cursor_position();

	auto wnd = (c_window*)c->get_parent();
	auto size = ImGui::CalcTextSize(label.c_str());
	if (!wnd) return;
	if (wnd->get_active_tab_index() != this->tab 
		&& wnd->get_tabs().size() > 0) return;
	auto alpha = (int)(wnd->get_transparency() * 2.55f) * this->openalphanimation;

	float StringColor = (animationch / 20) * 55.f;


	g_Render->DrawString(pos.x - 7, pos.y - 2, color_t(200 + StringColor, 200 + StringColor, 200 + StringColor, alpha),
		render::none, fonts::menu_desc, label.c_str());
	

	int AE = 270;

	g_Render->CircleFilled(pos.x + AE - 10, pos.y + 3 + 7, 7, vars.movable.colormode == 3 ? color_t(182, 151, 196, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 58, alpha) : vars.movable.colormode == 2 ? color_t(28, 28, 28, alpha) : color_t(23, 16, 72, alpha), 70);
	g_Render->CircleFilled(pos.x + AE - 5, pos.y + 3 + 7, 7, vars.movable.colormode == 3 ? color_t(182, 151, 196, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 58, alpha) : vars.movable.colormode == 2 ? color_t(28, 28, 28, alpha) : color_t(23, 16, 72, alpha), 70);
	g_Render->CircleFilled(pos.x + AE, pos.y + 3 + 7, 7, vars.movable.colormode == 3 ? color_t(182, 151, 196, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 58, alpha) : vars.movable.colormode == 2 ? color_t(28, 28, 28, alpha) : color_t(23, 16, 72, alpha), 70);
	g_Render->CircleFilled(pos.x + AE + 5, pos.y + 3 + 7, 7, vars.movable.colormode == 3 ? color_t(182, 151, 196, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 58, alpha) : vars.movable.colormode == 2 ? color_t(28, 28, 28, alpha) : color_t(23, 16, 72, alpha), 70);
	g_Render->CircleFilled(pos.x + AE + 10, pos.y + 3 + 7, 7, vars.movable.colormode == 3 ? color_t(182, 151, 196, alpha) : vars.movable.colormode == 1 ? color_t(0, 0, 58, alpha) : vars.movable.colormode == 2 ? color_t(28, 28, 28, alpha) : color_t(23, 16, 72, alpha), 70);

	if (hovered()) {
		floatnewanim = clamp(floatnewanim + animation_speed * 230.f, 0.f, 255.f);
		
	}
	else {
		floatnewanim = clamp(floatnewanim - animation_speed * 230.f, 0.f, 255.f);
	}

	if (floatnewanim > 0.f) {
		float pnalpha = (animationch < 8) ? (alpha / 255) * (floatnewanim  * 0.59f) : (alpha / 255) * floatnewanim;
		g_Render->CircleFilled(pos.x + AE - 10 + animationch * this->openanimation, pos.y + 3 + 7, 14, vars.movable.colormode == 3 ? color_t(170, 50, 255, (pnalpha / 255) * 70) : color_t(143, 150, 255, (pnalpha / 255) * 70), 70);
	}

		g_Render->CircleFilled(pos.x + AE - 10 + animationch * this->openanimation, pos.y + 3 + 7, 9, (animationch < 8) ? vars.movable.colormode == 3 ? color_t(215, 187, 252, alpha) : vars.movable.colormode == 1 ? color_t(34, 34, 64, alpha) : vars.movable.colormode == 2 ? color_t(64, 64, 64, alpha) : color_t(52, 34, 64, alpha) : vars.movable.colormode == 0 ? color_t(210, 145, 255, alpha) : vars.movable.colormode == 3 ? color_t(170, 50, 255, alpha) : color_t(143, 150, 255, alpha), 70);
	
	//g_Render->FilledRect(pos.x + AE, pos.y + 3, 12, 12,
	//	!(*(bool*)value) ? color_t(0, 0, 34, alpha) : color_t(210, 145,255,
	//		alpha * (1.f - press_animation))); 
	//g_Render->Rect(pos.x + AE - 1, pos.y + 2, 14, 14, color_t(210,145,255, alpha));

	if (bind != nullptr && bind->type > 0) {
		auto color = bind->key > 0 ? color_t(210 + 10.f * binder.animation - 10.f, 145 + 10.f * binder.animation - 10.f, 255, alpha)
			: color_t(150 + 105.f * binder.animation, 150 + 105.f * binder.animation, 150 + 105.f * binder.animation, alpha);

		g_Render->DrawString(pos.x + AE - 70, pos.y + size.y / 2 - 3, color,
			render::centered_y, fonts::menu_main, binder.active ? "-" : (bind->key > 0 ? "%s", KeyStrings[bind->key].c_str() : "?"));
	}
	if (bind != nullptr && binder.open) {
		pos.x += AE;
		pos.y += after_text_offset;
		auto size = Vector2D(g_size, 20);
		
		g_Render->FilledRect(pos.x, pos.y, size.x, size.y,
			color_t(17, 10, 47, alpha));

		auto base = ImVec2(pos.x + size.x - 10, pos.y + size.y / 2);
		g_Render->_drawList->AddTriangleFilled(
			ImVec2(base.x - 3, base.y + 2), ImVec2(base.x + 3, base.y + 2), ImVec2(base.x, base.y - 4), color_t(255, 255, 255, alpha).u32());
		g_Render->DrawString(pos.x + 10, pos.y + size.y / 2, color_t(255, 255, 255, alpha), render::centered_y,
			fonts::menu_desc, "%s (%s)", KeyStrings[bind->key].c_str(), binder.elements[bind->type].c_str());

		for (size_t i = 0; i < binder.elements.size(); i++) {
			pos.y += size.y;
			g_Render->FilledRect(pos.x, pos.y, size.x, size.y,
				color_t(17, 10, 47, alpha));

			auto clr2 = color_t(200 + binder.animations[i] * 55.f, 200 + binder.animations[i] * 55.f, 200 + binder.animations[i] * 55.f, alpha);
			g_Render->DrawString(pos.x + 10, pos.y + size.y / 2, clr2, render::centered_y,
				fonts::menu_desc, binder.elements[i].c_str());
		}
		//g_Render->DrawLine(pos.x, pos.y + size.y, pos.x + size.x, pos.y + size.y, color_t(0, 0, 0, alpha));
	}
}

