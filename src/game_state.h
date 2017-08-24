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

/**
 * Singleton class for handling the current state of the game
 */
class GameState {
public:
	static GameState *GetInstance()
	{
		if (!instance) {
			instance = new GameState();
		}

		return instance;
	}

	virtual ~GameState()
	{
		delete instance;
	}

	/* Public accessors */
	virtual bool IsGameMode(GameMode mode);
	virtual void SetGameMode(GameMode new_mode);
	virtual void RestoreGameMode();
	virtual GameMode GetGameMode();
	virtual GameMode GetOldGameMode();

	virtual bool IsSwitchMode(SwitchMode mode);
	virtual void SetSwitchMode(SwitchMode new_mode);
	virtual void SwitchToMode(SwitchMode new_mode);
	virtual SwitchMode GetSwitchMode();

	virtual void ExitGame(bool exit);
	virtual bool ExitGame();

private:
	static GameState *instance;

	GameState() {}
	GameState(GameState const&);
	GameState &operator=(const GameState&);

	GameMode game_mode = GM_BOOTSTRAP;
	GameMode old_game_mode = GM_BOOTSTRAP;
	SwitchMode switch_mode = SM_NONE;

	bool exit_game = false;
};

#endif /* GAME_STATE_H */
