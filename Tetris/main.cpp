#include <iostream>
#include <string>
#include <Windows.h>

/// <summary>
/// https://www.youtube.com/watch?v=8OK8_tHeCIA&list=PLrOv9FMX8xJE8NgepZR1etrsU63fDDGxO
/// Current time: 8:17
/// </summary>

std::wstring tetrominos[7];
// TODO make this customizable at runtime
int nFieldWidth = 12;
int nFieldHeight = 18;
const int MIN_SCREEN_WIDTH = 80;
const int MIN_SCREEN_HEIGHT = 30;
int nScreenWidth = 120; // Default cmd.exe width (# of columns)
int nScreenHeight = 30; // Default cmd.exe height (# of rows)
SMALL_RECT srScreenSize{0, 0, nScreenWidth - 1, nScreenHeight - 1};
COORD srBufferSize{nScreenWidth, nScreenHeight}; 
// we will store the elements of the playing field in an array of unsigned characters, allocated dynamically
unsigned char* pField = nullptr;

bool bLockedScreenSize = false;

int ConvertToRotatedIndex(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;			// 0 deg
	case 1: return 12 + py - (px * 4);	// 90 deg
	case 2: return 15 - (py * 4) - px;	// 180 deg
	case 3: return 3 - py + (px * 4);	// 270 deg
	}
	return 0; // question: when would this ever occur?
}

int main()
{
	// = is for when we draw the line, # is for borders
	const wchar_t CHAR_SET[11] = L" ABCDEFG=#";

	/// Create assets
	tetrominos[0].append(L"..X.");
	tetrominos[0].append(L"..X.");
	tetrominos[0].append(L"..X.");
	tetrominos[0].append(L"..X.");

	tetrominos[1].append(L"..X.");
	tetrominos[1].append(L".XX.");
	tetrominos[1].append(L".X..");
	tetrominos[1].append(L"....");

	tetrominos[2].append(L".X..");
	tetrominos[2].append(L".XX.");
	tetrominos[2].append(L"..X.");
	tetrominos[2].append(L"....");

	tetrominos[3].append(L"....");
	tetrominos[3].append(L".XX.");
	tetrominos[3].append(L".XX.");
	tetrominos[3].append(L"....");

	tetrominos[4].append(L"..X.");
	tetrominos[4].append(L".XX.");
	tetrominos[4].append(L"..X.");
	tetrominos[4].append(L"....");
	
	tetrominos[5].append(L"....");
	tetrominos[5].append(L".XX.");
	tetrominos[5].append(L"..X.");
	tetrominos[5].append(L"..X.");

	tetrominos[6].append(L"....");
	tetrominos[6].append(L".XX.");
	tetrominos[6].append(L".X..");
	tetrominos[6].append(L".X..");

	/// Construct the playing field array
	// TODO Move this into separate function
	// ! remember to delete this later when we're done with it
	pField = new unsigned char[nFieldWidth * nFieldHeight]; // create array
	for (int x = 0; x < nFieldWidth; x++) // Board boundary
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			// if this part of the array corresponds to the left/right edge of the field OR the bottom, fill it
			// ? why are we using 9 and 0 and not specific chars?
			// SEE HOW THIS CORRESPONDS TO THE CHAR INDICES IN CHAR_SET
			// We're doing rendering separate from the core logic
			// ! recall we are essentially flattening a 2D gridspace into a 1D array representation
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	// ! this may require a pause since this block comes in at 9:37
	// this comes from a previous FPS video, https://www.youtube.com/watch?v=xW8skO7MFYw
	// note that i've replaced all instances of wide strings and chars with their regular 1-byte counterparts
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		screen[i] = L' ';
	}

	// ! jank work around to deal with resizing windows
	wchar_t* clearScreen = nullptr;

	/// Instead of cout doing stuff, we use a seperate cmd to draw to the console buffer
	// Apparently a HANDLE is like a pointer with no specific type until you cast it
	// A DWORD is from Windows.h and behaves as an unsigned int with a specific range of values Windows expects
	// nice bitwise combination here
	// ! CreateConsoleScreenBuffer is from the Windows API and
	// ? MSDN says the preferred way is to use virtual terminal sequences
	// https://docs.microsoft.com/en-us/windows/console/createconsolescreenbuffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	/// Display the frame
	// ! this is why we needed wide chars and wide strings; this output method requires it
	// the method takes our console and draws our screen array of the given dimensions starting at 0, 0 (top-left)
	WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

	// For getting screen info as the program runs
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD currentWindowSize;

	bool dbgSCSBS;
	bool dbgSCWI;

	/// Game loop
	bool bGameOver = false;

	while (!bGameOver)
	{
		GetConsoleScreenBufferInfo(hConsole, &info);
		currentWindowSize = GetLargestConsoleWindowSize(hConsole);
		int dx = info.dwSize.X - nScreenWidth;
		int dy = info.dwSize.Y - nScreenHeight;
		if (dx != 0 || dy != 0)
		{
			// Dealing with window sizes
			if (bLockedScreenSize)
			{
				// reset console size to pre-defined dimensions
				dbgSCSBS = SetConsoleScreenBufferSize(hConsole, srBufferSize);
				dbgSCWI = SetConsoleWindowInfo(hConsole, true, &srScreenSize);
			}
			else
			{
				nScreenWidth = max(info.srWindow.Right + 1, MIN_SCREEN_WIDTH);
				nScreenHeight = max(info.srWindow.Bottom + 1, MIN_SCREEN_HEIGHT);
				srScreenSize.Right = max(info.srWindow.Right, MIN_SCREEN_WIDTH - 1);
				srScreenSize.Bottom = max(info.srWindow.Bottom, MIN_SCREEN_HEIGHT - 1);
				srBufferSize.X = nScreenWidth;
				srBufferSize.Y = nScreenHeight;

				// lock it in (prevents buffer from getting too big and causing scroll bars)
				dbgSCSBS = SetConsoleScreenBufferSize(hConsole, srBufferSize);
				dbgSCWI = SetConsoleWindowInfo(hConsole, true, &srScreenSize);

				// until I can figure out a better solution, I need to wipe the console clean when resizes happen
				// ? replace this with a vector perhaps?
				delete[] screen; // ! causes a crash when the screen gets too small, hence minimum window size
				screen = new wchar_t[nScreenWidth * nScreenHeight];
				for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
				{
					screen[i] = L' ';
				}
			}
		}

		// Draw the playing field
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				// we're adding + 2 to add some padding off from the very top-left of the console window
				screen[(y + 2) * nScreenWidth + (x + 2)] = CHAR_SET[pField[y * nFieldWidth + x]];
			}
		}

		// Display the frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}
}