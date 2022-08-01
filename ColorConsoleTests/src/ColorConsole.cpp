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
int color_start_index = 0;
int color_length = 10;
WORD color_red = FOREGROUND_RED;
WORD color_default = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

bool use_console_buffer = true;
int console_buffer_mode = 0; // 0, 1, 2
int buffer_size = screen_width * screen_height;

// inputs
const int input_count = 9;
ColorConsoleInput::Key input_keys[input_count] = {
	{VK_ESCAPE},
	{VK_RETURN},
	{VK_SPACE},
	{VK_UP},
	{VK_DOWN},
	{VK_RIGHT},
	{VK_LEFT},
	{VK_LCONTROL},
	{VK_LSHIFT}
};
HANDLE console_window;
HANDLE foreground_window;

// timing
std::chrono::steady_clock::time_point t1, t2;
std::chrono::duration<double> deltaTime;

// glyphs
const wchar_t* glyph_blocks = L"█▓▒░";

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

void ReplaceCharInfo(CHAR_INFO* ci, wchar_t* replace, int start, int len)
{
	for (int i = 0; i < len; i++)
	{
		//if (replace[i] == '\0')
		//{
		//	break;
		//}
		ci[i + start].Char.UnicodeChar = replace[i];
	}
}

void GenerateColorSwatches(CHAR_INFO* buffer)
{
	// fixed x, y
	for (int y = 0; y < 30; y++)
	{
		for (int x = 0; x < 8 * 4; x++)
		{
			buffer[y * screen_width + (2 * x)].Char.UnicodeChar = glyph_blocks[x % 4];
			buffer[y * screen_width + (2 * x + 1)].Char.UnicodeChar = glyph_blocks[x % 4];
		}
	}
}

int main()
{
	CHAR_INFO* screen_data = new CHAR_INFO[screen_width * screen_height];
	for (int i = 0; i < screen_width * screen_height; i++)
	{
		screen_data[i].Char.UnicodeChar = L' ';
		screen_data[i].Attributes = color_default;
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

	// attempt at making the cursor selection disappear
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD prev_mode;
	GetConsoleMode(hInput, &prev_mode);
	if (use_console_buffer)
	{
		SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
		SetConsoleMode(
			h_console_buffer,
			ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN
		);
		SetConsoleActiveScreenBuffer(h_console_buffer);
	}
	else
	{
		SetConsoleMode(hInput, prev_mode);
		std::cout << "Started outside of buffer mode" << std::endl;
	}

	// getting font info and updating it
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi); // MUST BE DONE
	GetCurrentConsoleFontEx(h_console_buffer, false, &cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 16;
	cfi.dwFontSize.Y = 16;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	// SetCurrentConsoleFontEx(h_console_buffer, NULL, &cfi);

	// get screen info as the program runs
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD currentWindowSize;

	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(h_console_buffer, &cci);
	cci.bVisible = false;
	SetConsoleCursorInfo(h_console_buffer, &cci);

	console_window = GetConsoleWindow();

	while (running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		/// INPUT (no events, do this old-school style)
		foreground_window = GetForegroundWindow();
		// only process inputs when the console is focused
		if (foreground_window == console_window)
		{
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
					SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
				}
				else
				{
					CloseHandle(h_console_buffer);
					system("cls");
					std::cout << "Exited buffer mode" << std::endl;
					wprintf(L"\x1B[31m" L"Buffer size: %-4d" L"\x1B[0m\n", buffer_size);
					SetConsoleMode(hInput, prev_mode);
				}
				use_console_buffer = !use_console_buffer;
			}
			if (use_console_buffer && input_keys[3].pressed)
			{
				buffer_size += input_keys[7].pressed ? 1000 : 10;
			}
			if (use_console_buffer && input_keys[4].pressed)
			{
				buffer_size = max(0, buffer_size - (input_keys[7].pressed ? 1000 : 10));
			}
			if (input_keys[5].pressed)
			{
				if (input_keys[8].pressed)
				{
					color_length += input_keys[7].pressed ? 20 : 4;
				}
				else
				{
					color_start_index += input_keys[7].pressed ? 20 : 4;
				}
			}
			if (input_keys[6].pressed)
			{
				if (input_keys[8].pressed)
				{
					color_length = max(0, color_length - (input_keys[7].pressed ? 20 : 4));
				}
				else
				{
					color_start_index = max(0, color_start_index - (input_keys[7].pressed ? 20 : 4));
				}
			}
		}

		/// OUTPUT
		if (use_console_buffer)
		{
			/*
			for (int i = 0; i < screen_width * screen_height; i++)
			{
				if (i < buffer_size)
				{
					screen_data[i].Char.UnicodeChar = L"█▓▒░"[i % 4];
					if (i >= color_start_index && i < color_start_index + color_length)
					{
						screen_data[i].Attributes = color_red;
					}
					else
					{
						screen_data[i].Attributes = color_default;
					}
				}
				else
				{
					screen_data[i].Char.UnicodeChar = L' ';
				}
			}
			*/

			t1 = std::chrono::steady_clock::now();

			GenerateColorSwatches(screen_data);

			wchar_t wchar_label[] = L"Buffer size: %-4d";
			// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
			// Below is a very raw, unsafe version of the above post
			int format_size = swprintf(
				nullptr,
				0,
				wchar_label,
				buffer_size
			) + 1; // extra space for the would-be '\0'
			swprintf(
				&wchar_label[0],
				format_size,
				wchar_label,
				buffer_size
			); // null char is still written, but now we know exactly where it is

			ReplaceCharInfo(screen_data, wchar_label, screen_width - (format_size - 1), format_size - 1);

			WriteConsoleOutput(
				h_console_buffer,
				screen_data,
				{ screen_width, screen_height },
				{ 0, 0 },
				&sr_screen_size
			);

			t2 = std::chrono::steady_clock::now();
			deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			SetConsoleTitleA(("DT: " + std::to_string(deltaTime.count()) + "; FPS: " + std::to_string(1 / deltaTime.count())).data());
		}
	}

	if (use_console_buffer)
		CloseHandle(h_console_buffer);
	system("pause");
}