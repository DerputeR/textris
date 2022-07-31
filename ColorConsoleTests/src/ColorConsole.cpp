#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "KeyInput.h"
#include <string>
#include <vector>

bool running = true;

// window management
short screen_width = 102; // col count
short screen_height = 30; // row count
const short min_screen_width = 80;
const short min_screen_height = 30;

SMALL_RECT sr_screen_size { 0, 0, screen_width - 1, screen_height - 1 };
COORD coord_buffer_size{ screen_width, screen_height };

bool locked_screen_size = false;

// pre-console raw string
unsigned char* string_field = nullptr;

// color position
int color_start_index = 10;
int color_length = 10;
WORD color = FOREGROUND_RED;

bool use_console_buffer = true;
int console_buffer_mode = 0; // 0, 1, 2
int buffer_size = screen_width * screen_height;

// inputs
const int input_count = 7;
ColorConsoleInput::Key input_keys[input_count] = {
	{VK_ESCAPE},
	{VK_RETURN},
	{VK_SPACE},
	{VK_UP},
	{VK_DOWN},
	{VK_RIGHT},
	{VK_LEFT}
};

// unsafe but i don't care
void ReplaceChars(char* string, char* replace, int start, int len)
{
	for (int i = 0; i < len; i++)
	{
		string[i + start] = replace[i];
	}
}

void ReplaceChars(wchar_t* string, wchar_t* replace, int start, int len)
{
	for (int i = 0; i < len; i++)
	{
		string[i + start] = replace[i];
	}
}

int main()
{
	wchar_t* screen = new wchar_t[screen_width * screen_height];
	CHAR_INFO* screen_data = new CHAR_INFO[screen_width * screen_height];
	for (int i = 0; i < screen_width * screen_height; i++)
	{
		screen[i] = L' ';
		screen_data[i].Char.UnicodeChar = L' ';
	}	

	// console buffer setup
	HANDLE h_console_buffer = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL);
	DWORD dw_bytes_written = 0;
	DWORD dw_attrs_written = 0;
	if (use_console_buffer)
	{
		SetConsoleMode(
			h_console_buffer,
			ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN
		);
		SetConsoleActiveScreenBuffer(h_console_buffer);
	}
	else
	{
		std::cout << "Started outside of buffer mode" << std::endl;
	}

	// getting font info and updating it
	/*
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi); // MUST BE DONE
	GetCurrentConsoleFontEx(h_console_buffer, false, &cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 16;
	cfi.dwFontSize.Y = 16;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	// SetCurrentConsoleFontEx(h_console_buffer, NULL, &cfi);
	*/

	// get screen info as the program runs
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD currentWindowSize;

	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(h_console_buffer, &cci);
	cci.bVisible = false;
	SetConsoleCursorInfo(h_console_buffer, &cci);

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
				h_console_buffer = CreateConsoleScreenBuffer(
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					CONSOLE_TEXTMODE_BUFFER,
					NULL);
				SetConsoleActiveScreenBuffer(h_console_buffer);
				SetConsoleCursorInfo(h_console_buffer, &cci);
			}
			else
			{
				CloseHandle(h_console_buffer);
				system("cls");
				std::cout << "Exited buffer mode" << std::endl;
				wprintf(L"\x1B[31m" L"Buffer size: %-4d" L"\x1B[0m\n", buffer_size);
			}
			use_console_buffer = !use_console_buffer;
		}

		if (input_keys[5].pressed)
		{
			color_start_index += 5;
		}
		if (input_keys[6].pressed)
		{
			color_start_index -= 5;
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
			// swprintf_s(&screen[0], 28, L"\x1B[31m" L"Buffer size: %-4d" L"\x1B[0m", buffer_size);

			/*
			
			std::wstring string_label = L"Buffer size: %-4d";
			wchar_t wchar_label[] = L"Buffer size: %-4d";
			swprintf(
				&wchar_label[0],
				28,
				wchar_label,
				buffer_size);
			std::wstring wstr_screen = (std::wstring) screen;
			wstr_screen.replace(
				0,
				28,
				wchar_label
			);
			std::vector<wchar_t> vector_label_copy{wstr_screen.begin(), wstr_screen.end()};
			screen = &vector_label_copy[0];
			// screen = &*(wstr_screen.data()); // doesn't work

			*/

			wchar_t wchar_label[] = L"Buffer size: %-4d";
			swprintf(
				&wchar_label[0],
				28,
				wchar_label,
				buffer_size);
			ReplaceChars(screen, wchar_label, 0, 28);

			// output
			//WriteConsoleOutputAttribute(
			//	h_console_buffer,
			//	&color,
			//	color_length,
			//	{ (short)(color_start_index % screen_width), (short)(color_start_index / screen_width) },
			//	&dw_attrs_written
			//);

			//WriteConsoleOutputCharacterW(
			//	h_console_buffer,
			//	screen,
			//	// buffer_size,
			//	screen_width * screen_height,
			//	{ 0, 0 },
			//	&dw_bytes_written);

			
			//WriteConsoleW(
			//	h_console_buffer,
			//	screen,
			//	// buffer_size,
			//	screen_width * screen_height,
			//	&dw_bytes_written,
			//	NULL);

			WriteConsoleOutput(
				h_console_buffer,
				&screen[0],
				{ screen_width, screen_height },
				{ 0, 0 },
				{ 0, 0, screen_width, screen_height }
			);

			SetConsoleCursorPosition(
				h_console_buffer,
				{ 0, 0 }
			);

		}
		else
		{
			
		}
	}

	if (use_console_buffer)
		CloseHandle(h_console_buffer);
	system("pause");
}