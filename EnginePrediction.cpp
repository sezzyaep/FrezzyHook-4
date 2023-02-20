#include "EnginePrediction.h"
#include "checksum_md5.h"
#include "AntiAims.h"

void CEnginePrediction::store_netvars()
{
	auto data = &netvars_data[csgo->client_state->m_command_ack % 150]; //-V807

	data->tickbase = csgo->local->GetTickBasePtr(); //-V807
	data->m_aimPunchAngle = *csgo->local->GetPunchAnglePtr();
	data->m_aimPunchAngleVel = csgo->local->GetPunchAngleVel();
	data->m_viewPunchAngle = *csgo->local->GetViewPunchAnglePtr();
	data->m_vecViewOffset = csgo->local->GetVecViewOffset();
}

void CEnginePrediction::restore_netvars()
{
	auto data = &netvars_data[(csgo->client_state->m_command_ack - 1) % 150]; //-V807

	if (data->tickbase != csgo->local->GetTickBasePtr()) //-V807
		return;

	auto aim_punch_angle_delta = (*csgo->local->GetPunchAnglePtr()) - data->m_aimPunchAngle;
	auto aim_punch_angle_vel_delta = csgo->local->GetPunchAngleVel() - data->m_aimPunchAngleVel;
	auto view_punch_angle_delta = (*csgo->local->GetViewPunchAnglePtr()) - data->m_viewPunchAngle;
	auto view_offset_delta = csgo->local->GetVecViewOffset() - data->m_vecViewOffset;

	if (fabs(aim_punch_angle_delta.x) < 0.02125f && fabs(aim_punch_angle_delta.y) < 0.02125f && fabs(aim_punch_angle_delta.z) < 0.02125f)
		*csgo->local->GetPunchAnglePtr() = data->m_aimPunchAngle;

	if (fabs(aim_punch_angle_vel_delta.x) < 0.03125f && fabs(aim_punch_angle_vel_delta.y) < 0.03125f && fabs(aim_punch_angle_vel_delta.z) < 0.02125f)
		csgo->local->GetPunchAngleVel() = data->m_aimPunchAngleVel;

	if (fabs(view_punch_angle_delta.x) < 0.02125f && fabs(view_punch_angle_delta.y) < 0.02125f && fabs(view_punch_angle_delta.z) < 0.02125f)
		*csgo->local->GetViewPunchAnglePtr() = data->m_viewPunchAngle;

	if (fabs(view_offset_delta.x) < 0.02125f && fabs(view_offset_delta.y) < 0.02125f && fabs(view_offset_delta.z) < 0.02125f)
		csgo->local->GetVecViewOffset() = data->m_vecViewOffset;
}

void CEnginePrediction::setup()
{
	if (prediction_data.prediction_stage != SETUP)
		return;

	backup_data.flags = csgo->local->GetFlags(); //-V807
	backup_data.velocity = csgo->local->GetVelocity();

	prediction_data.old_curtime = interfaces.global_vars->curtime; //-V807
	prediction_data.old_frametime = interfaces.global_vars->frametime;

	interfaces.global_vars->curtime = TICKS_TO_TIME(csgo->local->GetTickBasePtr());
	interfaces.global_vars->frametime = interfaces.prediction->bEnginePaused ? 0.0f : interfaces.global_vars->interval_per_tick;

	prediction_data.prediction_stage = PREDICT;
}

void CEnginePrediction::predict(CUserCmd* m_pcmd)
{
	if (prediction_data.prediction_stage != PREDICT)
		return;

	if (csgo->client_state->iDeltaTick > 0)  //-V807
		interfaces.prediction->Update(csgo->client_state->iDeltaTick, true, csgo->client_state->nLastCommandAck, csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands);

	if (!prediction_data.prediction_random_seed)
		prediction_data.prediction_random_seed = *reinterpret_cast <int**> (csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), ("A3 ? ? ? ? 66 0F 6E 86")) + 0x1);

	*prediction_data.prediction_random_seed = MD5_PseudoRandom(m_pcmd->command_number) & INT_MAX;

	if (!prediction_data.prediction_player)
		prediction_data.prediction_player = *reinterpret_cast <int**> (csgo->Utils.FindPatternIDA(GetModuleHandleA("client.dll"), ("89 35 ? ? ? ? F3 0F 10 48")) + 0x2);

	*prediction_data.prediction_player = reinterpret_cast <int> (csgo->local);

	interfaces.game_movement->StartTrackPredictionErrors(csgo->local); //-V807
	interfaces.move_helper->SetHost(csgo->local);

	CMoveData move_data;
	memset(&move_data, 1, sizeof(CMoveData));

	interfaces.prediction->SetupMove(csgo->local, m_pcmd, interfaces.move_helper, &move_data);
	interfaces.game_movement->ProcessMovement(csgo->local, &move_data);
	interfaces.prediction->FinishMove(csgo->local, m_pcmd, &move_data);

	interfaces.game_movement->FinishTrackPredictionErrors(csgo->local);
	interfaces.move_helper->SetHost(nullptr);

	auto viewmodel = csgo->local->GetViewModel();

	if (viewmodel)
	{
		viewmodel_data.weapon = viewmodel->GetWeapon();

	
		viewmodel_data.sequence = viewmodel->GetSequence();


		viewmodel_data.cycle = viewmodel->GetCycle();
		viewmodel_data.animation_time = viewmodel->GetAnimtime();
	}

	prediction_data.prediction_stage = FINISH;
}

void CEnginePrediction::finish()
{
	if (prediction_data.prediction_stage != FINISH)
		return;

	*prediction_data.prediction_random_seed = -1;
	*prediction_data.prediction_player = 1;

	interfaces.global_vars->curtime = prediction_data.old_curtime;
	interfaces.global_vars->frametime = prediction_data.old_frametime;
}
/*
void CEnginePrediction::Start(CUserCmd* cmd, IBasePlayer* local) {
	old_vars.curtime = interfaces.global_vars->curtime;
	old_vars.frametime = interfaces.global_vars->frametime;
	old_vars.tickcount = interfaces.global_vars->tickcount;

	interfaces.global_vars->curtime = TICKS_TO_TIME(local->GetTickBase());
	interfaces.global_vars->frametime = interfaces.global_vars->interval_per_tick;
	interfaces.global_vars->tickcount = TIME_TO_TICKS(interfaces.global_vars->curtime);

	interfaces.game_movement->StartTrackPredictionErrors(local);
	interfaces.move_helper->SetHost(local);

	memset(&data, 0, sizeof(data));

	interfaces.prediction->SetupMove(local, cmd, interfaces.move_helper, &data);
	interfaces.game_movement->ProcessMovement(local, &data);
	interfaces.prediction->FinishMove(local, cmd, &data);
}

void CEnginePrediction::Finish(IBasePlayer* local) {
	interfaces.game_movement->FinishTrackPredictionErrors(local);
	interfaces.move_helper->SetHost(nullptr);

	interfaces.global_vars->curtime = old_vars.curtime;
	interfaces.global_vars->frametime = old_vars.frametime;
}*/