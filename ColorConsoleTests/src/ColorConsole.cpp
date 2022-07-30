#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "KeyInput.h"

bool running = true;

// window management
int screen_width = 102; // col count
int screen_height = 30; // row count
const int min_screen_width = 80;
const int min_screen_height = 30;

SMALL_RECT sr_screen_size { 0, 0, screen_width - 1, screen_height - 1 };
COORD coord_buffer_size{ screen_width, screen_height };

bool locked_screen_size = false;

// pre-console raw string
unsigned char* string_field = nullptr;

bool use_console_buffer = true;
int console_buffer_mode = 0; // 0, 1, 2
int buffer_size = screen_width * screen_height;

// inputs
const int input_count = 5;
ColorConsoleInput::Key input_keys[input_count] = {
	{VK_ESCAPE},
	{VK_RETURN},
	{VK_SPACE},
	{VK_UP},
	{VK_DOWN}
};

int main()
{
	wchar_t* screen = new wchar_t[screen_width * screen_height];
	for (int i = 0; i < screen_width * screen_height; i++)
	{
		screen[i] = L' ';
	}	

	// console buffer setup
	HANDLE h_console = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL);
	DWORD dw_bytes_written = 0;
	if (use_console_buffer)
	{
		SetConsoleActiveScreenBuffer(h_console);
	}
	else
	{
		std::cout << "Started outside of buffer mode" << std::endl;
	}

	// getting font info and updating it
	/*
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi); // MUST BE DONE
	GetCurrentConsoleFontEx(h_console, false, &cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 16;
	cfi.dwFontSize.Y = 16;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	// SetCurrentConsoleFontEx(h_console, NULL, &cfi);
	*/

	// get screen info as the program runs
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD currentWindowSize;

	while (running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		/// INPUT (no events, do this old-school style)
		for (int k = 0; k < input_count; k++)
		{
			input_keys[k].UpdateKey(0x8000 & GetAsyncKeyState(input_keys[k].keycode));
		}

		if (input_keys[0].pressed)
		{
			running = false;
		}

		if (input_keys[1].pressed && !input_keys[1].holding)
		{
			if (!use_console_buffer)
			{
				h_console = CreateConsoleScreenBuffer(
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					CONSOLE_TEXTMODE_BUFFER,
					NULL);
				SetConsoleActiveScreenBuffer(h_console);
			}
			else
			{
				CloseHandle(h_console);
				system("cls");
				std::cout << "Exited buffer mode" << std::endl;
			}
			use_console_buffer = !use_console_buffer;
		}

		/// OUTPUT
		if (use_console_buffer)
		{
			if (input_keys[3].pressed)
			{
				buffer_size += 10;
			}
			if (input_keys[4].pressed)
			{
				buffer_size -= 10;
			}

			for (int i = 0; i < screen_width * screen_height; i++)
			{
				if (i < buffer_size)
				{
					screen[i] = L"█▓▒░"[i % 4];
				}
				else
				{
					screen[i] = L' ';
				}
			}
			//todo: figure out how to dynamically adjust buffer size
			swprintf_s(&screen[0], 18, L"Buffer size: %-4d", buffer_size);

			// output
			WriteConsoleOutputCharacterW(
				h_console,
				screen,
				// buffer_size,
				screen_width * screen_height,
				{ 0, 0 },
				&dw_bytes_written);
		}
		else
		{
			
		}
	}

	CloseHandle(h_console);
	system("pause");
}