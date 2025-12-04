#pragma once

enum class GameState {
	TITLE,
	LOBBY,
	PLAYING,
	GAME_OVER
};

GameState scene = GameState::TITLE;
