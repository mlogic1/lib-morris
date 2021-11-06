#include <MorrisMarker.h>

namespace Morris
{
	MorrisMarker::MorrisMarker(MorrisPlayer color) :
		_color(color)
	{

	}

	MorrisPlayer Morris::MorrisMarker::GetColor() const
	{
		return _color;
	}
}