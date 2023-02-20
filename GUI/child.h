#pragma once
#include "window.h"

class c_child : public c_window {
private:
	struct {
		float value, animation, hovering_animation, offset_y;
		float clicked_pos_y;
		bool dragging;
	} scroll;
	Vector2D active_element_position;
public:
	c_element* get_parent();
	c_child(string label, int tab, c_window* wnd) {
		this->set_title(label);
		this->child = wnd;
		this->tab = tab;
		this->type = c_elementtype::child;
		this->horizontal_offset = true;
		scroll.value = 0.f;
		scroll.hovering_animation = 0.f;
		scroll.animation = 0.f;
		scroll.dragging = false;
	}
	float OpenAnimation;
	bool should_draw_scroll;
	int total_elements_size;
	bool horizontal_offset;
	void lock_bounds();
	void unlock_bounds();
	int get_total_offset();
	bool update();
	void update_elements();
	void reset_elements();
	void render_elements();
	void render();
	void special_render() {};
	bool hovered();
	void initialize_elements();
};