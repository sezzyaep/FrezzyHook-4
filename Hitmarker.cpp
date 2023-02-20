#include "Hitmarker.h"
CHitMarker* g_Hitmarker = new CHitMarker();
IBasePlayer* CHitMarker::GetPlayer(int ID)
{
	int index = interfaces.engine->GetPlayerForUserID(ID);
	return interfaces.ent_list->GetClientEntity(index);
}
void CHitMarker::Paint()
{
	if (!vars.visuals.hitmarker)
		return;
	if (hitmarkers.size() == 0)
		return;

	// draw
	for (int i = 0; i < hitmarkers.size(); i++)
	{
		Vector pos3D = Vector(hitmarkers[i].impact.x, hitmarkers[i].impact.y, hitmarkers[i].impact.z), pos2D;
		if (!Math::WorldToScreen(pos3D, pos2D))
			continue;

		auto r = vars.visuals.hitmarker_color[0],
			g = vars.visuals.hitmarker_color[1],
			b = vars.visuals.hitmarker_color[2];
		
		if (vars.visuals.hitmarker > 0) {
			pos2D.y += 3;
			g_Render->DrawLine(pos2D.x + 3, pos2D.y + 3, pos2D.x + 8, pos2D.y + 8, color_t(r, g, b, hitmarkers[i].alpha));
			g_Render->DrawLine(pos2D.x - 3, pos2D.y - 3, pos2D.x - 8, pos2D.y - 8, color_t(r, g, b, hitmarkers[i].alpha));
			g_Render->DrawLine(pos2D.x + 3, pos2D.y - 3, pos2D.x + 8, pos2D.y - 8, color_t(r, g, b, hitmarkers[i].alpha));
			g_Render->DrawLine(pos2D.x - 3, pos2D.y + 3, pos2D.x - 8, pos2D.y + 8, color_t(r, g, b, hitmarkers[i].alpha));
			pos2D.y -= 3;
		}
		if (hitmarkers[i].alphadmg > 0) {
			g_Render->DrawString(pos2D.x, pos2D.y - 3 - hitmarkers[i].dmg_x, hitmarkers[i].kill ? color_t(255, 0, 0, hitmarkers[i].alphadmg) : color_t(255, 255, 255, hitmarkers[i].alphadmg), render::centered_x, fonts::NonBoldIcon, std::to_string(hitmarkers[i].dmg).c_str());
		}
	}
	// proceeed
	for (int i = 0; i < hitmarkers.size(); i++) {
		
		if (vars.visuals.hitmarker > 0) {
			if (hitmarkers[i].time + 4.25f <= csgo->get_absolute_time()) {
				hitmarkers[i].alpha -= 8.f;
			}
		}
		hitmarkers[i].dmg_x += 0.5f;
		if (hitmarkers[i].alphadmg > 0) {
			hitmarkers[i].alphadmg -= 2.f;
		}
		if (hitmarkers[i].alpha <= 0)
			hitmarkers.erase(hitmarkers.begin() + i);
	}
}
void CHitMarker::OnPlayerHurt(IGameEvent* pEvent)
{
	auto attacker = GetPlayer(pEvent->GetInt("attacker"));
	auto victim = GetPlayer(pEvent->GetInt("userid"));

	if (vars.misc.killsaysandshit.enabledeathsay) {
		if (victim == csgo->local && attacker != csgo->local) {
			if (pEvent->GetInt("dmg_health") >= attacker->GetHealth()) {
				std::string say = "say " + vars.misc.killsaysandshit.deathsay;
				interfaces.engine->ExecuteClientCmd(say.c_str());
			}
		}
	}

	if (!attacker || !victim || attacker != csgo->local)
		return;

	if (vars.misc.killsaysandshit.enablekillsay) {
		if (pEvent->GetInt("dmg_health") >= victim->GetHealth()) {
			std::string say = "say ";
			if (vars.misc.killsaysandshit.killsayflags & 1) {
				say += victim->GetName();
				say += " ";
			}
			say += vars.misc.killsaysandshit.killsay;
			if (vars.misc.killsaysandshit.killsayflags & 2) {
				say += victim->GetName();
			}
			interfaces.engine->ExecuteClientCmd(say.c_str());
		}
	}

	Vector enemypos = victim->GetOrigin();
	impact_info best_impact;
	float best_impact_distance = -1;
	float time = csgo->get_absolute_time();


	for (int i = 0; i < impacts.size(); i++)
	{
		auto iter = impacts[i];
		if (time > iter.time + 1.f)
		{
			impacts.erase(impacts.begin() + i);
			continue;
		}
		Vector position = Vector(iter.x, iter.y, iter.z);
		float distance = position.DistTo(enemypos);
		if (distance < best_impact_distance || best_impact_distance == -1)
		{
			best_impact_distance = distance;
			best_impact = iter;
		}
	}
	if (best_impact_distance == -1)
		return;

	hitmarker_info info;
	info.impact = best_impact;
	info.alpha = 255;
	info.time = csgo->get_absolute_time();
	info.dmg = pEvent->GetInt("dmg_health");
	info.dmg_x = 0;
	info.kill = pEvent->GetInt("dmg_health") >= victim->GetHealth();
	info.alphadmg = 255;
	hitmarkers.push_back(info);
}
void CHitMarker::OnBulletImpact(IGameEvent* pEvent)
{
	auto shooter = GetPlayer(pEvent->GetInt("userid"));
	if (!shooter || shooter != csgo->local)
		return;
	impact_info info;
	info.x = pEvent->GetFloat("x");
	info.y = pEvent->GetFloat("y");
	info.z = pEvent->GetFloat("z");
	info.time = csgo->get_absolute_time();
	impacts.push_back(info);
}


