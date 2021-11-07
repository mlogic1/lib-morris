#pragma once

#include "IMorrisEventListener.h"
#include "MorrisField.h"
#include "MorrisGameState.h"
#include "MorrisPlayer.h"
#include "MorrisMarker.h"
#include <vector>

namespace Morris
{
	class MorrisGame
	{
	public:
		MorrisGame(IMorrisEventListener& morrisEventListener);
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
		IMorrisEventListener& m_morrisEventListener;
	};
}