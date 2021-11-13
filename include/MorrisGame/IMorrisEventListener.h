#pragma once

#include "MorrisGameState.h"
#include "MorrisMarker.h"
#include "MorrisPlayer.h"

namespace Morris
{
	class IMorrisEventListener
	{
	public:
		virtual ~IMorrisEventListener() {};

		virtual void OnPlayerTurnChangedCallback(MorrisPlayer player) = 0;
		virtual void OnGamestateChangedCallback(MorrisGameState previousGamestate, MorrisGameState currentGameState) = 0;
		virtual void OnPlayerWinCallback(MorrisPlayer winner) = 0;
		virtual void OnMarkerEliminatedCallback(const MorrisMarkerPtr marker) = 0;
		virtual void OnMarkerPlacedCallback(int pos, const MorrisMarkerPtr marker) = 0;
		virtual void OnMarkerMovedCallback(int pos, const MorrisMarkerPtr marker) = 0;
		virtual void OnMillFormed(int pos1, int pos2, int pos3, MorrisPlayer player) = 0;
		virtual void OnMillUnFormed(int pos1, int pos2, int pos3, MorrisPlayer player) = 0;
	};
}