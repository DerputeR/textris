#pragma once

namespace ColorConsoleInput
{
	struct Key
	{
		int keycode;
		bool pressed = false;
		bool holding = false;

		Key(int keycode)
			: keycode{ keycode }
		{
			
		}

		void UpdateKey(bool activated);

	private:
		Key();
	};
}
