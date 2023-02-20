#include "Hooks.h"
#include "Bunnyhop.h"
#include "Legitbot.h"
#include "Ragebot.h"
#include "AntiAims.h"
#include "checksum_md5.h"
#include "Resolver.h"
#include "Knifebot.h"
#include <intrin.h>
#include "EnginePrediction.h"
#include "MM.h"
#include "NetChannelhook.h"
static bool in_cm = false;

float last_time_got_impact = 0;

float IBasePlayer::GetOldSimulationTime()
{
	static int m_flSimulationTime = netvars.GetOffset(hs::DT_BaseEntity::s().c_str(), hs::m_flSimulationTime::s().c_str()) + 4;
	return *(float*)((DWORD)this + m_flSimulationTime);
}

void UpdateSimulationTime() {
	for (int i = 1; i < 65; i++)
	{
		auto ent = interfaces.ent_list->GetClientEntity(i);
		if (!ent)
			continue;
		if (ent->IsDormant() || !ent->isAlive() || !ent->IsPlayer())
			continue;
		if (csgo->old_simtime[ent->GetIndex()] != ent->GetSimulationTime())
			csgo->old_simtime[ent->GetIndex()] = ent->GetSimulationTime();
	}
}

int get_max_tickbase_shift()
{
	if (!csgo->local->isAlive())
		return 0;
	if (!CanDT())
		return csgo->game_rules->IsValveDS() ? 6 : 16;

	auto idx = csgo->local->GetWeapon()->GetItemDefinitionIndex();
	auto max_tickbase_shift = 0;

	switch (idx)
	{
	case WEAPON_M249:
	case WEAPON_MAC10:
	case WEAPON_P90:
	case WEAPON_MP5SD:
	case WEAPON_NEGEV:
	case WEAPON_MP9:
		max_tickbase_shift = 12;
		break;
	case WEAPON_ELITE:
	case WEAPON_UMP45:
	case WEAPON_BIZON:
	case WEAPON_TEC9:
	case WEAPON_MP7:
		max_tickbase_shift = 7;
		break;
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_CZ75A:
		max_tickbase_shift = 10;
		break;
	case WEAPON_FIVESEVEN:
	case WEAPON_GLOCK:
	case WEAPON_P250:
		max_tickbase_shift = 8;
	case WEAPON_SG553:
		max_tickbase_shift = 8;
		break;
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		max_tickbase_shift = 9;
		break;
	case WEAPON_DEAGLE:
		max_tickbase_shift = 13;
		break;
	case WEAPON_G3SG1:
		max_tickbase_shift = 15;
		break;
	case WEAPON_SCAR20:
		max_tickbase_shift = 15;
		break;
	case WEAPON_SSG08:
		max_tickbase_shift = 16;
		break;
	case WEAPON_AWP:
		max_tickbase_shift = 16;
		break;
	}

	if (max_tickbase_shift < 16) {
		if (vars.ragebot.BDT == 0)
		{
			max_tickbase_shift = clamp(max_tickbase_shift, 0, 13);
		}
		else if (vars.ragebot.BDT == 1)
		{
			max_tickbase_shift = clamp(max_tickbase_shift, 0, 14);
		}
		else if (vars.ragebot.BDT == 2)
		{
			max_tickbase_shift = clamp(max_tickbase_shift, 0, 15);
		}
	}




	return max_tickbase_shift;
}

std::string GetCurrentTag(std::string Text, std::vector<int> Indices) {
	std::string text_anim = "               " + Text + "                      " + Text + "               " + Text + "                      ";

	int i = interfaces.global_vars->tickcount / TIME_TO_TICKS(0.3f);
	i = floor(i % Indices.size());
	i = Indices[i + 1] + 1;

	return text_anim.substr(i, i + 15);
}

void DoClantag()
{
	if (vars.misc.chatspam > 0) {
		static int Lastsay = 0;
		if (vars.misc.chatspam == 1) {
			if (interfaces.global_vars->tickcount % 12 == 1) {
				std::string say;
				int isay = rand() % 11;
				if (Lastsay == isay) {
					Lastsay = rand() % 11;
				}
				else
				{
					Lastsay = isay;
				}
				switch (Lastsay) {
				case 0:
					say = "Refund your Paste";
					break;
				case 1:
					say = "Nice aimwhere";
					break;
				case 2:
					say = "What you using? everlose.cc ?";
					break;
				case 3:
					say = "Nice OneCrack";
					break;
				case 4:
					say = "Nice \"gamesense\"";
					break;
				case 5:
					say = "I think you need Ke Ji Ren CFG for that OTC";
					break;
				case 6:
					say = "Another Crack user";
					break;
				case 7:
					say = "Are you sad Skeet Crack was taken down? because you dont have skeet?";
					break;
				case 8:
					say = "OneTap can't kill me in 1 tap can it?";
					break;
				case 9:
					say = "Stop using Stupid Paste Cheats";
					break;
				case 10:
					say = "Aimwhere.net?";
					break;
				}
				interfaces.engine->ExecuteClientCmd(("say " + say).c_str());
			}
		}
		else if (vars.misc.chatspam == 2) {
			if (interfaces.global_vars->tickcount % 12 == 1) {
				std::string say;
				int isay = rand() % 11;
				if (Lastsay == isay) {
					Lastsay = rand() % 11;
				}
				else
				{
					Lastsay = isay;
				}
				switch (Lastsay) {
				case 0:
					say = "nn nice KD lmao";
					break;
				case 1:
					say = "stupid newfag trying to get kills";
					break;
				case 2:
					say = "public cheat newfag";
					break;
				case 3:
					say = "dog keeps reseting score big embarrasment";
					break;
				case 4:
					say = "nn dog sit, stop barking";
					break;
				case 5:
					say = "at least your not that trash, just KIDDING newfag";
					break;
				case 6:
					say = "user issue";
					break;
				case 7:
					say = "uid issue";
					break;
				case 8:
					say = "life issue, go suicide";
					break;
				case 9:
					say = "braing issue";
					break;
				case 10:
					say = "cheat issue, go use a real p2c newfag";
					break;
				}
				interfaces.engine->ExecuteClientCmd(("say " + say).c_str());
			}
		}
	}
	auto SetClanTag = [](const char* tag, const char* name)
	{
		static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>((DWORD)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
			hs::set_clantag::s().c_str())));
		pSetClanTag(tag, name);
	};



	//static bool Reset = false;
	static bool ShouldReset = false;
	std::string Prev;
	if (vars.visuals.clantagspammer)
		SetClanTag("FREZZYHOOK V4", "FREZZYHOOK");
	else
		SetClanTag("", "");
}





namespace MoveFixer
{
	void FixPistol() {
		if (!csgo->local || !csgo->local->isAlive() || !csgo->cmd || !csgo->weapon)
			return;

		if (csgo->local->GetFlags() & FL_FROZEN
			|| csgo->local->HasGunGameImmunity())
			return;

		if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER
			|| csgo->weapon->GetItemDefinitionIndex() == WEAPON_HEALTHSHOT
			|| !(csgo->weapon->isPistol() || csgo->weapon->IsHeavyPistol())
			|| csgo->weapon->IsNade()
			|| csgo->weapon->IsBomb()
			|| csgo->weapon->isAutoSniper())
			return;

		if (!Ragebot::Get().IsAbleToShoot()) {
			if (csgo->cmd->buttons & IN_ATTACK)
				csgo->cmd->buttons &= ~IN_ATTACK;
			
		}
	}
	void AutoPeekMove(float wish_yaw, CUserCmd* cmd)
	{
		cmd->buttons &= ~IN_RIGHT;
		cmd->buttons &= ~IN_MOVERIGHT;
		cmd->buttons &= ~IN_LEFT;
		cmd->buttons &= ~IN_MOVELEFT;
		cmd->buttons &= ~IN_FORWARD;
		cmd->buttons &= ~IN_BACK;



		static auto cl_forwardspeed = interfaces.cvars->FindVar("cl_forwardspeed");
		if (csgo->AutoPeek.ShouldGoBack) {
			auto difference = csgo->local->GetRenderOrigin() - csgo->AutoPeek.Position;
			if (difference.Length2D() > 5.0f)
			{
				auto velocity = Vector(difference.x * cos(wish_yaw / 180.0f * PI) + difference.y * sin(wish_yaw / 180.0f * PI), difference.y * cos(wish_yaw / 180.0f * PI) - difference.x * sin(wish_yaw / 180.0f * PI), difference.z);
				cmd->forwardmove = clamp(-velocity.x * 1000.0f,-cl_forwardspeed->GetFloat(), cl_forwardspeed->GetFloat());
				cmd->sidemove = clamp(velocity.y * 1000.0f, -cl_forwardspeed->GetFloat(), cl_forwardspeed->GetFloat());

			}
			else {
				csgo->AutoPeek.ShouldGoBack = false;
			}
		}

	}

	void QuickStop(CUserCmd* cmd) {
		auto wpn_info = csgo->weapon->GetCSWpnData();
		bool pressed_move_key = cmd->buttons & IN_FORWARD || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVERIGHT || cmd->buttons & IN_JUMP;
		if (!wpn_info)
			return;
		if (pressed_move_key)
			return;
		auto velocity = csgo->local->GetVelocity();
		float speed = velocity.Length2D();

		if (speed > 15.f) {
			Vector direction;
			Math::VectorAngles(velocity, direction);
			direction.y = csgo->original.y - direction.y;
			Vector forward;
			Math::AngleVectors(direction, forward);
			static const auto cl_sidespeed = interfaces.cvars->FindVar(hs::cl_sidespeed::s().c_str());
			Vector negated_direction = forward * -(cl_sidespeed->GetFloat());

			csgo->cmd->forwardmove = negated_direction.x;


			csgo->cmd->sidemove = negated_direction.y;

		}
	}
	void Run(Vector original)
	{
		/*
		Vector real_viewangles;
		interfaces.engine->GetViewAngles(real_viewangles);

		Vector vecMove(csgo->cmd->forwardmove, csgo->cmd->sidemove, csgo->cmd->upmove);
		float speed = sqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);

		Vector angMove;
		Math::VectorAngles(vecMove, angMove);

		float yaw = DEG2RAD(csgo->cmd->viewangles.y - original.y + angMove.y);

		csgo->cmd->forwardmove = cos(yaw) * speed;
		csgo->cmd->sidemove = sin(yaw) * speed;

		csgo->cmd->buttons &= ~IN_RIGHT;
		csgo->cmd->buttons &= ~IN_MOVERIGHT;
		csgo->cmd->buttons &= ~IN_LEFT;
		csgo->cmd->buttons &= ~IN_MOVELEFT;
		csgo->cmd->buttons &= ~IN_FORWARD;
		csgo->cmd->buttons &= ~IN_BACK;

		if (csgo->cmd->forwardmove > 0.f)
			csgo->cmd->buttons |= IN_FORWARD;
		else if (csgo->cmd->forwardmove < 0.f)
			csgo->cmd->buttons |= IN_BACK;

		if (csgo->cmd->sidemove > 0.f)
		{
			csgo->cmd->buttons |= IN_RIGHT;
			csgo->cmd->buttons |= IN_MOVERIGHT;
		}
		else if (csgo->cmd->sidemove < 0.f)
		{
			csgo->cmd->buttons |= IN_LEFT;
			csgo->cmd->buttons |= IN_MOVELEFT;
		}
		*/
		QuickStop(csgo->cmd);
		if (vars.misc.AutoPeek->active) {
			AutoPeekMove(original.y,csgo->cmd);
		}
		else {
			csgo->AutoPeek.ShouldGoBack = false;
		}
		Vector real_viewangles;
		interfaces.engine->GetViewAngles(real_viewangles);

		Vector vecMove(csgo->cmd->forwardmove, csgo->cmd->sidemove, csgo->cmd->upmove);
		float speed = sqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);

		Vector angMove;
		Math::VectorAngles(vecMove, angMove);

		float yaw = DEG2RAD(csgo->cmd->viewangles.y - original.y + angMove.y);

		csgo->cmd->forwardmove = cos(yaw) * speed;
		csgo->cmd->sidemove = sin(yaw) * speed;


		
		

		csgo->cmd->buttons &= ~IN_RIGHT;
		csgo->cmd->buttons &= ~IN_MOVERIGHT;
		csgo->cmd->buttons &= ~IN_LEFT;
		csgo->cmd->buttons &= ~IN_MOVELEFT;
		csgo->cmd->buttons &= ~IN_FORWARD;
		csgo->cmd->buttons &= ~IN_BACK;

		if (csgo->cmd->forwardmove > 0.f)
			csgo->cmd->buttons |= IN_FORWARD;
		else if (csgo->cmd->forwardmove < 0.f)
			csgo->cmd->buttons |= IN_BACK;

		if (csgo->cmd->sidemove > 0.f)
		{
			csgo->cmd->buttons |= IN_RIGHT;
			csgo->cmd->buttons |= IN_MOVERIGHT;
		}
		else if (csgo->cmd->sidemove < 0.f)
		{
			csgo->cmd->buttons |= IN_LEFT;
			csgo->cmd->buttons |= IN_MOVELEFT;
		}

		

		if (!(csgo->local->GetFlags() & FL_ONGROUND))
			return;


		auto cmd = csgo->cmd;
		if (vars.antiaim.Legs == 1) {
			if ((CMAntiAim::Get().shouldAA && vars.antiaim.Legs > 0))
			{
				if (cmd->forwardmove > 0.0f)
				{
					cmd->buttons |= IN_BACK;
					cmd->buttons &= ~IN_FORWARD;
				}
				else if (cmd->forwardmove < 0.0f)
				{
					cmd->buttons |= IN_FORWARD;
					cmd->buttons &= ~IN_BACK;
				}

				if (cmd->sidemove > 0.0f)
				{
					cmd->buttons |= IN_MOVELEFT;
					cmd->buttons &= ~IN_MOVERIGHT;
				}
				else if (cmd->sidemove < 0.0f)
				{
					cmd->buttons |= IN_MOVERIGHT;
					cmd->buttons &= ~IN_MOVELEFT;
				}
			}
			else
			{
				auto buttons = cmd->buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

				if (vars.antiaim.Legs > 0)
				{
					if (cmd->forwardmove <= 0.0f)
						buttons |= IN_BACK;
					else
						buttons |= IN_FORWARD;

					if (cmd->sidemove > 0.0f)
						goto LABEL_15;
					else if (cmd->sidemove >= 0.0f)
						goto LABEL_18;

					goto LABEL_17;
				}
				else
					goto LABEL_18;

				if (cmd->forwardmove <= 0.0f) //-V779
					buttons |= IN_FORWARD;
				else
					buttons |= IN_BACK;

				if (cmd->sidemove > 0.0f)
				{
				LABEL_17:
					buttons |= IN_MOVELEFT;
					goto LABEL_18;
				}

				if (cmd->sidemove < 0.0f)
					LABEL_15:

				buttons |= IN_MOVERIGHT;

			LABEL_18:
				cmd->buttons = buttons;
			}
		}
		else if (vars.antiaim.Legs == 2) {
			bool ri = csgo->need_recharge_rn;
			if ((CMAntiAim::Get().shouldAA && ri))
			{
				if (cmd->forwardmove > 0.0f)
				{
					cmd->buttons |= IN_BACK;
					cmd->buttons &= ~IN_FORWARD;
				}
				else if (cmd->forwardmove < 0.0f)
				{
					cmd->buttons |= IN_FORWARD;
					cmd->buttons &= ~IN_BACK;
				}

				if (cmd->sidemove > 0.0f)
				{
					cmd->buttons |= IN_MOVELEFT;
					cmd->buttons &= ~IN_MOVERIGHT;
				}
				else if (cmd->sidemove < 0.0f)
				{
					cmd->buttons |= IN_MOVERIGHT;
					cmd->buttons &= ~IN_MOVELEFT;
				}
			}
			else
			{
				auto buttons = cmd->buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

				if (ri)
				{
					if (cmd->forwardmove <= 0.0f)
						buttons |= IN_BACK;
					else
						buttons |= IN_FORWARD;

					if (cmd->sidemove > 0.0f)
						goto LABEL_15122;
					else if (cmd->sidemove >= 0.0f)
						goto LABEL_18122;

					goto LABEL_17122;
				}
				else
					goto LABEL_18122;

				if (cmd->forwardmove <= 0.0f) //-V779
					buttons |= IN_FORWARD;
				else
					buttons |= IN_BACK;

				if (cmd->sidemove > 0.0f)
				{
				LABEL_17122:
					buttons |= IN_MOVELEFT;
					goto LABEL_18122;
				}

				if (cmd->sidemove < 0.0f)
					LABEL_15122:

				buttons |= IN_MOVERIGHT;

			LABEL_18122:
				cmd->buttons = buttons;
			}
		}
	
		else {
			if ((CMAntiAim::Get().shouldAA && vars.antiaim.Legs > 0))
			{
				if (cmd->forwardmove > 0.0f)
				{
					cmd->buttons |= IN_BACK;
					cmd->buttons &= ~IN_FORWARD;
				}
				else if (cmd->forwardmove < 0.0f)
				{
					cmd->buttons |= IN_FORWARD;
					cmd->buttons &= ~IN_BACK;
				}

				if (cmd->sidemove > 0.0f)
				{
					cmd->buttons |= IN_MOVELEFT;
					cmd->buttons &= ~IN_MOVERIGHT;
				}
				else if (cmd->sidemove < 0.0f)
				{
					cmd->buttons |= IN_MOVERIGHT;
					cmd->buttons &= ~IN_MOVELEFT;
				}
			}
			else
			{
				auto buttons = cmd->buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

				if (vars.antiaim.Legs > 0)
				{
					if (cmd->forwardmove <= 0.0f)
						buttons |= IN_BACK;
					else
						buttons |= IN_FORWARD;

					if (cmd->sidemove > 0.0f)
						goto LABEL_152;
					else if (cmd->sidemove >= 0.0f)
						goto LABEL_182;

					goto LABEL_172;
				}
				else
					goto LABEL_182;

				if (cmd->forwardmove <= 0.0f) //-V779
					buttons |= IN_FORWARD;
				else
					buttons |= IN_BACK;

				if (cmd->sidemove > 0.0f)
				{
				LABEL_172:
					buttons |= IN_MOVELEFT;
					goto LABEL_182;
				}

				if (cmd->sidemove < 0.0f)
					LABEL_152:

				buttons |= IN_MOVERIGHT;

			LABEL_182:
				cmd->buttons = buttons;
			}
		}

	}
}

void ProcessMissedShots()
{
	if (shot_snapshots.size() == 0)
		return;
	auto& snapshot = shot_snapshots.front();
	if (fabs((csgo->local->GetTickBase() * interfaces.global_vars->interval_per_tick) - snapshot.time) > 1.f)
	{
		if (snapshot.weapon_fire) {
			string msg;
			msg += "missed " + snapshot.player->GetName();
			msg += "'s ";
			msg += snapshot.hitbox_where_shot;
			msg += " due to spread | aimed at " + snapshot.hitbox_where_shot + " | bt: " + to_string(snapshot.backtrack) + " | hc: " + to_string(clamp(snapshot.hitchance - rand() % 50, clamp(snapshot.hitchance, 10, 100), 100)) + " | dmg: " + to_string(snapshot.predicteddmg);
			Msg(msg, color_t(179, 66, 245));
		}
		shot_snapshots.erase(shot_snapshots.begin());
		return;
	}
	if (snapshot.first_processed_time != -1.f) {


		if (snapshot.damage == -1 && snapshot.weapon_fire && snapshot.bullet_impact && snapshot.record->player) {
			if (rand() % 8 == 1) {
				bool spread = false;
				if (snapshot.record->player) {
					const auto studio_model = interfaces.models.model_info->GetStudioModel(snapshot.record->player->GetModel());

					if (studio_model)
					{
						const auto angle = Math::CalculateAngle(snapshot.start, snapshot.impact);
						Vector forward;
						Math::AngleVectors(angle, forward);
						const auto end = snapshot.impact + forward * 2000.f;
						if (!CanHitHitbox(snapshot.start, end, snapshot.record, studio_model, snapshot.hitbox))
							spread = true;
					}
				}
				if (spread) {
					string msg;
					msg += "missed " + snapshot.player->GetName();
					msg += "'s ";
					msg += snapshot.hitbox_where_shot;
					msg += " due to resolver | aimed at " + snapshot.hitbox_where_shot + " | bt: " + to_string(snapshot.backtrack) + " | hc: " + to_string(snapshot.hitchance) + " | dmg: " + to_string(snapshot.predicteddmg);
					Msg(msg, color_t(179, 66, 245));
					shot_snapshots.erase(shot_snapshots.begin());
				}
				else {
					string msg;
					msg += "missed " + snapshot.player->GetName();
					msg += "'s ";
					msg += snapshot.hitbox_where_shot;
					msg += " due to spread | aimed at " + snapshot.hitbox_where_shot + " | bt: " + to_string(snapshot.backtrack) + " | hc: " + to_string(clamp(snapshot.hitchance - rand() % 50, clamp(snapshot.hitchance, 10, 100), 100)) + " | dmg: " + to_string(snapshot.predicteddmg);
					Msg(msg, color_t(179, 66, 245));
					shot_snapshots.erase(shot_snapshots.begin());
				}
			}
			else {
				string msg;
				msg += "missed " + snapshot.player->GetName();
				msg += "'s ";
				msg += snapshot.hitbox_where_shot;
				msg += " due to spread | aimed at " + snapshot.hitbox_where_shot + " | bt: " + to_string(snapshot.backtrack) + " | hc: " + to_string(clamp(snapshot.hitchance - rand() % 50, clamp(snapshot.hitchance, 10, 100), 100)) + " | dmg: " + to_string(snapshot.predicteddmg);
				Msg(msg, color_t(179, 66, 245));
				shot_snapshots.erase(shot_snapshots.begin());
			}
		}
	}
}

void FakeDuck(bool& send_packet)
{
	

	float maxLevel = 31.f;
	if (vars.antiaim.FLOK->active)
		send_packet = false;

	if (csgo->cmd->buttons & IN_JUMP || !(csgo->local->GetFlags() & FL_ONGROUND) || !vars.antiaim.enable) {
		csgo->fake_duck = false;
		return;
	}
	csgo->cmd->buttons |= IN_BULLRUSH;
	int Dchoke =/* csgo->game_rules->IsValveDS() ? 6 :*/ 14;
	static int P_FD2 = 0;
	static int P_FD = 0;
	switch (vars.antiaim.FD)
	{
	case 0:
		if (vars.antiaim.fakeduck->active)
		{
			csgo->fake_duck = true;

			if (csgo->client_state->iChokedCommands <= Dchoke / 2)
			{
				csgo->CSI_FD = true;
				csgo->cmd->buttons &= ~IN_DUCK;

				if (csgo->client_state->iChokedCommands > (Dchoke / 3) + 1)
					csgo->stand = true; // костыль с зевса чтобы по сприду не срать 
				else
					csgo->stand = false;
			}
			else
			{
				csgo->CSI_FD = false;
				csgo->cmd->buttons |= IN_DUCK;
				csgo->stand = false;

			}

			if (csgo->client_state->iChokedCommands < Dchoke)
				send_packet = false;								// choke
			else
				send_packet = true;
		}
		else {
			csgo->fake_duck = false;
			csgo->stand = false;
			csgo->CSI_FD = true;
		}
		break;
	case 1:
		if (vars.antiaim.fakeduck->active)
		{
			csgo->fake_duck = true;
			csgo->stand = true;
			if (P_FD < 80)
			{
				//csgo->cmd->buttons &= ~IN_DUCK;
				csgo->cmd->buttons |= IN_DUCK;
				send_packet = true;

				csgo->CSI_FD = false;
			}
			else if (P_FD > 75 && P_FD < 105)
			{
				send_packet = false;
				csgo->CSI_FD = false;
			}
			else if (P_FD > 80 && P_FD < 95)
			{
				send_packet = false;
				csgo->cmd->buttons &= ~IN_DUCK;
				csgo->CSI_FD = true;
			}
			else if (P_FD > 95)
			{
				csgo->cmd->buttons |= IN_DUCK;
				send_packet = false;
				csgo->CSI_FD = true;
			}
			if (P_FD > 115)
				P_FD = 0;

		}
		else {
			csgo->fake_duck = false;
			csgo->stand = false;
			csgo->CSI_FD = true;
		}
		P_FD += 2;
		break;
	case 2://mm fd




		if (vars.antiaim.fakeduck->active)
		{
			csgo->fake_duck = true;
			csgo->stand = true;
			static bool crouch = false;
			if (csgo->local->GetDuckAmount() <= maxLevel / 100)
			{
				crouch = true;
				send_packet = false;
			}
			//0.31
			if (csgo->local->GetDuckAmount() <= 0.326f)
			{
				csgo->CSI_FD = true;
			}
			else if (csgo->local->GetDuckAmount() > 0.326f)
			{
				csgo->CSI_FD = false;
			}
			if (csgo->local->GetDuckAmount() >= 0.8f)
			{
				crouch = false;
				send_packet = true;
			}

			if (crouch)
			{
				csgo->cmd->buttons |= IN_DUCK;
			}
			else
			{
				csgo->cmd->buttons |= IN_BULLRUSH;

			}
		}
		else {
			csgo->fake_duck = false;
			csgo->stand = false;
			csgo->CSI_FD = true;
		}

		break;
	case 3:
		if (vars.antiaim.fakeduck->active)
		{
			csgo->fake_duck = true;
			csgo->stand = true;
			static bool start = true;
			static int timer = 0;
			int main = vars.antiaim.FD_Spam;
			//csgo->cmd->buttons &= ~IN_DUCK;
			if (timer < 50)
			{
				csgo->cmd->buttons |= IN_DUCK;
				csgo->CSI_FD = false;
				if (vars.antiaim.GG)
					send_packet = true;
			}
			else
			{
				csgo->cmd->buttons &= ~IN_DUCK;
				csgo->CSI_FD = true;
				if (vars.antiaim.GG)
					send_packet = false;
			}
			if (timer == 100)
				timer = 0;

			timer += main;
		}
		else {
			csgo->fake_duck = false;
			csgo->stand = false;
			csgo->CSI_FD = true;
		}

		break;
	}
}

void FixRevolver()
{
	if (!csgo->weapon)
		return;

	if (csgo->weapon->GetItemDefinitionIndex() == 64 && csgo->weapon_struct.work && csgo->weapon->NextSecondaryAttack() == FLT_MAX)
		csgo->weapon->NextSecondaryAttack() = csgo->weapon_struct.next_attack;
}

float GetCurTime(CUserCmd* ucmd) {
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = csgo->local->GetTickBase();
	}
	else {
		// Required because prediction only runs on frames, not ticks
		// So if your framerate goes below tickrate, m_nTickBase won't update every tick
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces.global_vars->interval_per_tick;
	return curtime;
}

void __fastcall Hooked_RunCommand(void* ecx, void* edx, IBasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
{
	static auto RunCommand = g_pPredictHook->GetOriginal< RunCommandFn >(19);

	if (csgo->DoUnload || player == nullptr || csgo->local == nullptr || !csgo->local->isAlive())
		return RunCommand(ecx, player, ucmd, moveHelper);



	if (!interfaces.engine->IsConnected() || !interfaces.engine->IsInGame())
		return RunCommand(ecx, player, ucmd, moveHelper);

	if (ucmd->tick_count > interfaces.global_vars->tickcount + 72) //-V807
	{
		ucmd->hasbeenpredicted = true;
		player->SetAbsOrigin(player->GetOrigin());
		if (!interfaces.engine->IsPaused()) {

			if (ucmd->command_number == CMAntiAim::Get().cmd_tick) {
				player->GetTickBasePtr() = player->GetTickBase() - CMAntiAim::Get().base_tick;
				interfaces.global_vars->curtime = TICKS_TO_TIME(csgo->local->GetTickBasePtr());
			}
			player->GetTickBasePtr()++;
		}
		return;
	}
	if (ucmd->command_number == CMAntiAim::Get().cmd_tick) {
		player->GetTickBasePtr() = player->GetTickBase() - CMAntiAim::Get().base_tick;
		interfaces.global_vars->curtime = TICKS_TO_TIME(csgo->local->GetTickBasePtr());
	}

	RunCommand(ecx, player, ucmd, moveHelper);



	/*

	if (interfaces.engine->IsConnected() && interfaces.engine->IsInGame()) {
		if (player == csgo->local) {
			if (int(1.f / interfaces.global_vars->interval_per_tick) + csgo->unpdred_tick + 6 <= ucmd->tick_count)
			{
				ucmd->hasbeenpredicted = true;
				return;
			}

			float m_flVelModBackup = csgo->local->GetVelocityModifier();

			int m_nTickbase = csgo->local->GetTickBase();
			int m_flCurtime = interfaces.global_vars->curtime;

			if (ucmd->command_number == CMAntiAim::Get().cmd_tick) {
				csgo->local->GetTickBasePtr() = CMAntiAim::Get().base_tick - csgo->m_nTickbaseShift + csgo->client_state->iChokedCommands;
				++csgo->local->GetTickBasePtr();
				interfaces.global_vars->curtime = TICKS_TO_TIME(csgo->local->GetTickBasePtr());
			}


			if (csgo->ShouldUpdate && ucmd->command_number == csgo->client_state->nLastCommandAck + 1)
				csgo->local->GetVelocityModifier() = csgo->g_flVelMod;

			RunCommand(ecx, player, ucmd, moveHelper);

			if (!csgo->ShouldUpdate)
				csgo->local->GetVelocityModifier() = m_flVelModBackup;

			if (ucmd->command_number == CMAntiAim::Get().cmd_tick)
			{
				csgo->local->GetTickBasePtr() = m_nTickbase;
				interfaces.global_vars->curtime = m_flCurtime;
			}

		}
	}
	else
		RunCommand(ecx, player, ucmd, moveHelper);
		*/
}

bool __fastcall Hooked_InPrediction(CPrediction* prediction, uint32_t)
{
	static auto InPrediction = g_pPredictHook->GetOriginal< InPredictionFn >(14);
	return InPrediction(prediction);
//	if (csgo->DoUnload)
	//	return InPrediction(prediction);

	//static const auto return_to_maintain_sequence_transitions = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
	//	hs::seq_transition::s().c_str());

//	if (_ReturnAddress() == (uint32_t*)return_to_maintain_sequence_transitions)
	//	return false;

	return InPrediction(prediction);
}

void __fastcall Hooked_SetupMove(void* ecx, void* edx, IBasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper, void* pMoveData)
{
	using Fn = void(__thiscall*)(void*, IBasePlayer*, CUserCmd*, IMoveHelper*, void*);

	static auto SetupMove = g_pPredictHook->GetOriginal< Fn >(20);

	//if (&Prediction::Get().move_data != pMoveData && pMoveData != nullptr)
	//	memcpy(&Prediction::Get().move_data, pMoveData, 0x564);

	SetupMove(ecx, player, ucmd, moveHelper, pMoveData);
}

void chokelimit()
{
	static auto clMoveChokeClamp = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
		hs::choke_limit::s().c_str()) + 1;
	unsigned long protect = 0;
	VirtualProtect((void*)clMoveChokeClamp, 4, PAGE_EXECUTE_READWRITE, &protect);
	*(std::uint32_t*)clMoveChokeClamp = 62;
	VirtualProtect((void*)clMoveChokeClamp, 4, protect, &protect);
}

void WriteUsercmd(bf_write* buf, CUserCmd* in, CUserCmd* out)
{
	static DWORD WriteUsercmdF = (DWORD)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::write_user_cmd::s().c_str());

	__asm
	{
		mov ecx, buf
		mov edx, in
		push out
		call WriteUsercmdF
		add esp, 4
	}
}
bool Shifting = false;

void __cdecl CL_SendMove() {
	while (csgo->CLMOVEShift > 0) {
		H::oCLSendMove();
	}

	return H::oCLSendMove();
}
void __vectorcall CL_Move(float accumulated_extra_samples, bool bFinalTick)
{

	if (!csgo->local->isAlive()) {
		H::ClMove(accumulated_extra_samples, bFinalTick);
		return;
	}

	if (csgo->skip_ticks > 0) {
		csgo->cmd->buttons &= ~IN_ATTACK;
		csgo->cmd->sidemove = 0;
		csgo->cmd->forwardmove = 0;
		csgo->cmd->upmove = 0;
		
		csgo->local->GetTickBasePtr()++;

		csgo->cmd->tick_count = INT_MAX;
		csgo->send_packet = false;

		csgo->skip_ticks--;

		if (csgo->skip_ticks <= 0) {
			csgo->local->GetTickBasePtr()--; //final 1 tick fix
		}
	
		return;
	}

	//H::ClMove(accumulated_extra_samples, bFinalTick);

	while (csgo->CLMOVEShift > 0)
	{
	

		csgo->CLMOVEShift--;
		H::ClMove(accumulated_extra_samples, csgo->CLMOVEShift <= 0);
	
	}

	H::ClMove(accumulated_extra_samples, bFinalTick);
	/*
	if (!csgo->local || !csgo->local->isAlive())
		return H::ClMove(accumulated_extra_samples, bFinalTick);

	if (csgo->local->isAlive()) {

		if (!vars.antiaim.fakeduck->active) {

			while (csgo->m_nTickbaseShift > 0) {
				if (!(vars.ragebot.DoubletapFlags & 1)) {
					csgo->cmd->tick_count = INT_MAX;
				}
				csgo->shifting = true;
				csgo->send_packet = false;
				csgo->m_nTickbaseShift--;
				H::ClMove(accumulated_extra_samples, bFinalTick);
			}
			csgo->shifting = false;
		}
		
		while (csgo->CLMOVEShift > 0) {
			csgo->shifting = true;
			csgo->send_packet = false;
			csgo->CLMOVEShift--;
			H::ClMove(accumulated_extra_samples, bFinalTick);
			if (csgo->CLMOVEShift <= 0) {
				csgo->shifting = false;
				return;
			}
		}
		csgo->shifting = false;
		if (GetAsyncKeyState(VK_DELETE))
			return;

		if (csgo->skip_ticks > 0) {

			csgo->skip_ticks--;
			//csgo->cmd->tick_count = INT_MAX;
			if (csgo->skip_ticks == 0)
				csgo->dt_charged = true;

			return;
		}
	}
	return H::ClMove(accumulated_extra_samples, bFinalTick);
	*/
}

bool __fastcall Hooked_WriteUsercmdDeltaToBuffer(void* ecx, void*, int slot, bf_write* buf, int from, int to, bool isnewcommand)
{
	static auto original = g_pClientHook->GetOriginal <WriteUsercmdDeltaToBufferFn>(24);
	if (!csgo->local
		|| !interfaces.engine->IsConnected()
		|| !interfaces.engine->IsInGame()
		|| csgo->game_rules->IsFreezeTime()
		|| csgo->local->HasGunGameImmunity()
		|| csgo->local->GetFlags() & FL_FROZEN)
		return original(ecx, slot, buf, from, to, isnewcommand);

	if (!csgo->PDefensiveTicks)
		return original(ecx, slot, buf, from, to, isnewcommand);

	if (from != -1)
		return true;

	auto final_from = -1;

	uintptr_t frame_ptr;
	__asm mov frame_ptr, ebp;

	auto backup_commands = reinterpret_cast <int*> (frame_ptr + 0xFD8);
	auto new_commands = reinterpret_cast <int*> (frame_ptr + 0xFDC);

	auto newcmds = *new_commands;
	auto shift = csgo->PDefensiveTicks;

	csgo->PDefensiveTicks = 0;
	*backup_commands = 0;

	auto choked_modifier = newcmds + shift;

	if (choked_modifier > 62)
		choked_modifier = 62;

	*new_commands = choked_modifier;

	auto next_cmdnr = csgo->client_state->iChokedCommands + csgo->client_state->nLastOutgoingCommand + 1;
	auto final_to = next_cmdnr - newcmds + 1;

	if (final_to <= next_cmdnr)
	{
		while (original(ecx, slot, buf, final_from, final_to, true))
		{
			final_from = final_to++;

			if (final_to > next_cmdnr)
				goto next_cmd;
		}

		return false;
	}
next_cmd:

	auto user_cmd = interfaces.input->GetUserCmd(slot, final_from);

	if (!user_cmd)
		return true;

	CUserCmd to_cmd;
	CUserCmd from_cmd;

	from_cmd = *user_cmd;
	to_cmd = from_cmd;

	to_cmd.command_number++;
	to_cmd.tick_count += 200;

	if (newcmds > choked_modifier)
		return true;

	for (auto i = choked_modifier - newcmds + 1; i > 0; --i)
	{
		WriteUsercmd(buf, &to_cmd, &from_cmd);

		from_cmd = to_cmd;
		to_cmd.command_number++;
		to_cmd.tick_count++;
	}

	return true;
	/*
	static auto retn = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::engine_dll::s().c_str())].c_str()),
		hs::write_user_cmd_retn::s().c_str());

	if ((_ReturnAddress()) != retn || csgo->DoUnload)
		return ofunct(ecx, slot, buf, from, to, isnewcommand);

	if (interfaces.engine->IsConnected() && interfaces.engine->IsInGame()) {
		if (csgo->game_rules->IsFreezeTime())
			return ofunct(ecx, slot, buf, from, to, isnewcommand);

		if (csgo->PDefensiveTicks <= 0 || csgo->client_state->iChokedCommands > 3)
			return ofunct(ecx, slot, buf, from, to, isnewcommand);

		if (from != -1)
			return true;
		uintptr_t stackbase;
		__asm mov stackbase, ebp;
		CCLCMsg_Move_t* msg = reinterpret_cast<CCLCMsg_Move_t*>(stackbase + 0xFCC);
		auto net_channel = *reinterpret_cast <INetChannel**> (reinterpret_cast <uintptr_t> (csgo->client_state) + 0x9C);
		int32_t new_commands = msg->m_nNewCommands;

		int32_t next_cmdnr = csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands + 1;
		int32_t total_new_commands = std::clamp(csgo->PDefensiveTicks, 0, 16);
		csgo->PDefensiveTicks -= total_new_commands;

		from = -1;
		msg->m_nNewCommands = total_new_commands;
		msg->m_nBackupCommands = 0;

		for (to = next_cmdnr - new_commands + 1; to <= next_cmdnr; to++) {
			if (!ofunct(ecx, slot, buf, from, to, true))
				return false;

			from = to;
		}

		CUserCmd* last_realCmd = interfaces.input->GetUserCmd(slot, from);
		CUserCmd fromCmd;

		if (last_realCmd)
			fromCmd = *last_realCmd;

		CUserCmd toCmd = fromCmd;
		toCmd.command_number++;
		toCmd.tick_count++;

		for (int i = new_commands; i <= total_new_commands; i++) {
			WriteUsercmd(buf, &toCmd, &fromCmd);
			fromCmd = toCmd;
			toCmd.command_number++;
			toCmd.tick_count++;
		}
		return true;

	}
	else
		return ofunct(ecx, slot, buf, from, to, isnewcommand);
		*/
}

bool CMAntiAim::can_exploit(int tickbase_shift)
{
	auto weapon = csgo->weapon;

	if (!weapon)
		return false;

	const auto info = (weapon->GetCSWpnData());

	if (!(info))
		return false;

	float curtime = TICKS_TO_TIME(csgo->local->GetTickBase() - tickbase_shift);

	if (curtime < csgo->local->m_flNextAttack())
		return false;

	if (curtime < weapon->NextPrimaryAttack())
		return false;

	return true;
}
void CopyCommand(CUserCmd* cmd, int tickbase_shift)
{


	//vars.antiaim.DTT == 1


	if (vars.misc.AutoPeek->active) {
		MoveFixer::AutoPeekMove(csgo->original.y, cmd);
	}
	else if (csgo->LagExploit) {
		cmd->forwardmove = csgo->cmd->forwardmove;
		cmd->sidemove = csgo->cmd->sidemove;
	}
	else {

		if (vars.ragebot.DoubletapFlags == 0) {
			cmd->sidemove = 0;
			cmd->forwardmove = 0;
		}
		else {
			cmd->forwardmove = csgo->cmd->forwardmove;
			cmd->sidemove = csgo->cmd->sidemove;
		}
	}
	


	auto commands_to_add = 0;

	do
	{
		auto sequence_number = commands_to_add + cmd->command_number;

		auto command = interfaces.input->GetUserCmd(sequence_number);
		auto verified_command = interfaces.input->GetVerifiedCmd(sequence_number);

		memcpy(command, cmd, sizeof(CUserCmd));
		if (command->tick_count != INT_MAX && csgo->client_state->iDeltaTick > 0) {
			interfaces.prediction->Update(
				csgo->client_state->iDeltaTick, true,
				csgo->client_state->nLastCommandAck,
				csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands
			);
		//	csgo->local->GetTickBasePtr()--;
		}

		command->command_number = sequence_number;
		command->hasbeenpredicted = command->tick_count != INT_MAX;

		++csgo->client_state->iChokedCommands;

		if (csgo->client_state->pNetChannel)
		{
			++csgo->client_state->pNetChannel->iChokedPackets;
			++csgo->client_state->pNetChannel->iOutSequenceNr;
		}

		command->viewangles = Math::normalize(command->viewangles);

		memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd));
		verified_command->m_crc = command->GetChecksum();

		++commands_to_add;
	} while (commands_to_add != tickbase_shift);
	//csgo->local->GetTickBasePtr()++;
	interfaces.prediction->PreviousAckHadErrors = true;
	interfaces.prediction->CommandsPredicted = 0;

}

void Exploit(CUserCmd* cmd)
{
	static int LastDoubletap;
	static bool Recharged;
	static bool LastPeek = false;
	
	static auto ShiftTicks = [](int Shift) {
		/*
		if (vars.antiaim.DTT == 0 || vars.misc.AutoPeek->active) {
			csgo->CLMOVEShift = Shift;
		}
		else {
			auto next = csgo->cmd->command_number + 1;
			auto c = interfaces.input->GetUserCmd(next);

			memcpy(c, csgo->cmd, sizeof(CUserCmd));
			c->command_number = next;
			CopyCommand(c, Shift);
		}
		*/
		if (vars.misc.AutoPeek->active && vars.ragebot.double_tap->active) {
			csgo->CLMOVEShift = 15;
			return;
		}
		auto next = csgo->cmd->command_number + 1;
		auto c = interfaces.input->GetUserCmd(next);

		memcpy(c, csgo->cmd, sizeof(CUserCmd));
		c->command_number = next;
		CopyCommand(c, Shift);
		//csgo->CLMOVEShift = Shift;
	};
	static auto ResetData = [&]() {
		csgo->send_packet = true;
		if (Recharged) {
			csgo->LagExploit = true;
		//	ShiftTicks(15);
			csgo->CLMOVEShift = 15;
			csgo->LagExploit = false;
			LastDoubletap = 0;
		}
		Recharged = false;
		LastDoubletap = 0;
	};

	

	if (csgo->game_rules->IsFreezeTime()) {
		Recharged = false;
		csgo->LagExploit = false;
		LastDoubletap = 0;
		return;
	}



	if (!vars.ragebot.double_tap->active) {
		if (vars.ragebot.hideshots->active) {
			if ((csgo->weapon->IsCanScope() && !csgo->weapon->isRifle()) || csgo->weapon->IsHeavyPistol()) {
				if (vars.ragebot.hideshotmode == 1) {
					static int HideShotThing;
					if (++HideShotThing > 15) {
						HideShotThing = 0;
					}
					csgo->PDefensiveTicks = HideShotThing > 0 ? 13 : 0; //16
				}
				static bool LastShot = false;
				if (LastShot) {
					csgo->cmd->buttons &= ~IN_ATTACK;
				}
				if (Ragebot::Get().shot) {
					ShiftTicks(3);
					csgo->PDefensiveTicks = 3;
					LastShot = true;
				}
				else {
					LastShot = false;
				}
			}
		}
		ResetData();
		return;
	}

	if ((csgo->weapon->IsMiscWeapon() && !csgo->weapon->IsKnife()) && !csgo->weapon->IsZeus())
	{
	//	ResetData();
		return;
	}

	bool CanDefensive = vars.ragebot.LagExploitS > 0;

	if (csgo->skip_ticks > 0) {
		CanDefensive = false;
	}



	if (cmd->tick_count > LastDoubletap && !Recharged) {
		csgo->skip_ticks = 15;
		Recharged = true;
		return;
	}

	if ((cmd->buttons & IN_ATTACK || Ragebot::Get().hitchanced) && !Recharged) {
		LastDoubletap = cmd->tick_count + 33;
	}

	if (Ragebot::Get().shot && Recharged) {
		if(vars.misc.AutoPeek->active)
			csgo->AutoPeek.ShouldGoBack = true;
		ShiftTicks(get_max_tickbase_shift());
		CMAntiAim::Get().cmd_tick = csgo->cmd->command_number;
		CMAntiAim::Get().base_tick = get_max_tickbase_shift();
		LastDoubletap = cmd->tick_count + 33;
		Recharged = false;
	}

	static int DefensiveCounter; //peek check blablabla jsut test
	static int TicksToFuck = 0;
	if (vars.ragebot.LagExploitS == 0) {
		/*
		if (LastPeek != Ragebot::Get().hitchanced) {
			LastPeek = Ragebot::Get().hitchanced;
			int tickstoshoot = Ragebot::Get().GetTicksToShoot();
			int tickstostop = Ragebot::Get().GetTicksToStop();
			TicksToFuck = cmd->tick_count + (tickstoshoot > tickstostop ? tickstoshoot : tickstostop);
			
		}
		if (cmd->tick_count <= TicksToFuck) {
			csgo->PDefensiveTicks = 13;
		}
		*/
		if (LastPeek != Ragebot::Get().hitchanced) {
			LastPeek = Ragebot::Get().hitchanced;
			int CurrentShift = 2;
			int PredictedShift = Ragebot::Get().GetTicksToShoot() - 1;
			if (PredictedShift > CurrentShift) {
				CurrentShift = PredictedShift;
			}
			TicksToFuck = cmd->tick_count + CurrentShift;

		}
		if (cmd->tick_count <= TicksToFuck) {
			csgo->PDefensiveTicks = 13;
		}
	}

	if (CanDefensive) {
		if (vars.ragebot.LagExploitS == 1) {
			
				if (++DefensiveCounter > 15) {
					DefensiveCounter = 0;
				}
				csgo->PDefensiveTicks = DefensiveCounter > 0 ? 13 : 0; //16
			
		}
		else if (vars.ragebot.LagExploitS == 2) {
			int AppliedShift = 13;//13

			if (Ragebot::Get().aimbotted_in_current_tick) {
				DefensiveCounter++;
				AppliedShift = min(DefensiveCounter, 13);//14
			}
			else
				DefensiveCounter = 2;

			csgo->PDefensiveTicks = AppliedShift;
		}
		else {
			csgo->PDefensiveTicks = 13;//16
		}
	}


}


float LagFix()
{
	float updaterate = interfaces.cvars->FindVar("cl_updaterate")->GetFloat();
	ConVar* minupdate = interfaces.cvars->FindVar("sv_minupdaterate");
	ConVar* maxupdate = interfaces.cvars->FindVar("sv_maxupdaterate");

	if (minupdate && maxupdate)
		updaterate = maxupdate->GetFloat();

	float ratio = interfaces.cvars->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = interfaces.cvars->FindVar("cl_interp")->GetFloat();
	ConVar* cmin = interfaces.cvars->FindVar("sv_client_min_interp_ratio");
	ConVar* cmax = interfaces.cvars->FindVar("sv_client_max_interp_ratio");

	if (cmin && cmax && cmin->GetFloat() != 1)
		ratio = std::clamp(ratio, cmin->GetFloat(), cmax->GetFloat());

	return max(lerp, ratio / updaterate);
}



using newCreateMove_t = void(__thiscall*)(IBaseClientDll*, int, float, bool);

void __stdcall hooks_new_hooked_createmove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
{
	static auto original_fn = g_pClientHook->GetOriginal <newCreateMove_t>(22);
	original_fn(interfaces.client, sequence_number, input_sample_frametime, active);
	CUserCmd* m_pcmd = interfaces.input->GetUserCmdp(sequence_number);
	CVerifiedUserCmd* verified = interfaces.input->GetVerifiedCmd(sequence_number);

	if (!m_pcmd)
		return;

	if (!m_pcmd->command_number)
		return;
	//maybe need better validation
	//bSendPacket = true;

	bSendPacket = csgo->send_packet;
	verified->m_cmd = *m_pcmd;
	verified->m_crc = m_pcmd->GetChecksum();
}





bool __stdcall Hooked_CreateMove(float a, CUserCmd* cmd) {
	static auto CreateMove = g_pClientModeHook->GetOriginal< CreateMoveFn >(24);

	Ragebot::Get().DropTarget();



	if (!cmd || !cmd->command_number || !csgo->local || csgo->DoUnload)
		return CreateMove(interfaces.client_mode, a, cmd);

	csgo->cmd = cmd;
	in_cm = false;
	csgo->send_packet = true;
	csgo->original = cmd->viewangles;

	
	DoClantag();

	if (csgo->local->isAlive()) {
		if (csgo->skip_ticks > 0) {
		
			csgo->send_packet = false;
			csgo->cmd->tick_count = INT_MAX;
			return false;
			
		}
		/*
		if (csgo->NoTeleportRecharge > 0) {
			csgo->NoTeleportRecharge--;
			cmd->tick_count = INT_MAX;
			csgo->send_packet = true;
		}
		*/
		INetChannel* pNetChannel = (INetChannel*)csgo->client_state->pNetChannel;
		csgo->tick_rate = 1.f / interfaces.global_vars->interval_per_tick;

		csgo->vecUnpredictedVel = csgo->local->GetVelocity();

		in_cm = true;
		csgo->unpdred_tick = interfaces.global_vars->tickcount/*cmd->tick_count*/; // ekzi еблан тут надо с globalvars брать ибо он не меняется
		Ragebot::Get().DropTarget();
		//Legitbot::Get().Reset();


		if (vars.misc.bunnyhop)
			features->Bunnyhop->Run(csgo->original);

		csgo->weapon = csgo->local->GetWeapon();

		csgo->origin = csgo->local->GetRenderOrigin();

		csgo->unpred_eyepos = csgo->local->GetEyePosition();

		if (csgo->weapon)
		{
			csgo->g_bOverrideVelMod = true;

			if (csgo->g_flVelMod < 1.f)
				*(bool*)((uintptr_t)interfaces.prediction + 0x116) = true; //196


			csgo->weapon_struct.next_attack = csgo->weapon->NextSecondaryAttack();

			CMAntiAim::Get().Fakelag(csgo->send_packet);
			CEnginePrediction::Get().prediction_data.reset(); 
			CEnginePrediction::Get().setup();

			CEnginePrediction::Get().predict(cmd);

			{
		

				csgo->weapon_data = csgo->weapon->GetCSWpnData();
				if (vars.visuals.innacuracyoverlay && csgo->weapon->IsGun())
				{
					csgo->spread = csgo->weapon->GetSpread();
					csgo->innacuracy = csgo->weapon->GetInaccuracy();
					csgo->weaponspread = (csgo->spread + csgo->innacuracy) * 1000.f;
				}
				else
					csgo->weaponspread = 0.f;


				MoveFixer::FixPistol();

				if (vars.misc.knifebot)
					CKnifebot::Get().Run();

				if (vars.ragebot.enable)
					Ragebot::Get().Run();
			//	else if (vars.legitbot.enable)
				//	Legitbot::Get().Run(cmd);


				csgo->eyepos = csgo->local->GetEyePosition();

				ProcessMissedShots();
				if (vars.antiaim.enable)
				{
					FakeDuck(csgo->send_packet);
					CMAntiAim::Get().Run(csgo->send_packet);
					if (vars.antiaim.enable) {
						CMAntiAim::Get().Sidemove();
					}
				}

				if (csgo->cmd->buttons & IN_ATTACK)
				{
					if (auto net = interfaces.engine->GetNetChannelInfo(); net != nullptr)
						csgo->impact_time = interfaces.global_vars->curtime + net->GetLatency(MAX_FLOWS);
				}

				csgo->g_bOverrideVelMod = false;

				//if (csgo->client_state->iChokedCommands <= 3) {
				//}

			

		

				Exploit(cmd);
				/*
				if (vars.antiaim.DTT == 0 && csgo->CLMOVEShift > 0 && !vars.misc.AutoPeek->active && !csgo->LagExploit) {

					if (vars.ragebot.DoubletapFlags == 0) {
						static auto cl_sidespeed = interfaces.cvars->FindVar("cl_sidespeed");
						//reverse teleport
						Vector direction;
						Math::VectorAngles(csgo->local->GetVelocity(), direction);
						direction.y = csgo->original.y - direction.y;
						Vector forward;
						Math::AngleVectors(direction, forward);
						Vector negated_direction = forward * -(csgo->local->GetVelocity().Length2D() / 3.f);
						csgo->cmd->forwardmove = clamp(negated_direction.x, -cl_sidespeed->GetFloat(), cl_sidespeed->GetFloat());
						csgo->cmd->sidemove = clamp(negated_direction.y, -cl_sidespeed->GetFloat(), cl_sidespeed->GetFloat());
					}
					else if (vars.ragebot.DoubletapFlags == 1) {
						csgo->cmd->sidemove = 0;
						csgo->cmd->forwardmove = 0;
					}
				}*/

			}
			CEnginePrediction::Get().finish();

			MoveFixer::Run(csgo->original);
			/*
				auto& correct = csgo->c_data.emplace_front();

			correct.command_number = csgo->cmd->command_number;
			correct.choked_commands = csgo->client_state->iChokedCommands + 1;
			correct.tickcount = interfaces.global_vars->tickcount;

			if (csgo->send_packet)
				csgo->choked_number.clear();
			else
				csgo->choked_number.emplace_back(correct.command_number);

			while (csgo->c_data.size() > (int)(2.0f / interfaces.global_vars->interval_per_tick))
				csgo->c_data.pop_back();

			auto& out = csgo->packets.emplace_back();

			out.is_outgoing = csgo->send_packet;
			out.is_used = false;
			out.cmd_number = csgo->cmd->command_number;
			out.previous_command_number = 0;

			while (csgo->packets.size() > (int)(1.0f / interfaces.global_vars->interval_per_tick))
				csgo->packets.pop_front();

			if (!csgo->send_packet && !csgo->game_rules->IsValveDS())
			{
				auto net_channel = csgo->client_state->pNetChannel;

				if (net_channel->iChokedPackets > 0 && !(net_channel->iChokedPackets % 4))
				{
					auto backup_choke = net_channel->iChokedPackets;
					net_channel->iChokedPackets = 0;

					net_channel->SendDatagram();
					--net_channel->iOutSequenceNr;

					net_channel->iChokedPackets = backup_choke;
				}
			}
			*/
			CGrenadePrediction::Get().Tick(csgo->cmd->buttons);
		}

		csgo->cmd->viewangles = Math::normalize(csgo->cmd->viewangles);

		csgo->choke_meme = csgo->local->GetSimulationTime() != csgo->local->GetOldSimulationTime();

		if (csgo->send_packet) {
			csgo->FakeAngle = csgo->cmd->viewangles;
		}
		else {
			csgo->VisualAngle = csgo->cmd->viewangles;
		}
		if (vars.ragebot.resolver == 1) {
			resolver->StoreAntifreestand();
		}
		if (vars.misc.hold_firinganims)
		{
			if (Ragebot::Get().IsAbleToShoot() && csgo->cmd->buttons & IN_ATTACK)
				csgo->last_shot_angle = csgo->cmd->viewangles;
			if (Ragebot::Get().HoldFiringAnimation())
				csgo->FakeAngle = csgo->last_shot_angle;
		}

		static INetChannel* old_net_chan = nullptr;

		if (csgo->client_state != nullptr
			&& pNetChannel != nullptr
			&& csgo->local != nullptr && csgo->local->isAlive())
		{
			csgo->g_pNetChannelHook = std::make_unique<VMTHook>();
			csgo->g_pNetChannelHook->Setup(pNetChannel);
			csgo->g_pNetChannelHook->Hook(39, Hooked_ProcessPacket);
			csgo->g_pNetChannelHook->Hook(40, Hooked_SendNetMsg);
		//	csgo->g_pNetChannelHook->Hook(46, Hooked_SendDatagram);
		}
		if (csgo->local->isAlive()) {
			auto animstate = csgo->local->GetPlayerAnimState();
			if (animstate)
				csgo->desync_angle = animstate->m_fGoalFeetYaw;

			csgo->ping = interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + interfaces.engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
			csgo->ping *= 1000.f;
		}
	}
	else {
		csgo->ForceOffAA = false;
		csgo->send_packet = true;
		in_cm = false;
	}
	/*
	if (csgo->CLMOVEShift > 0 && vars.ragebot.double_tap->active && !vars.misc.AutoPeek->active)
		*(bool*)(*framePtr - 0x1C) = false;
	else
		*(bool*)(*framePtr - 0x1C) = csgo->send_packet;
		* */

	//*(bool*)(*framePtr - 0x34) = csgo->send_packet;
	g_Animfix->UpdateFakeState();
	return false;
}

int __stdcall Hooked_Camera(ConVar* var, void* return_address, void* eax)
{
	if (csgo->DoUnload)
		return 0;

	static auto return_to_modify_eye_pos = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnv::hash(hs::client_dll::s().c_str())].c_str()),
		hs::modify_eye_pos::s().c_str());

	const auto local = csgo->local;
	bool on_ground = false;
	auto animstate = local->GetPlayerAnimState();
	if (animstate)
		on_ground = animstate->m_landing;
	if (return_address == (void*)return_to_modify_eye_pos)
		return !in_cm || (local && !on_ground);

	return 0;
}

// ReSharper disable once CppDeclaratorNeverUsed
static uint32_t get_bool_retn_address = 0;

__declspec(naked) bool get_bool(void* ecx, void* edx)
{
	_asm
	{
		push eax

		mov eax, [esp + 4]
		mov get_bool_retn_address, eax

		push get_bool_retn_address
		push ecx
		call Hooked_Camera

		cmp eax, 0
		je _retn0

		cmp eax, 1
		je _retn1

		cmp eax, 2
		je _retn2

		_retn0 :
		mov al, 0
			retn

			_retn1 :
		pop eax

			mov eax, get_bool_retn_address
			add eax, 0x462

			push eax

			retn

			_retn2 :
		pop eax

			mov eax, get_bool_retn_address
			add eax, 0x462

			push eax

			retn
	}
}