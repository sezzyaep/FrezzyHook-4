#pragma once
#include "Hooks.h"
int showtime = 5.f;
string prefix = "[ Cat ] ";


class CMessage
{
public:
	CMessage(string str, float time, color_t clr)
	{
		this->str = str;
		this->time = csgo->get_absolute_time();
		this->clr = clr;
		alpha = 255;
	}
	std::string str;
	float time;
	color_t clr;
	int alpha;
};

class CEventlog
{
public:
	std::vector <CMessage> messages;
	void Draw()
	{
		if (!interfaces.engine->IsConnected() || !interfaces.engine->IsInGame()) {
			messages.clear();
			return;
		}

		if (!vars.visuals.eventlog)
			return;
		while (messages.size() > 10)
			messages.erase(messages.begin());
		for (int i = messages.size() - 1; i >= 0; i--)
		{
			if (messages[i].time + showtime <= csgo->get_absolute_time())
				messages[i].alpha -= 1;

			//std::transform(messages[i].str.begin(), messages[i].str.end(), messages[i].str.begin(), toupper);
			int E = (16 * i);
			int V = E + 4;
			if (messages[i].alpha < 40)
			{
				V = E + 4 - (40 * (40 - messages[i].alpha) / 40.f);
			}
			Drawing::DrawString(fonts::esp_logs, 4, V,
				color_t(messages[i].clr.get_red(), messages[i].clr.get_green(), messages[i].clr.get_blue(), messages[i].alpha), FONT_LEFT,
				messages[i].str.c_str());

			if (messages[i].alpha <= 0)
				messages.erase(messages.begin() + i);
		}

	}
};