#include <MorrisGame.h>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>

namespace Morris
{
	MorrisGame::MorrisGame()
	{
		ResetGame();
	}

	MorrisGame::MorrisGame(IMorrisEventListener* morrisEventListener, IMorrisLogger* logger)
	{
		ResetGame();
		m_morrisEventListeners.emplace_back(morrisEventListener);
		if (logger)
			m_morrisLogger = logger;
	}

	void MorrisGame::ResetGame()
	{
		m_morrisEventListeners.clear();
		_eliminatedMakers.clear();
		_unplacedMarkers.clear();
		_placedMarkers.clear();
		for (int i = 0; i < 9; ++i)
			_unplacedMarkers.emplace_back(std::make_shared<MorrisMarker>(MorrisPlayer::Player1));

		for (int i = 0; i < 9; ++i)
			_unplacedMarkers.emplace_back(std::make_shared<MorrisMarker>(MorrisPlayer::Player2));

		_gameField = MorrisField();
		_gameField.SetMillEventsCallbacks(std::bind(&MorrisGame::OnMillFormed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&MorrisGame::OnMillUnformed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		_gameState = MorrisGameState::Playing;
		_currentPlayerTurn = MorrisPlayer::Player1;
	}

	void MorrisGame::SubscribeToEvents(IMorrisEventListener* morrisEventListener)
	{
		if (std::count(m_morrisEventListeners.cbegin(), m_morrisEventListeners.cend(), morrisEventListener) > 0)
			return;

		m_morrisEventListeners.emplace_back(morrisEventListener);
	}

	void MorrisGame::UnsubscribeFromEvents(IMorrisEventListener* morrisEventListener)
	{
		if (std::count(m_morrisEventListeners.cbegin(), m_morrisEventListeners.cend(), morrisEventListener) > 0)
			m_morrisEventListeners.erase(std::remove(m_morrisEventListeners.begin(), m_morrisEventListeners.end(), morrisEventListener));
	}

	MorrisGameState MorrisGame::GetGameState() const
	{
		return _gameState;
	}
	
	MorrisPlayer MorrisGame::GetCurrentPlayerTurn() const
	{
		return _currentPlayerTurn;
	}

	const MorrisMarkerPtr MorrisGame::GetMarkerAt(int pos) const
	{
		return _gameField.GetAt(pos);
	}

	const std::vector<MorrisMarkerPtr>& MorrisGame::GetUnplacedMarkers() const
	{
		return _unplacedMarkers;
	}

	bool MorrisGame::PlaceMarketAtPoint(int pos, const MorrisMarkerPtr marker)
	{
		// check gamestate
		if (_gameState != MorrisGameState::Playing)
			return false;

		// check out of bounds
		if (pos < 0 || pos > 23)
			return false;

		// check if it's that player's turn
		if (_currentPlayerTurn != marker->GetColor())
			return false;

		// check if marker belongs in unplaced markers
		auto result = std::find(_unplacedMarkers.begin(), _unplacedMarkers.end(), marker);
		if (result == _unplacedMarkers.end())
			return false;

		// check if marker can be placed at that point
		if (_gameField.GetAt(pos) != nullptr)
			return false;

		_gameField.SetAt(pos, marker);
		_unplacedMarkers.erase(result);
		_placedMarkers.push_back(marker);

		TRIGGER_EVENT(OnMarkerPlacedCallback, pos, marker);
		LogMessage("Marker placed on position " + std::to_string(pos));
		AfterMoveLogic(marker);
		return true;
	}

	bool MorrisGame::MoveMarkerToPoint(int pos, const MorrisMarkerPtr marker)
	{
		// check gamestate
		if (_gameState != MorrisGameState::Playing)
			return false;

		// check out of bounds
		if (pos < 0 || pos > 23)
			return false;

		const MorrisPlayer markerColor = marker->GetColor();

		// check if it's that player's turn
		if (_currentPlayerTurn != markerColor)
			return false;

		// check if all markers of that color are placed on the board
		const int64_t markerCount = std::count_if(_unplacedMarkers.begin(), _unplacedMarkers.end(), [markerColor](const MorrisMarkerPtr marker_) { return marker_->GetColor() == markerColor; });
		if (markerCount > 0)
			return false;

		int cpos;
		if (!_gameField.GetMarkerPosition(cpos, marker))
			return false;

		bool moveSuccess;
		if (_gameField.AreAdjacent(cpos, pos))
		{
			moveSuccess = _gameField.SlideMarkerTo(pos, marker);
		}
		else
		{
			moveSuccess = _gameField.JumpMarkerTo(pos, marker);
		}
	
		if (!moveSuccess)
			return false;

		TRIGGER_EVENT(OnMarkerMovedCallback, pos, marker);
		LogMessage("Marker moved to position " + std::to_string(pos));
		AfterMoveLogic(marker);
		return true;
	}

	bool MorrisGame::EliminateMarker(const MorrisMarkerPtr marker)
	{
		if (!CanMarkerBeEliminated(marker))
			return false;

		if (!_gameField.EliminateMarker(marker))
			return false;

		_eliminatedMakers.emplace_back(marker);
		_placedMarkers.erase(std::remove(_placedMarkers.begin(), _placedMarkers.end(), marker), _placedMarkers.end());

		TRIGGER_EVENT(OnMarkerEliminatedCallback, marker);
		LogMessage("Marker eliminated");
		AfterMoveLogic(marker);
		return true;
	}

	bool MorrisGame::CanMarkerBeEliminated(const MorrisMarkerPtr marker) const
	{
		// check gamestate
		if (_gameState != MorrisGameState::RemoveP1Marker && _gameState != MorrisGameState::RemoveP2Marker)
			return false;

		// check marker color
		const MorrisPlayer markerColor = marker->GetColor();
		if (_gameState == MorrisGameState::RemoveP1Marker && markerColor != MorrisPlayer::Player1)
			return false;

		if (_gameState == MorrisGameState::RemoveP2Marker && markerColor != MorrisPlayer::Player2)
			return false;

		bool canBeEliminated = false;
		bool isMarkerPartOfMill = _gameField.IsMarkerPartOfMill(marker);

		if (!isMarkerPartOfMill)
		{
			canBeEliminated = true;
		}
		else
		{
			const int markersThatDontFormMillsCount = _gameField.GetPlayerMarkerCountWhichDoNotFormMills(markerColor);
			if (markersThatDontFormMillsCount == 0) // exception is made when all player's markers form mills
			{
				canBeEliminated = true;
			}
			else
			{
				canBeEliminated = false;
			}
		}

		return canBeEliminated;
	}

	void MorrisGame::OnMillFormed(int pos1, int pos2, int pos3, MorrisPlayer player)
	{
		LogMessage("Mill formed: " + std::to_string(pos1) + " " + std::to_string(pos2) + " " + std::to_string(pos3));
		TRIGGER_EVENT(OnMillFormed, pos1, pos2, pos3, player);
	}

	void MorrisGame::OnMillUnformed(int pos1, int pos2, int pos3, MorrisPlayer player)
	{
		LogMessage("Mill unformed: " + std::to_string(pos1) + " " + std::to_string(pos2) + " " + std::to_string(pos3));
		TRIGGER_EVENT(OnMillUnFormed, pos1, pos2, pos3, player);
	}
	
	bool MorrisGame::CanPlayerMakeAMove(MorrisPlayer player) const
	{
		// if player has unplaced markers player can still make a move
		const int64_t unplacedMarkerCount = std::count_if(_unplacedMarkers.cbegin(), _unplacedMarkers.cend(), [player](const MorrisMarkerPtr marker_) { return marker_->GetColor() == player; });
		if (unplacedMarkerCount > 0)
			return true;
		
		// check if any player marker has any adjencent free spots, if yes, return true, if false player loses
		const int playerMarkerCount = _gameField.GetMarkerCount(player);
		if (playerMarkerCount > 3)
		{
			std::vector<MorrisMarkerPtr> playerPlacedMarkers;
			std::copy_if(_placedMarkers.cbegin(), _placedMarkers.cend(), std::back_inserter(playerPlacedMarkers), [player](MorrisMarkerPtr marker)
			{
				return marker->GetColor() == player;
			});

			bool playerCanMove = false;
			for (const MorrisMarkerPtr& marker : playerPlacedMarkers)
			{
				if (_gameField.CanMarkerBeMoved(marker))
				{
					playerCanMove = true;
					break;
				}
			}

			if (!playerCanMove)	// player is stuck
				return false;
		}		
		
		// player can definitely make a move
		return true;
	}

	void MorrisGame::ChangePlayerTurn()
	{
		_currentPlayerTurn = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisPlayer::Player2 : MorrisPlayer::Player1;
		TRIGGER_EVENT(OnPlayerTurnChangedCallback, _currentPlayerTurn);
		LogMessage("Player turn changed");
	}

	void MorrisGame::AfterMoveLogic(const MorrisMarkerPtr& marker)
	{
		const MorrisGameState prevGameState = _gameState;
		switch (_gameState)
		{
			case MorrisGameState::RemoveP1Marker:
			case MorrisGameState::RemoveP2Marker:
			{
				const int unplacedMarkersCount = _unplacedMarkers.size();
				if (_gameField.GetMarkerCount(MorrisPlayer::Player1) < 3 && unplacedMarkersCount == 0)
				{
					_gameState = MorrisGameState::P2Wins;
					TRIGGER_EVENT(OnPlayerWinCallback, MorrisPlayer::Player2);
					LogMessage("Player 2 wins");
					break;
				}
			
				if (_gameField.GetMarkerCount(MorrisPlayer::Player2) < 3 && unplacedMarkersCount == 0)
				{
					_gameState = MorrisGameState::P1Wins;
					TRIGGER_EVENT(OnPlayerWinCallback, MorrisPlayer::Player1);
					LogMessage("Player 1 wins");
					break;
				}
				
				_gameState = MorrisGameState::Playing;

				// if the next player is unable to make a move, declare victory
				MorrisPlayer oposingPlayer = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisPlayer::Player2 : MorrisPlayer::Player1;
				if (!CanPlayerMakeAMove(oposingPlayer))
				{
					_gameState = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisGameState::P1Wins : MorrisGameState::P2Wins;
					TRIGGER_EVENT(OnPlayerWinCallback, _currentPlayerTurn);
					LogMessage("Game over");
				}
				else
				{
					// if the move didn't form a mill, change the turn
					ChangePlayerTurn();
				}
				break;
			}

			case MorrisGameState::Playing:
			{
				// check for 3 in a row for current player
				// if player has 3 in a row, switch to removemarker state without changing the player turn
				// after a player removes a marker then change the turn
				if (_gameField.Has3InARow(marker))
				{
					if (_currentPlayerTurn == MorrisPlayer::Player1)
					{
						_gameState = MorrisGameState::RemoveP2Marker;
					}
					else
					{
						_gameState = MorrisGameState::RemoveP1Marker;
					}
				}
				else
				{
					// if the next player is unable to make a move, declare victory
					MorrisPlayer oposingPlayer = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisPlayer::Player2 : MorrisPlayer::Player1;
					if (!CanPlayerMakeAMove(oposingPlayer))
					{
						_gameState = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisGameState::P1Wins : MorrisGameState::P2Wins;
						TRIGGER_EVENT(OnPlayerWinCallback, _currentPlayerTurn);
						LogMessage("Game over");
					}
					else
					{
						// if the move didn't form a mill, change the turn
						ChangePlayerTurn();
					}
				}
				break;
			}
			default:
				break;
		}

		if (prevGameState != _gameState)
		{
			TRIGGER_EVENT(OnGamestateChangedCallback, prevGameState, _gameState);
			LogMessage("Game state changed");
		}
	}
	
	void MorrisGame::LogMessage(const std::string& message)
	{
		if (m_morrisLogger)
			m_morrisLogger->OnLog(message);
	}
}