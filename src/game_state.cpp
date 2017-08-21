/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file game_state.cpp Functions related to the game state. */

#include "stdafx.h"
#include "game_state.h"

#include "ai/ai.hpp"
#include "base_media_base.h"
#include "command_func.h"
#include "company_func.h"
#include "console_func.h"
#include "date_func.h"
#include "engine_base.h"
#include "error.h"
#include "genworld.h"
#include "network/network.h"
#include "network/network_func.h"
#include "openttd.h"
#include "saveload/saveload.h"
#include "screenshot.h"
#include "settings_func.h"
#include "settings_type.h"
#include "string_func.h"
#include "window_func.h"

extern void LoadIntroGame(bool load_newgrfs = false);
extern void MakeNewGame(bool from_heightmap, bool reset_settings);
extern void MakeNewgameSettingsLive();
extern void MakeNewEditorWorld();
extern bool SafeLoad(const char *filename, SaveLoadOperation fop, DetailedFileType dft, GameMode newgm, Subdirectory subdir, struct LoadFilter *lf = NULL);

void SwitchToMode(SwitchMode new_mode)
{
#ifdef ENABLE_NETWORK
	/* If we are saving something, the network stays in his current state */
	if (new_mode != SM_SAVE_GAME) {
		/* If the network is active, make it not-active */
		if (_networking) {
			if (_network_server && (new_mode == SM_LOAD_GAME || new_mode == SM_NEWGAME || new_mode == SM_RESTARTGAME)) {
				NetworkReboot();
			} else {
				NetworkDisconnect();
			}
		}

		/* If we are a server, we restart the server */
		if (_is_network_server) {
			/* But not if we are going to the menu */
			if (new_mode != SM_MENU) {
				/* check if we should reload the config */
				if (_settings_client.network.reload_cfg) {
					LoadFromConfig();
					MakeNewgameSettingsLive();
					ResetGRFConfig(false);
				}
				NetworkServerStart();
			} else {
				/* This client no longer wants to be a network-server */
				_is_network_server = false;
			}
		}
	}
#endif /* ENABLE_NETWORK */
	/* Make sure all AI controllers are gone at quitting game */
	if (new_mode != SM_SAVE_GAME) AI::KillAll();

	switch (new_mode) {
		case SM_EDITOR: // Switch to scenario editor
			MakeNewEditorWorld();
			break;

		case SM_RESTARTGAME: // Restart --> 'Random game' with current settings
		case SM_NEWGAME: // New Game --> 'Random game'
#ifdef ENABLE_NETWORK
			if (_network_server) {
				seprintf(_network_game_info.map_name, lastof(_network_game_info.map_name), "Random Map");
			}
#endif /* ENABLE_NETWORK */
			MakeNewGame(false, new_mode == SM_NEWGAME);
			break;

		case SM_LOAD_GAME: { // Load game, Play Scenario
			ResetGRFConfig(true);
			ResetWindowSystem();

			if (!SafeLoad(_file_to_saveload.name, _file_to_saveload.file_op, _file_to_saveload.detail_ftype, GM_NORMAL, NO_DIRECTORY)) {
				SetDParamStr(0, GetSaveLoadErrorString());
				ShowErrorMessage(STR_JUST_RAW_STRING, INVALID_STRING_ID, WL_ERROR);
			} else {
				if (_file_to_saveload.abstract_ftype == FT_SCENARIO) {
					/* Reset engine pool to simplify changing engine NewGRFs in scenario editor. */
					EngineOverrideManager::ResetToCurrentNewGRFConfig();
				}
				/* Update the local company for a loaded game. It is either always
				 * company #1 (eg 0) or in the case of a dedicated server a spectator */
				SetLocalCompany(_network_dedicated ? COMPANY_SPECTATOR : COMPANY_FIRST);
				/* Execute the game-start script */
				IConsoleCmdExec("exec scripts/game_start.scr 0");
				/* Decrease pause counter (was increased from opening load dialog) */
				DoCommandP(0, PM_PAUSED_SAVELOAD, 0, CMD_PAUSE);
#ifdef ENABLE_NETWORK
				if (_network_server) {
					seprintf(_network_game_info.map_name, lastof(_network_game_info.map_name), "%s (Loaded game)", _file_to_saveload.title);
				}
#endif /* ENABLE_NETWORK */
			}
			break;
		}

		case SM_START_HEIGHTMAP: // Load a heightmap and start a new game from it
#ifdef ENABLE_NETWORK
			if (_network_server) {
				seprintf(_network_game_info.map_name, lastof(_network_game_info.map_name), "%s (Heightmap)", _file_to_saveload.title);
			}
#endif /* ENABLE_NETWORK */
			MakeNewGame(true, true);
			break;

		case SM_LOAD_HEIGHTMAP: // Load heightmap from scenario editor
			SetLocalCompany(OWNER_NONE);

			GenerateWorld(GWM_HEIGHTMAP, 1 << _settings_game.game_creation.map_x, 1 << _settings_game.game_creation.map_y);
			MarkWholeScreenDirty();
			break;

		case SM_LOAD_SCENARIO: { // Load scenario from scenario editor
			if (SafeLoad(_file_to_saveload.name, _file_to_saveload.file_op, _file_to_saveload.detail_ftype, GM_EDITOR, NO_DIRECTORY)) {
				SetLocalCompany(OWNER_NONE);
				_settings_newgame.game_creation.starting_year = _cur_year;
				/* Cancel the saveload pausing */
				DoCommandP(0, PM_PAUSED_SAVELOAD, 0, CMD_PAUSE);
			} else {
				SetDParamStr(0, GetSaveLoadErrorString());
				ShowErrorMessage(STR_JUST_RAW_STRING, INVALID_STRING_ID, WL_ERROR);
			}
			break;
		}

		case SM_MENU: // Switch to game intro menu
			LoadIntroGame();
			if (BaseSounds::ini_set == NULL && BaseSounds::GetUsedSet()->fallback) {
				ShowErrorMessage(STR_WARNING_FALLBACK_SOUNDSET, INVALID_STRING_ID, WL_CRITICAL);
				BaseSounds::ini_set = stredup(BaseSounds::GetUsedSet()->name);
			}
			break;

		case SM_SAVE_GAME: // Save game.
			/* Make network saved games on pause compatible to singleplayer */
			if (SaveOrLoad(_file_to_saveload.name, SLO_SAVE, DFT_GAME_FILE, NO_DIRECTORY) != SL_OK) {
				SetDParamStr(0, GetSaveLoadErrorString());
				ShowErrorMessage(STR_JUST_RAW_STRING, INVALID_STRING_ID, WL_ERROR);
			} else {
				DeleteWindowById(WC_SAVELOAD, 0);
			}
			break;

		case SM_SAVE_HEIGHTMAP: // Save heightmap.
			MakeHeightmapScreenshot(_file_to_saveload.name);
			DeleteWindowById(WC_SAVELOAD, 0);
			break;

		case SM_GENRANDLAND: // Generate random land within scenario editor
			SetLocalCompany(OWNER_NONE);
			GenerateWorld(GWM_RANDOM, 1 << _settings_game.game_creation.map_x, 1 << _settings_game.game_creation.map_y);
			/* XXX: set date */
			MarkWholeScreenDirty();
			break;

		default: NOT_REACHED();
	}
}