#pragma once

#include "MorrisField.h"
#include "MorrisGameState.h"
#include "MorrisPlayer.h"
#include "MorrisMarker.h"

#include <functional>
#include <vector>

namespace Morris
{
	class MorrisGame
	{
	public:
		MorrisGame();
		MorrisGame(
			std::function<void(MorrisPlayer)> playerTurnChangedCallback,
			std::function<void(MorrisGameState, MorrisGameState)> gameStateChangedCallback,
			std::function<void(MorrisPlayer)> playerWonCallback,
			std::function<void(const MorrisMarkerPtr)> markerEliminatedCallback,
			std::function<void(int, const MorrisMarkerPtr)> markerPlacedCallback,
			std::function<void(int, const MorrisMarkerPtr)> markerMovedCallback);
		~MorrisGame() = default;
		void ResetGame();

		MorrisGameState GetGameState() const;
		MorrisPlayer GetCurrentPlayerTurn() const;
		const MorrisMarkerPtr GetMarkerAt(int pos) const;
		const std::vector<MorrisMarkerPtr>& GetUnplacedMarkers() const;
		
		bool PlaceMarketAtPoint(int pos, const MorrisMarkerPtr marker);
		bool MoveMarkerToPoint(int pos, const MorrisMarkerPtr marker);
		bool EliminateMarker(const MorrisMarkerPtr marker);

	private:
		bool CanPlayerMakeAMove(MorrisPlayer player) const;
		void ChangePlayerTurn();
		void AfterMoveLogic(const MorrisMarkerPtr& marker);

	private:
		MorrisField _gameField;
		MorrisGameState _gameState = MorrisGameState::Playing;
		MorrisPlayer _currentPlayerTurn = MorrisPlayer::Player1;

		std::vector<MorrisMarkerPtr> _unplacedMarkers;
		std::vector<MorrisMarkerPtr> _placedMarkers;
		std::vector<MorrisMarkerPtr> _eliminatedMakers;

	private:
		// callbacks
		std::function<void(MorrisPlayer)> _playerTurnChangedCallback;
		std::function<void(MorrisGameState, MorrisGameState)> _gameStateChangedCallback;
		std::function<void(MorrisPlayer)> _playerWonCallback;

		std::function<void(const MorrisMarkerPtr)> _markerEliminatedCallback;
		std::function<void(int, const MorrisMarkerPtr)> _markerPlacedCallback;
		std::function<void(int, const MorrisMarkerPtr)> _markerMovedCallback;

		// callback wrapper
		template<typename ... Args>
		void TriggerCallback(std::function<void(Args...)>& func, Args ... params)
		{
			if (func != nullptr)
				func(params...);
		}
	};
}