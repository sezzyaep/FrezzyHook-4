#include "EnemyGPrediction.h"

Vector AGrenadePrediction::PredictNade(Vector Origin, Vector Velocity, GrenadePredictionType type) {
	GrenadePredictionReturn_t returnn;
	returnn.Predicted = Origin;
	Vector Push;
	Vector Last = Origin;
	for (unsigned int i = 0; i < returnn.Path.max_size() - 1; ++i) {
		Vector Gravity;
		PGravity(Gravity, Velocity, interfaces.global_vars->interval_per_tick, false);
		Vector Angle;
		Math::VectorAngles(Gravity - Origin, Angle);
		Math::AngleVectors(Angle, Push);
		returnn.Predicted = Origin + (Angle * 8.f);
		returnn.Path.push_back(returnn.Predicted);
		trace_t tr;
		PTraceHull(Last, returnn.Predicted, tr);
		Last = returnn.Predicted;
	}
	return returnn;
}