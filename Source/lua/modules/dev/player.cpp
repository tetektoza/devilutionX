#ifdef _DEBUG
#include "lua/modules/dev/player.hpp"

#include <cstdint>
#include <string>

#include <sol/sol.hpp>

#include "debug.h"
#include "engine/assets.hpp"
#include "lua/metadoc.hpp"
#include "lua/modules/dev/player/gold.hpp"
#include "lua/modules/dev/player/spells.hpp"
#include "lua/modules/dev/player/stats.hpp"
#include "player.h"

namespace devilution {

namespace {

std::string DebugCmdArrow(std::string_view effect)
{
	Player &myPlayer = *MyPlayer;

	myPlayer._pIFlags &= ~ItemSpecialEffect::FireArrows;
	myPlayer._pIFlags &= ~ItemSpecialEffect::LightningArrows;

	if (effect == "normal") {
		// we removed the parameter at the top
	} else if (effect == "fire") {
		myPlayer._pIFlags |= ItemSpecialEffect::FireArrows;
	} else if (effect == "lightning") {
		myPlayer._pIFlags |= ItemSpecialEffect::LightningArrows;
	} else if (effect == "spectral") {
		myPlayer._pIFlags |= (ItemSpecialEffect::FireArrows | ItemSpecialEffect::LightningArrows);
	} else {
		return "Invalid effect!";
	}

	return StrCat("Arrows changed to: ", effect);
}

std::string DebugCmdGodMode(std::optional<bool> on)
{
	DebugGodMode = on.value_or(!DebugGodMode);
	return StrCat("God mode: ", DebugGodMode ? "On" : "Off");
}

std::string DebugCmdPlayerInfo(std::optional<uint8_t> id)
{
	const uint8_t playerId = id.value_or(0);
	if (playerId >= Players.size())
		return StrCat("Invalid player ID (max: ", Players.size() - 1, ")");
	Player &player = Players[playerId];
	if (!player.plractive)
		return StrCat("Player ", playerId, " is not active!");

	const Point target = player.GetTargetPosition();
	return StrCat("Plr ", playerId, " is ", player._pName,
	    "\nLvl: ", player.plrlevel, " Changing: ", player._pLvlChanging,
	    "\nTile.x: ", player.position.tile.x, " Tile.y: ", player.position.tile.y, " Target.x: ", target.x, " Target.y: ", target.y,
	    "\nMode: ", player._pmode, " destAction: ", player.destAction, " walkpath[0]: ", player.walkpath[0],
	    "\nInvincible: ", player._pInvincible ? 1 : 0, " HitPoints: ", player._pHitPoints);
}

std::string DebugSetPlayerTrn(std::string_view path)
{
	if (!path.empty()) {
		if (const AssetRef ref = FindAsset(path); !ref.ok()) {
			const char *error = ref.error();
			return error == nullptr || *error == '\0' ? StrCat("File not found: ", path) : error;
		}
	}
	debugTRN = path;
	Player &player = *MyPlayer;
	InitPlayerGFX(player);
	StartStand(player, player._pdir);
	return path.empty() ? "TRN unset" : "TRN set";
}

sol::table LuaDevPlayerTrnModule(sol::state_view &lua)
{
	sol::table table = lua.create_table();
	SetDocumented(table, "mon", "(name: string)", "Set player TRN to monsters\\monsters\\${name}.trn",
	    [](std::string_view name) { return DebugSetPlayerTrn(StrCat("monsters\\monsters\\", name, ".trn")); });
	SetDocumented(table, "plr", "(name: string)", "Set player TRN to plrgfx\\${name}.trn",
	    [](std::string_view name) { return DebugSetPlayerTrn(StrCat("plrgfx\\", name, ".trn")); });
	SetDocumented(table, "clear", "()", "Unset player TRN",
	    []() { return DebugSetPlayerTrn(""); });
	return table;
}

} // namespace

sol::table LuaDevPlayerModule(sol::state_view &lua)
{
	sol::table table = lua.create_table();
	SetDocumented(table, "arrow", "(effect: 'normal'|'fire'|'lightning'|'explosion')", "Set arrow effect.", &DebugCmdArrow);
	SetDocumented(table, "god", "(on: boolean = nil)", "Toggle god mode.", &DebugCmdGodMode);
	SetDocumented(table, "gold", "", "Adjust player gold.", LuaDevPlayerGoldModule(lua));
	SetDocumented(table, "info", "(id: number = 0)", "Show player info.", &DebugCmdPlayerInfo);
	SetDocumented(table, "spells", "", "Adjust player spells.", LuaDevPlayerSpellsModule(lua));
	SetDocumented(table, "stats", "", "Adjust player stats (Strength, HP, etc).", LuaDevPlayerStatsModule(lua));
	SetDocumented(table, "trn", "", "Set player TRN to '${name}.trn'", LuaDevPlayerTrnModule(lua));
	return table;
}

} // namespace devilution
#endif // _DEBUG
