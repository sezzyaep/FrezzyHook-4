#pragma once
#include "window.h"

class c_tab : public c_window {
private:
	int index;
	float animation;
public:
	c_tab(string name, int index, c_window* child) {
		this->child = child;
		this->index = index;
		set_title(name);
		animation = 0.f;
	}
	void draw(Vector2D pos, Vector2D size, int gay);
	void render();
	void special_render();
};