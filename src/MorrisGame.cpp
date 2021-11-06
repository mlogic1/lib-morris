#include <MorrisGame.h>
#include <algorithm>
#include <cmath>
#include <iterator>

namespace Morris
{
	MorrisGame::MorrisGame()
	{
		ResetGame();
	}

	MorrisGame::MorrisGame(
		std::function<void(MorrisPlayer)> playerTurnChangedCallback,
		std::function<void(MorrisGameState, MorrisGameState)> gameStateChangedCallback,
		std::function<void(MorrisPlayer)> playerWonCallback,
		std::function<void(const MorrisMarkerPtr)> markerEliminatedCallback,
		std::function<void(int, const MorrisMarkerPtr)> markerPlacedCallback,
		std::function<void(int, const MorrisMarkerPtr)> markerMovedCallback) :
			_playerTurnChangedCallback(playerTurnChangedCallback),
			_gameStateChangedCallback(gameStateChangedCallback),
			_playerWonCallback(playerWonCallback),
			_markerEliminatedCallback(markerEliminatedCallback),
			_markerPlacedCallback(markerPlacedCallback),
			_markerMovedCallback(markerMovedCallback)
	{
		ResetGame();
	}

	void MorrisGame::ResetGame()
	{
		_eliminatedMakers.clear();
		_unplacedMarkers.clear();
		_placedMarkers.clear();
		for (int i = 0; i < 9; ++i)
			_unplacedMarkers.emplace_back(std::make_shared<MorrisMarker>(MorrisPlayer::Player1));

		for (int i = 0; i < 9; ++i)
			_unplacedMarkers.emplace_back(std::make_shared<MorrisMarker>(MorrisPlayer::Player2));

		_gameField = MorrisField();
		_gameState = MorrisGameState::Playing;
		_currentPlayerTurn = MorrisPlayer::Player1;
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

		TriggerCallback(_markerPlacedCallback, pos, marker);
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

		TriggerCallback(_markerMovedCallback, pos, marker);
		AfterMoveLogic(marker);
		return true;
	}

	bool MorrisGame::EliminateMarker(const MorrisMarkerPtr marker)
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

		if (!_gameField.EliminateMarker(marker))
			return false;

		_eliminatedMakers.emplace_back(marker);
		_placedMarkers.erase(std::remove(_placedMarkers.begin(), _placedMarkers.end(), marker), _placedMarkers.end());

		TriggerCallback(_markerEliminatedCallback, marker);
		AfterMoveLogic(marker);
		return true;
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
		TriggerCallback(_playerTurnChangedCallback, _currentPlayerTurn);
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
					TriggerCallback(_playerWonCallback, MorrisPlayer::Player2);
					break;
				}
			
				if (_gameField.GetMarkerCount(MorrisPlayer::Player2) < 3 && unplacedMarkersCount == 0)
				{
					_gameState = MorrisGameState::P1Wins;
					TriggerCallback(_playerWonCallback, MorrisPlayer::Player1);
					break;
				}
				
				_gameState = MorrisGameState::Playing;

				// if the next player is unable to make a move, declare victory
				MorrisPlayer oposingPlayer = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisPlayer::Player2 : MorrisPlayer::Player1;
				if (!CanPlayerMakeAMove(oposingPlayer))
				{
					// MorrisPlayer winningPlayer = (_currentPlayerTurn == MorrisPlayer::Player1) ? 
					_gameState = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisGameState::P1Wins : MorrisGameState::P2Wins;
					TriggerCallback(_playerWonCallback, _currentPlayerTurn);
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
					// TODO: if all markers form mills, it's allowed to remove any marker
				}
				else
				{
					// if the next player is unable to make a move, declare victory
					MorrisPlayer oposingPlayer = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisPlayer::Player2 : MorrisPlayer::Player1;
					if (!CanPlayerMakeAMove(oposingPlayer))
					{
						// MorrisPlayer winningPlayer = (_currentPlayerTurn == MorrisPlayer::Player1) ? 
						_gameState = (_currentPlayerTurn == MorrisPlayer::Player1) ? MorrisGameState::P1Wins : MorrisGameState::P2Wins;
						TriggerCallback(_playerWonCallback, _currentPlayerTurn);
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
			TriggerCallback(_gameStateChangedCallback, prevGameState, _gameState);
	}
}