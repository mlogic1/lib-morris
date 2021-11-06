#pragma once

#include "MorrisPlayer.h"
#include <memory>

namespace Morris
{
	class MorrisMarker
	{
	public:
		MorrisMarker(MorrisPlayer color);
		MorrisPlayer GetColor() const;

	private:
		const MorrisPlayer _color;
	};

	using MorrisMarkerPtr = std::shared_ptr<MorrisMarker>;
}