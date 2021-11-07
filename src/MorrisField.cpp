#include <MorrisField.h>
#include <algorithm>
#include <cmath>

namespace Morris
{
	MorrisField::MorrisField()
	{

	}

	const std::array<MorrisMarkerPtr, 24>& MorrisField::GetField() const
	{
		return _cells;
	}

	bool MorrisField::GetMarkerPosition(int& pos, const MorrisMarkerPtr marker) const
	{
		for (int i = 0; i < _cells.size(); ++i)
		{
			if (_cells[i] == marker)
			{
				pos = i;
				return true;
			}
		}
		return false;
	}

	const MorrisMarkerPtr MorrisField::GetAt(int pos) const
	{
		return _cells[pos];
	}
	
	bool MorrisField::SetAt(int pos, const MorrisMarkerPtr marker)
	{
		if (_cells[pos] != nullptr)
			return false;

		_cells[pos] = marker;
		AfterMoveCheckMills(_cells[pos]);
		return true;
	}

	bool MorrisField::SlideMarkerTo(int pos, const MorrisMarkerPtr marker)
	{
		int cpos;	// current marker spot
		if (!GetMarkerPosition(cpos, marker))
			return false;

		// check if target spot is free
		if (_cells[pos] != nullptr)
			return false;

		if (!AreAdjacent(cpos, pos))
		return false;

		// move and clear the previous spot
		_cells[pos] = std::move(_cells[cpos]);
		AfterMoveCheckMills(_cells[pos]);
		return true;
	}

	bool MorrisField::JumpMarkerTo(int pos, const MorrisMarkerPtr marker)
	{
		// jumps can only be made if that player has exactly 3 markers
		if (GetMarkerCount(marker->GetColor()) != 3)
			return false;

		// check if target spot is free
		if (_cells[pos] != nullptr)
			return false;

		int cpos;
		if (!GetMarkerPosition(cpos, marker))
			return false;

		if (_cells[pos] != nullptr)
			return false;

		// move and clear the previous spot
		_cells[pos] = std::move(_cells[cpos]);
		AfterMoveCheckMills(_cells[pos]);
		return true;
	}

	bool MorrisField::EliminateMarker(const MorrisMarkerPtr marker)
	{
		int pos;
		if (!GetMarkerPosition(pos, marker))
			return false;

		_cells[pos] = nullptr;
		return true;
	}

	int MorrisField::GetMarkerCount(MorrisPlayer player) const
	{
		int count = 0;

		for (const MorrisMarkerPtr marker : _cells)
		{
			if (marker && marker->GetColor() == player)
				++count;
		}
		return count;
	}

	bool MorrisField::Has3InARow(const MorrisMarkerPtr marker) const
	{
		const MorrisPlayer markerColor = marker->GetColor();
		int markerPos = -1;
		if (!GetMarkerPosition(markerPos, marker))
			return false;

		std::vector<std::array<int, 3>> validLines;

		std::for_each(_lines.cbegin(), _lines.cend(), [&validLines, markerPos](std::array<int, 3> line)
			{
				bool lineValid = std::find(line.cbegin(), line.cend(), markerPos) != line.cend();
				if (lineValid)
					validLines.push_back(line);
			});
		for (std::array<int, 3> line : validLines)
		{
			const int pos1 = line[0], pos2 = line[1], pos3 = line[2];
			if (_cells[pos1] && _cells[pos1]->GetColor() == markerColor)
				if (_cells[pos2] && _cells[pos2]->GetColor() == markerColor)
					if (_cells[pos3] && _cells[pos3]->GetColor() == markerColor)
						return true;
		}

		return false;
	}

	bool MorrisField::IsMarkerPartOfMill(const MorrisMarkerPtr marker) const
	{
		for (const std::array<MorrisMarkerPtr, 3>& mill : _mills)
		{
			if (std::count(mill.cbegin(), mill.cend(), marker) > 0)
				return true;
		}
		return false;
	}

	const std::vector<std::array<MorrisMarkerPtr, 3>>& MorrisField::GetMills() const
	{
		return _mills;
	}

	bool MorrisField::CanMarkerBeMoved(const MorrisMarkerPtr marker) const
	{
		int cpos;
		if (!GetMarkerPosition(cpos, marker))
			return false;

		for (int adjacent : _adjacents[cpos])
		{
			if (_cells[adjacent] == nullptr)
				return true;
		}

		return false;
	}

	int MorrisField::GetPlayerMarkerCountWhichFormMills(MorrisPlayer player) const
	{
		int markerThatFormMillsCount = 0;
		for (const MorrisMarkerPtr& marker : _cells)
		{
			if (marker && marker->GetColor() == player)
			{
				if (IsMarkerPartOfMill(marker))
				{
					++markerThatFormMillsCount;
				}
			}
		}

		return markerThatFormMillsCount;
	}

	int MorrisField::GetPlayerMarkerCountWhichDoNotFormMills(MorrisPlayer player) const
	{
		int markerThatDoNotFormMillsCount = 0;
		for (const MorrisMarkerPtr& marker : _cells)
		{
			if (marker && marker->GetColor() == player)
			{
				if (!IsMarkerPartOfMill(marker))
				{
					++markerThatDoNotFormMillsCount;
				}
			}
		}

		return markerThatDoNotFormMillsCount;
	}

	bool MorrisField::AreAdjacent(int pos1, int pos2) const
	{
		return _adjacents[pos1].find(pos2) != _adjacents[pos1].end();
	}

	void MorrisField::AfterMoveCheckMills(const MorrisMarkerPtr marker)
	{
		const MorrisPlayer markerColor = marker->GetColor();

		// check if the marker was a part of a mill previously
		for (const std::array<MorrisMarkerPtr, 3>& mill : _mills)
		{
			if (IsMarkerPartOfMill(marker))
				UnformMill(mill);
		}

		// check if the moved marker forms a mill
		int cpos;
		if (!GetMarkerPosition(cpos, marker))
			return;

		std::vector<std::array<int, 3>> validLines;
		std::for_each(_lines.cbegin(), _lines.cend(), [&validLines, cpos](std::array<int, 3> line)
		{
			bool lineValid = std::find(line.cbegin(), line.cend(), cpos) != line.cend();
			if (lineValid)
				validLines.push_back(line);
		});

		for (std::array<int, 3> line : validLines)
		{
			const int pos1 = line[0], pos2 = line[1], pos3 = line[2];
			if (_cells[pos1] && _cells[pos1]->GetColor() == markerColor)
				if (_cells[pos2] && _cells[pos2]->GetColor() == markerColor)
					if (_cells[pos3] && _cells[pos3]->GetColor() == markerColor)
						FormMill({_cells[pos1], _cells[pos2], _cells[pos3]});
		}
	}
	
	void MorrisField::FormMill(std::array<MorrisMarkerPtr, 3> line)
	{
		_mills.emplace_back(line);
	}

	void MorrisField::UnformMill(std::array<MorrisMarkerPtr, 3> mill)
	{
		_mills.erase(std::remove(_mills.begin(), _mills.end(), mill), _mills.end());
	}
}