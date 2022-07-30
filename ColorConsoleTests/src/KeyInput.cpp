#include "KeyInput.h"

void ColorConsoleInput::Key::UpdateKey(bool activated)
{
	if (activated)
	{
		if (pressed)
		{
			holding = true;
		}
		pressed = true;
	}
	else
	{
		pressed = false;
		holding = false;
	}
}