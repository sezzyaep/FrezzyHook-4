#include "Glow.h"

void GlowESP::Draw()
{
	static int Optimization = 0;

	static CGlowObjectManager::GlowObjectDefinition_t* glow_object;
	for (auto i = 0; i < interfaces.glow_manager->size; i++)
	{
		if (Optimization != vars.visuals.op_strength + 1 && vars.visuals.optimize && vars.visuals.To_Op & 3)
			goto skip;
			
			glow_object = &interfaces.glow_manager->m_GlowObjectDefinitions[i];

			IBasePlayer* m_entity = glow_object->m_pEntity;

			if (!glow_object->m_pEntity || glow_object->IsUnused())
				continue;
			auto classid = m_entity->GetClientClass()->m_ClassID;
			if (classid == ClassId->CCSPlayer && vars.visuals.glow)
			{
				if (!m_entity->isAlive() || m_entity->IsDormant())
					continue;
				glow_object->m_bFullBloomRender = false;
				if (m_entity == csgo->local)
				{
					if (vars.visuals.local_glow && interfaces.input->m_fCameraInThirdPerson)
					{

						glow_object->m_vGlowcolor_t = Vector(
							(vars.visuals.local_glow_clr[0] / 255.f) * 0.38,
							(vars.visuals.local_glow_clr[1] / 255.f) * 0.38,
							(vars.visuals.local_glow_clr[2] / 255.f) * 0.38
						);
						if (vars.visuals.glowtype == 1)
							glow_object->m_bPulsatingChams = true;
						glow_object->m_flGlowAlpha = vars.visuals.local_glow_clr[3] / 255.f;
						glow_object->m_bRenderWhenOccluded = false;
						glow_object->m_bRenderWhenUnoccluded = true;
					}
					else
						continue;
				}

				if (m_entity->GetTeam() == csgo->local->GetTeam())
					continue;

				glow_object->m_vGlowcolor_t = Vector(
					(vars.visuals.glow_color[0] / 255.f) * 0.38,
					(vars.visuals.glow_color[1] / 255.f) * 0.38,
					(vars.visuals.glow_color[2] / 255.f) * 0.38
				);
				if (vars.visuals.glowtype == 1)
					glow_object->m_bPulsatingChams = true;
				glow_object->m_flGlowAlpha = vars.visuals.glow_color[3] / 255.f;
				glow_object->m_bRenderWhenOccluded = false;
				glow_object->m_bRenderWhenUnoccluded = true;
				continue;
			}
			else if (vars.visuals.GrenadeProximityWarning & 4 && (classid == (int)157
				|| classid == (int)48
				|| classid == (int)114
				|| classid == (int)9)) {

				glow_object->m_vGlowcolor_t = Vector(
					(vars.visuals.GrenadeColor[0] / 255.f),
					(vars.visuals.GrenadeColor[1] / 255.f),
					(vars.visuals.GrenadeColor[2] / 255.f)
				);

				glow_object->m_flGlowAlpha = vars.visuals.GrenadeColor[3] / 255.f;
				glow_object->m_bRenderWhenOccluded = false;
				glow_object->m_bRenderWhenUnoccluded = true;
			}
		

	}
	skip:
	if (Optimization > 3)
		Optimization = 0;
	Optimization++;
}

GlowESP* glow = new GlowESP();