#pragma once

#include "MorrisMarker.h"
#include <array>
#include <functional>
#include <set>
#include <vector>

namespace Morris
{
	class MorrisField
	{
	private:
		MorrisField();
		MorrisField& operator=(const MorrisField& other)
		{
			// _cells = other._cells;
			return *this;
		}

		void SetMillEventsCallbacks(std::function<void(int, int, int, MorrisPlayer)> onMillFormedCallback, std::function<void(int, int, int, MorrisPlayer)> onMillUnormedCallback);

		const std::array<MorrisMarkerPtr, 24>& GetField() const;
		bool GetMarkerPosition(int& pos, const MorrisMarkerPtr marker) const;
		
		const MorrisMarkerPtr GetAt(int pos) const;
		bool SetAt(int pos, const MorrisMarkerPtr marker);
		bool SlideMarkerTo(int pos, const MorrisMarkerPtr marker);
		bool JumpMarkerTo(int pos, const MorrisMarkerPtr marker);
		bool EliminateMarker(const MorrisMarkerPtr marker);
		int GetMarkerCount(MorrisPlayer player) const;
		bool Has3InARow(const MorrisMarkerPtr marker) const;
		bool IsMarkerPartOfMill(const MorrisMarkerPtr marker) const;
		const std::vector<std::array<MorrisMarkerPtr, 3>>& GetMills() const;
		bool CanMarkerBeMoved(const MorrisMarkerPtr marker) const;
		
		int GetPlayerMarkerCountWhichFormMills(MorrisPlayer player) const;
		int GetPlayerMarkerCountWhichDoNotFormMills(MorrisPlayer player) const;

	private:
		bool AreAdjacent(int pos1, int pos2) const;
		void AfterMoveCheckMills(const MorrisMarkerPtr marker);
		void FormMill(std::array<MorrisMarkerPtr, 3> line);
		void UnformMill(std::array<MorrisMarkerPtr, 3> mill);

	private:
		std::array<MorrisMarkerPtr, 24> _cells;

		std::function<void(int, int, int, MorrisPlayer)> m_onMillFormedCallback;
		std::function<void(int, int, int, MorrisPlayer)> m_onMillUnormedCallback;

		const std::vector<std::set<int>> _adjacents =
		{
			{1, 9},				// 0
			{0, 2, 4},			// 1
			{1, 14},			// 2
			{4, 10},			// 3
			{1, 3, 5, 7},		// 4
			{4, 13},			// 5
			{7, 11},			// 6
			{4, 6, 8},			// 7
			{7, 12},			// 8
			{0, 10, 21},		// 9
			{3, 9, 11, 18},		// 10
			{6, 10, 15},		// 11
			{8, 13, 17},		// 12
			{5, 12, 14, 20},	// 13
			{2, 13, 23},		// 14
			{11, 16},			// 15
			{15, 17, 19},		// 16
			{12, 16},			// 17
			{10, 19},			// 18
			{16, 18, 20, 22},	// 19
			{13, 19},			// 20
			{9, 22},			// 21
			{19, 21, 23},		// 22
			{14, 22}			// 23
		};

		const std::vector<std::array<int, 3>> _lines =
		{
			{0, 1, 2},
			{3, 4, 5},
			{6, 7, 8},
			{9, 10, 11},
			{12, 13, 14},
			{15, 16, 17},
			{18, 19, 20},
			{21, 22, 23},
			{0, 9, 21},
			{3, 10, 18},
			{6, 11, 15},
			{1, 4, 7},
			{16, 19, 22},
			{8, 12, 17},
			{5, 13, 20},
			{2, 14, 23}
		};

		std::vector<std::array<MorrisMarkerPtr, 3>> _mills;

		friend class MorrisGame;
	};
}

