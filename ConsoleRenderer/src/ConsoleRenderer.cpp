#include "ConsoleRenderer.h"

ConsoleRenderer::ConsoleRenderer
(
	short width,
	short height,
	const wchar_t* title,
	bool enable_mouse_input,
	bool use_full_colors,
	bool use_separate_buffer
) : width(width),
	height(height),
	use_full_colors(use_full_colors),
	use_separate_buffer(use_separate_buffer)
{
	// Get handles
	h_in = GetStdHandle(STD_INPUT_HANDLE);
	if (use_separate_buffer)
	{
		h_out = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(h_out);
	}
	else
	{
		h_out = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	// Set flags
	GetConsoleMode(h_out, &default_output_mode);
	GetConsoleMode(h_in, &default_input_mode);
	console_output_mode = default_output_mode;
	console_input_mode = default_input_mode;

	console_output_mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	// by default, all input flags except virtual terminal input and window input are enabled
	// todo : allow constructor to choose which flags to use, and provide list of default constant options
	console_input_mode = ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT;
	SetConsoleMode(h_out, console_output_mode);
	SetConsoleMode(h_in, console_input_mode);

	// Set title
	SetConsoleTitle(title);

	// Save previous window configuration, in case we are running this within another terminal
	GetConsoleScreenBufferInfo(h_out, &default_config);

	// Set window and buffer size
	window_size.Top = 0;
	window_size.Left = 0;
	window_size.Right = width - 1;
	window_size.Bottom = height - 1;

	// ! We surround with screen buffer adjust since:
	// If we don't set the buffer size after shrinking the window from default, then we get scroll bars,
	//	and we cannot make the buffer size smaller than the window size (which would happen if we set buffer size first only)
	// But if we don't set the buffer size before expanding the window from default, then the window won't grow
	//	and we get more scroll bars if we set the buffer size last only
	// ! We can make the buffer size >= window size, but we cannot make the buffer size < window size

	SetConsoleScreenBufferSize(h_out, { width, height });
	SetConsoleWindowInfo(h_out, 1, &window_size);
	SetConsoleScreenBufferSize(h_out, { width, height });
}

ConsoleRenderer::~ConsoleRenderer()
{
	// Restore previous terminal settings
	UpdateWindowAndBufferSize(default_config.dwSize.X, default_config.dwSize.Y);
	SetConsoleMode(h_out, default_output_mode);
	SetConsoleMode(h_in, default_input_mode);
	
	// Close handle, if being used
	if (use_separate_buffer)
	{
		CloseHandle(h_out);
	}
}

void ConsoleRenderer::UpdateWindowAndBufferSize(short new_width, short new_height)
{
	width = new_width;
	height = new_height;
	window_size.Right = width - 1;
	window_size.Bottom = height - 1;

	SetConsoleScreenBufferSize(h_out, { width, height });
	SetConsoleWindowInfo(h_out, 1, &window_size);
	SetConsoleScreenBufferSize(h_out, { width, height });
}