/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file game_state.h class for handling the game state variables. */

#ifndef GAME_STATE_H
#define GAME_STATE_H

/** Mode which defines the state of the game. */
enum GameMode {
	GM_MENU,
	GM_NORMAL,
	GM_EDITOR,
	GM_BOOTSTRAP
};

/** Mode which defines what mode we're switching to. */
enum SwitchMode {
	SM_NONE,
	SM_NEWGAME,         ///< New Game --> 'Random game'.
	SM_RESTARTGAME,     ///< Restart --> 'Random game' with current settings.
	SM_EDITOR,          ///< Switch to scenario editor.
	SM_LOAD_GAME,       ///< Load game, Play Scenario.
	SM_MENU,            ///< Switch to game intro menu.
	SM_SAVE_GAME,       ///< Save game.
	SM_SAVE_HEIGHTMAP,  ///< Save heightmap.
	SM_GENRANDLAND,     ///< Generate random land within scenario editor.
	SM_LOAD_SCENARIO,   ///< Load scenario from scenario editor.
	SM_START_HEIGHTMAP, ///< Load a heightmap and start a new game from it.
	SM_LOAD_HEIGHTMAP,  ///< Load heightmap from scenario editor.
};

extern GameMode _game_mode;
extern SwitchMode _switch_mode;
extern bool _exit_game;

void SwitchToMode(SwitchMode new_mode);

#endif /* GAME_STATE_H */
