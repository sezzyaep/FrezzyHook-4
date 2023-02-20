#pragma once
#include "hooks.h"
enum Prediction_stage
{
	SETUP,
	PREDICT,
	FINISH
};
#define ZZERO Vector(0,0,0)
class CEnginePrediction : public Singleton<CEnginePrediction>
{
public:
	struct Netvars_data
	{
		int tickbase = INT_MIN;

		Vector m_aimPunchAngle = ZZERO;
		Vector m_aimPunchAngleVel = ZZERO;
		Vector m_viewPunchAngle = ZZERO;
		Vector m_vecViewOffset = ZZERO;
	};

	struct Backup_data
	{
		int flags = 0;
		Vector velocity = ZZERO;
	};

	struct Prediction_data
	{
		void reset()
		{
			prediction_stage = SETUP;
			old_curtime = 0.0f;
			old_frametime = 0.0f;
		}

		Prediction_stage prediction_stage = SETUP;
		float old_curtime = 0.0f;
		float old_frametime = 0.0f;
		int* prediction_random_seed = nullptr;
		int* prediction_player = nullptr;
	};

	struct Viewmodel_data
	{
		IBaseCombatWeapon* weapon = nullptr;


		int sequence = 0;


		float cycle = 0.0f;
		float animation_time = 0.0f;
	};
public:
	Netvars_data netvars_data[150];
	Backup_data backup_data;
	Prediction_data prediction_data;
	Viewmodel_data viewmodel_data;

	void store_netvars();
	void restore_netvars();
	void setup();
	void predict(CUserCmd* m_pcmd);
	void finish();
};