#pragma once

#include "IMorrisEventListener.h"
#include "IMorrisLogger.h"
#include "MorrisField.h"
#include "MorrisGameState.h"
#include "MorrisPlayer.h"
#include "MorrisMarker.h"
#include <vector>

// TODO: something is wrong after eliminating a marker that formed a mill.

namespace Morris
{
	class MorrisGame
	{
	public:
		MorrisGame();
		MorrisGame(IMorrisEventListener* morrisEventListener, IMorrisLogger* logger = nullptr);
		~MorrisGame() = default;
		void ResetGame();

		void SubscribeToEvents(IMorrisEventListener* morrisEventListener);
		void UnsubscribeFromEvents(IMorrisEventListener* morrisEventListener);
		MorrisGameState GetGameState() const;
		MorrisPlayer GetCurrentPlayerTurn() const;
		const MorrisMarkerPtr GetMarkerAt(int pos) const;
		const std::vector<MorrisMarkerPtr>& GetUnplacedMarkers() const;
		
		bool PlaceMarketAtPoint(int pos, const MorrisMarkerPtr marker);
		bool MoveMarkerToPoint(int pos, const MorrisMarkerPtr marker);
		bool EliminateMarker(const MorrisMarkerPtr marker);
		bool CanMarkerBeEliminated(const MorrisMarkerPtr marker) const;

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
		std::vector<IMorrisEventListener*> m_morrisEventListeners;
		IMorrisLogger* m_morrisLogger = nullptr;

	#define TRIGGER_EVENT(callbackMethodName, ...) \
		for (IMorrisEventListener* listener : m_morrisEventListeners) \
			listener->callbackMethodName(__VA_ARGS__);

	private:
		void LogMessage(const std::string& message);
	};
}