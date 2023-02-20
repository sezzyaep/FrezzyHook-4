#include "Hooks.h"
#include "RageBackTracking.h"
#include "Ragebot.h"
#include "Resolver.h"
#include "AntiAims.h"
CMBacktracking* g_Backtrack = new CMBacktracking();

void CMBacktracking::DoPingSpike(INetChannel* net_channel)
{
	if (!csgo->local)
		return;

	if (vars.misc.fakeping <= 0.f)
		return;

	auto ping_value = vars.misc.fakeping / 1000.f;
	INetChannelInfo* nci = interfaces.engine->GetNetChannelInfo();
	if (nci)
	{
		ping_value -= nci->GetLatency(FLOW_OUTGOING);
	}
	static auto oldseqnum = 0;

	if (oldseqnum == net_channel->iInSequenceNr)
		return;

	oldseqnum = net_channel->iInSequenceNr;

	net_channel->iInSequenceNr += 126 - static_cast<uint32_t>(63 * (ping_value));
}