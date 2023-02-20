#include "rcp.h"
#include "Register.h"
#include "CommonRPC.h"
#pragma comment(lib, "discord-rpc.lib")
#include <iostream>;
#include <time.h>
#include <wtypes.h>
using namespace std;
DiscordRichPresence discordPresence;
const char* token = "879582422700806145"; //not necessary
void DiscordRPC::Update()
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = "State";
	discordPresence.details = "Details";
	discordPresence.startTimestamp = time(0); //initlialize time
	discordPresence.largeImageKey = "csgo"; //large image file name no extension
	discordPresence.largeImageText = "Playing CS:GO";
	discordPresence.smallImageKey = "infinite"; //same as large
	discordPresence.smallImageText = "Using Infinite.tech"; //displays on hover
	Discord_UpdatePresence(&discordPresence); //do the do
}

void DiscordRPC::Initialize()
{
	DiscordEventHandlers handlers;
	std::memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize("879582422700806145", &handlers, TRUE, nullptr);
	Update();
}

void Shutdown()
{
	Discord_Shutdown(); //goodbye
}

