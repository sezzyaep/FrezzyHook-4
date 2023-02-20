#include "discord.h"
#include "disrpc/Includes/discord_rpc.h"

void Discord::Initialize()
{
    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize("795358898688753665", &Handle, 1, NULL);
}

void Discord::Update()
{
    DiscordRichPresence discord;
    memset(&discord, 0, sizeof(discord));
    discord.details = "Playing CS:GO";
    discord.state = "With FrezzyHook.xyz";
    discord.largeImageKey = "large1";
    discord.smallImageKey = "small2";
    Discord_UpdatePresence(&discord);
}