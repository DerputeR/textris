#include <Windows.h>
#include <iostream>

int screen_width = 120; // col count
int screen_height = 30; // row count
const int min_screen_width = 80;
const int min_screen_height = 30;

SMALL_RECT sr_screen_size { 0, 0, screen_width - 1, screen_height - 1 };
COORD coord_buffer_size{ screen_width, screen_height };

bool locked_screen_size = false;

unsigned char* string_field = nullptr;

bool inputs[1]{};

int main()
{
	wchar_t* screen = new wchar_t[screen_width * screen_height];
	for (int i = 0; i < screen_width * screen_height; i++)
	{
		screen[i] = L' ';
	}

	// initial setup
	HANDLE h_console = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL);
	SetConsoleActiveScreenBuffer(h_console);
	DWORD dw_bytes_written = 0;

	// getting font info and updating it
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi); // MUST BE DONE
	GetCurrentConsoleFontEx(h_console, false, &cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 32;
	cfi.dwFontSize.Y = 16;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	// SetCurrentConsoleFontEx(h_console, NULL, &cfi);

	// get screen info as the program runs
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD currentWindowSize;

	while (true)
	{
		// output
		WriteConsoleOutputCharacterW(
			h_console,
			screen,
			screen_width * screen_height,
			{ 0, 0 },
			&dw_bytes_written);
	}

	system("pause");
}