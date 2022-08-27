#pragma once
#include <Windows.h>
#include <iostream>

class ConsoleRenderer
{
private:
	CONSOLE_SCREEN_BUFFER_INFO default_config;
	DWORD default_output_mode = 0;
	DWORD default_input_mode = 0;
	DWORD console_output_mode = 0;
	DWORD console_input_mode = 0;

	short width = 130;
	short height = 20;
	HANDLE h_in;
	HANDLE h_out;
	SMALL_RECT window_size;
	bool use_full_colors = false;
	bool use_separate_buffer = false;
public:
	ConsoleRenderer
	(
		short width = 130,
		short height = 20,
		const wchar_t* title = L"Console App",
		bool enable_mouse_input = false,
		bool use_full_colors = false,
		bool use_separate_buffer = false
	);

	~ConsoleRenderer();

	void UpdateWindowAndBufferSize(short new_width, short new_height);
};

