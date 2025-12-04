#pragma once

enum class GameState {
	TITLE,
	LOBBY,
	PLAYING,
	GAME_OVER
};

extern GameState scene;
