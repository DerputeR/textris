#include <iostream>
#include <string>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>

/// <summary>
/// https://www.youtube.com/watch?v=8OK8_tHeCIA&list=PLrOv9FMX8xJE8NgepZR1etrsU63fDDGxO
/// Current time: 29:00
/// </summary>

const int TETROMINO_COUNT = 7;
std::wstring tetrominos[TETROMINO_COUNT];
const wchar_t CHAR_SET[11] = L" ███████⁄#";
std::vector<int> vCurrentPieces;

// TODO make this customizable at runtime
int nFieldWidth = 12;
int nFieldHeight = 18;
const int MIN_SCREEN_WIDTH = 80;
const int MIN_SCREEN_HEIGHT = 30;
int nScreenWidth = 120; // Default cmd.exe width (# of columns)
int nScreenHeight = 30; // Default cmd.exe height (# of rows)
SMALL_RECT srScreenSize{0, 0, nScreenWidth - 1, nScreenHeight - 1};
COORD srBufferSize{nScreenWidth, nScreenHeight}; 
bool bLockedScreenSize = false;

// we will store the elements of the playing field in an array of unsigned characters, allocated dynamically
unsigned char* pField = nullptr;

// todo: reference https://brianrackle.github.io/update/c++/2014/08/03/easy-timing/
const int TICKRATE = 60;
double dDeltaTime = 0.0;
double dAccumulator_FrameTime = 0.0;
double dTimeToSleep = 0.0;

std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::milli>> start;
std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::milli>> end;
std::chrono::duration<double, std::milli> miliseconds_elapsed{ 0 };

/// Input setup
struct Key
{
	int keycode;
	bool pressed = false;
	bool holding = false;
	double dAccumulator_KeyRepeatDelay = 0.0;
	double dAccumulator_KeyRepeatRate = 0.0;
	float fKeyRepeatDelay = 170.0f;
	float fKeyRepeatRate = 50.0f;

	Key(int keycode, float repeatDelay = 170.0f, float repeatRate = 50.0f)
		: keycode{ keycode }, fKeyRepeatDelay{ repeatDelay }, fKeyRepeatRate{ repeatRate }
	{ }

	void UpdateKey(bool activated)
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

private:
	Key();
};

HANDLE console_window = GetConsoleWindow();
HANDLE foreground_window = GetForegroundWindow();

void InitPieces(std::wstring (& tetrominos)[TETROMINO_COUNT])
{
	/// Create assets
	// TODO: update piece bounding box and rotation system to satisfy SRS
	// https://tetris.fandom.com/wiki/SRS
	tetrominos[0].append(L"....");
	tetrominos[0].append(L"XXXX");
	tetrominos[0].append(L"....");
	tetrominos[0].append(L"....");

	tetrominos[1].append(L"....");
	tetrominos[1].append(L".XX.");
	tetrominos[1].append(L"..XX");
	tetrominos[1].append(L"....");

	tetrominos[2].append(L"....");
	tetrominos[2].append(L"..XX");
	tetrominos[2].append(L".XX.");
	tetrominos[2].append(L"....");

	tetrominos[3].append(L"....");
	tetrominos[3].append(L".XX.");
	tetrominos[3].append(L".XX.");
	tetrominos[3].append(L"....");

	tetrominos[4].append(L"....");
	tetrominos[4].append(L"..X.");
	tetrominos[4].append(L".XXX");
	tetrominos[4].append(L"....");

	tetrominos[5].append(L"....");
	tetrominos[5].append(L"..X.");
	tetrominos[5].append(L"XXX.");
	tetrominos[5].append(L"....");

	tetrominos[6].append(L"....");
	tetrominos[6].append(L".X..");
	tetrominos[6].append(L".XXX");
	tetrominos[6].append(L"....");
}

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

/// <summary>
/// Tetromino collision test
/// </summary>
/// <param name="nTetromino">the type of tetromino</param>
/// <param name="nRotation">0 for 0, 1 for 90, 2 for 180, 3 for 270</param>
/// <param name="nPosX">x-coord of top-left of piece in the field</param>
/// <param name="nPosY">y-coord of top-left of piece in the field</param>
/// <returns></returns>
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	// Iterate through the entire tetromino piece (rotated accordingly) and check if it overlaps with anything
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = ConvertToRotatedIndex(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			// First make sure we're not going out of bounds
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					// fail on first overlap
					if (tetrominos[nTetromino][pi] == L'X' && pField[fi] != 0)
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

void FillAndPermutateIntVector(std::vector<int>& vec, int count)
{
	for (int i = 0; i < count; i++)
	{
		vec.push_back(i);
	}
	for (int i = count - 1; i > 0; i--)
	{
		int randIndex = rand() % i;
		int temp = vec[randIndex];
		vec[randIndex] = vec[i];
		vec[i] = temp;
	}
}

/// <summary>
/// https://tetris.fandom.com/wiki/Random_Generator
/// </summary>
/// <param name="currentPieces">an empty vector to hold the permutation</param>
/// <returns>Next piece in the permutation</returns>
int PopNextPiece()
{
	int nextPiece = vCurrentPieces.back();
	vCurrentPieces.pop_back();
	// refill permutation set when last item is used
	if (vCurrentPieces.empty())
	{
		FillAndPermutateIntVector(vCurrentPieces, TETROMINO_COUNT);
	}
	return nextPiece;
}

void CreateNewPiece(int& piece, int& rotation, int& x, int& y)
{
	piece = PopNextPiece();
	rotation = 0;
	x = nFieldWidth / 2;
	y = 0;
}

void TracePieceToBottom(int& piece, int& rotation, int& x, int& y)
{
	while (DoesPieceFit(piece, rotation, x, y + 1))
	{
		y++;
	}
}

float CalculateGravityTimeMS(int level)
{
	return pow(0.8 - ((level - 1) * 0.007f), level - 1) * 1000.0f;
}

int main()
{
	srand(time(NULL));

	// initialize pieces and permutation vector
	InitPieces(tetrominos);
	FillAndPermutateIntVector(vCurrentPieces, TETROMINO_COUNT);

	/// Construct the playing field array
	// TODO Move this into separate function
	// ! remember to delete this later when we're done with it
	pField = new unsigned char[nFieldWidth * nFieldHeight]; // create array
	for (int x = 0; x < nFieldWidth; x++) // Board boundary
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			/// if this part of the array corresponds to the left/right edge of the field OR the bottom, fill it
			// ? why are we using 9 and 0 and not specific chars?
			// SEE HOW THIS CORRESPONDS TO THE CHAR INDICES IN CHAR_SET
			// We're doing rendering separate from the core logic
			// ! recall we are essentially flattening a 2D gridspace into a 1D array representation
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	// this comes from a previous FPS video, https://www.youtube.com/watch?v=xW8skO7MFYw
	// x note that i've replaced all instances of wide strings and chars with their regular 1-byte counterparts
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		screen[i] = L' ';
	}

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

	CONSOLE_FONT_INFOEX fontex;
	fontex.cbSize = sizeof(fontex);
	// GetCurrentConsoleFontEx(hConsole, 0, &fontex);
	fontex.nFont = 0;
	fontex.dwFontSize.X = 16;
	fontex.dwFontSize.Y = 16;
	fontex.FontFamily = FF_DONTCARE;
	fontex.FontWeight = FW_NORMAL;

	// SetCurrentConsoleFontEx(hConsole, NULL, &fontex);

	/// Display the frame
	// ! this is why we needed wide chars and wide strings; this output method requires it
	// the method takes our console and draws our screen array of the given dimensions starting at 0, 0 (top-left)
	WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

	// For getting screen info as the program runs
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD currentWindowSize;

	/// Debug stuff
	bool dbgSCSBS;
	bool dbgSCWI;

	/// Game loop
	bool bGameOver = false;
	// TODO: game pausing
	bool bPaused = false;
	int nScore = 0;
	int nLevel = 1;
	int nLinesCleared = 0;
	int nLinesJustCleared = 0;
	bool bJustLocked = false;
	int nCombo = -1;

	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	int nGhostX = nCurrentX;
	int nGhostY = nCurrentY;

	CreateNewPiece(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
	
	struct
	{
		const enum DROPTYPE: byte {
			NONE,
			SOFT,
			HARD
		};
		byte dropType = NONE;
		u_short rowsDropped = 0;
		void reset()
		{
			dropType = NONE;
			rowsDropped = 0;
		}
	} sDropData;

	// Timings (in ms)
	bool bForceDown = false;

	float fGravityTime = 1000.0f;
	float fLockDelay = 500.0f;
	float fRemovalTime = 500.0f;

	double dAccumulator_Gravity = 0.0;
	double dAccumulator_LockDelay = 0.0;
	double dAccumulator_RemovalTime = -1.0;

	// ? is this efficient? can we get away with a size 4 array?
	std::vector<int> vLines;


	bool bLinePause = false;

	/// Input tracking
	const int INPUT_COUNT = 8;
	Key input_keys[INPUT_COUNT] = {
		{VK_ESCAPE},
		{VK_RETURN},
		{VK_SPACE},
		{VK_UP},
		{VK_DOWN},
		{VK_RIGHT},
		{VK_LEFT},
		{0x43} // C key
	};
	bool bKey[INPUT_COUNT]{};


	// TODO better input mapping system

	start = std::chrono::steady_clock::now();
	end = std::chrono::steady_clock::now();

	while (!bGameOver)
	{
		/// GAME TIMING
		// 60 fps as per https://tetris.wiki/TGM_legend#Frame
		start = end;

		/// INPUT (no events, do this old-school style)
		foreground_window = GetForegroundWindow();
		// only process inputs when the console is focused
		if (foreground_window == console_window)
		{
			for (int k = 0; k < INPUT_COUNT; k++)
			{
				input_keys[k].UpdateKey(0x8000 & GetAsyncKeyState(input_keys[k].keycode));
			}
		}

		/// GAME LOGIC
		if (!bLinePause)
		{
			sDropData.reset();

			/// translate input state to action state
			for (int i = 0; i < INPUT_COUNT; i++)
			{
				bKey[i] = false;
				if (input_keys[i].pressed)
				{
					if (!input_keys[i].holding)
					{
						bKey[i] = true;
					}
					else
					{
						if (input_keys[i].dAccumulator_KeyRepeatDelay >= input_keys[i].fKeyRepeatDelay)
						{
							if (input_keys[i].dAccumulator_KeyRepeatRate >= input_keys[i].fKeyRepeatRate)
							{
								bKey[i] = true;
								input_keys[i].dAccumulator_KeyRepeatRate = 0;
							}
						}
					}
				}
			}
			
			// ? Very jank cancelation solution. Try not to use this in the rewrite.
			// if right key pressed
			nCurrentX += (bKey[5] && !input_keys[6].holding && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
			// if left key pressed
			nCurrentX += (bKey[6] && !input_keys[5].holding && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? -1 : 0;
			// if down key pressed
			if (bKey[4])
			{
				sDropData.dropType = sDropData.SOFT;
				if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				{
					bForceDown = true;
					sDropData.rowsDropped++;
				}
				else
				{
					// todo figure out what feels good for when the piece is at the bottom and you press/hold down
					bForceDown = true;
				}
			}
			// if rotate key pressed
			if (bKey[3] && !input_keys[3].holding)
			{
				nCurrentRotation += DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY) ? 1 : 0;
			}
			// hard drop - we want it to drop from where we currently see it, not where it's going to be after this tick
			if (bKey[2] && !input_keys[2].holding)
			{
				dAccumulator_Gravity = 0;
				bForceDown = true;
				sDropData.dropType = sDropData.HARD;
				sDropData.rowsDropped = nGhostY - nCurrentY;
				nCurrentX = nGhostX;
				nCurrentY = nGhostY;
				dAccumulator_LockDelay = fLockDelay;
			}
		}

		/// Line removal
		if (!vLines.empty())
		{
			if (dAccumulator_RemovalTime >= fRemovalTime)
			{
				for (int row : vLines)
				{
					// column by column
					for (int fx = 1; fx < nFieldWidth - 1; fx++)
					{
						// since the vector contains rows in order of top-most to bottom
						// we will shift everything down as we go top-down the field
						for (int fy = row; fy > 0; fy--)
						{
							pField[(fy * nFieldWidth) + fx] = pField[((fy - 1) * nFieldWidth) + fx];
						}
						// set topmost row to 0
						pField[fx] = 0;
					}
				}
				vLines.clear();
				nLinesJustCleared = 0;
				bLinePause = false;
				dAccumulator_RemovalTime = -1.0;
			}
			else
			{
				if (dAccumulator_RemovalTime < 0)
				{
					dAccumulator_RemovalTime = 0;
				}
				bLinePause = true;
			}
		}

		/// "gravity"
		if (bForceDown && !bLinePause)
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				dAccumulator_Gravity = 0;
				dAccumulator_LockDelay = 0;
				nCurrentY++;
			}
			else if (dAccumulator_LockDelay >= fLockDelay)
			{
				bJustLocked = true; // to be consumed by the scoring system
				/// lock current piece in the field
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetrominos[nCurrentPiece][ConvertToRotatedIndex(px, py, nCurrentRotation)] == L'X')
						{
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}

				/// do we have any horizontals? 
				// optimization: we don't need to check the entire board, just where the last piece was locked
				for (int py = 0; py < 4; py++)
				{
					// boundary check to avoid checking outside the array & playing field (possible with horizontal line pieces)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						bool dbg_bLine = true;
						// start at 1 since index 0 and index (nFieldWidth - 1) are boundaries
						// scan left to right
						for (int fx = 1; fx < nFieldWidth - 1; fx++)
						{
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + fx]) != 0; // bitwise version
							// bLine = bLine && (pField[(nCurrentY + py) * nFieldWidth + fx]) != 0; // logical version
						}

						if (bLine)
						{
							// replace lines with = character
							for (int fx = 1; fx < nFieldWidth - 1; fx++)
							{
								pField[(nCurrentY + py) * nFieldWidth + fx] = 8;
							}
							// add current row index to the vector to reference later for removal
							// ! by our iteration order, this will add rows top-to-bottom, left-to-right in the vector
							vLines.push_back(nCurrentY + py);
							nCombo++;
							nLinesCleared++;
							nLinesJustCleared++;
							bLinePause = true;
						}
					}
				}

				/// choose next piece
				CreateNewPiece(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

				/// if next piece can't spawn, game over
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		// ! Scoring
		// partial implementation as per https://tetris.wiki/Scoring
		if (bJustLocked)
		{
			switch (nLinesJustCleared)
			{
			case 0: // combo break
				nCombo = -1;
				break;
			case 1:
				nScore += 100 * nLevel;
			case 2:
				nScore += 300 * nLevel;
			case 3:
				nScore += 500 * nLevel;
			case 4:
				nScore += 800 * nLevel;
			default:
				if (nCombo > 0)
				{
					nScore += 50 * nCombo * nLevel;
				}
			}
			bJustLocked = false;
		}

		switch (sDropData.dropType)
		{
		case sDropData.SOFT:
			nScore += sDropData.rowsDropped;
			break;
		case sDropData.HARD:
			nScore += 2 * sDropData.rowsDropped;
			break;
		}

		sDropData.rowsDropped = 0;

		// update level
		nLevel = 1 + nLinesCleared / 10;
		if (nLevel <= 19)
		{
			fGravityTime = CalculateGravityTimeMS(nLevel);
		}

		/// GHOST PIECE
		nGhostX = nCurrentX;
		nGhostY = nCurrentY;
		TracePieceToBottom(nCurrentPiece, nCurrentRotation, nGhostX, nGhostY);

		/// CHECK WINDOW
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

		/// RENDER
		//! NOTE THAT DUE TO HOW CMD.exe RENDERS STUFF, I'M MAKING THE PLAYING AREA DOUBLE WIDTH
		// Draw the playing field
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				// we're adding + 2 to add some padding off from the very top-left of the console window
				// screen[(y + 2) * nScreenWidth + (x + 2)] = CHAR_SET[pField[y * nFieldWidth + x]];
				screen[(y + 2) * nScreenWidth + ((2 * x) + 2)] = CHAR_SET[pField[y * nFieldWidth + x]];
				screen[(y + 2) * nScreenWidth + ((2 * x + 1) + 2)] = CHAR_SET[pField[y * nFieldWidth + x]];
			}
		}

		// Since the current piece isn't part of the field yet, we draw it on top of the field after the field is drawn
		// DON'T DO THIS UNTIL THE LINE HAS CLEARED (if exists)
		// iterate thru entire piece
		// also draws the ghost piece
		if (!bLinePause)
		{
			for (int px = 0; px < 4; px++)
			{
				for (int py = 0; py < 4; py++)
				{
					if (tetrominos[nCurrentPiece][ConvertToRotatedIndex(px, py, nCurrentRotation)] == L'X')
					{
						// ghost piece draw
						SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
						// screen[(nGhostY + py + 2) * nScreenWidth + ((nGhostX + px) + 2)] = L'░';
						screen[(nGhostY + py + 2) * nScreenWidth + (2 * (nGhostX + px) + 2)] = L'░';
						screen[(nGhostY + py + 2) * nScreenWidth + (2 * (nGhostX + px) + 1 + 2)] = L'░';
						// real piece draw
						SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
						// screen[(nCurrentY + py + 2) * nScreenWidth + ((nCurrentX + px) + 2)] = CHAR_SET[nCurrentPiece + 1];
						screen[(nCurrentY + py + 2) * nScreenWidth + (2 * (nCurrentX + px) + 2)] = CHAR_SET[nCurrentPiece + 1];
						screen[(nCurrentY + py + 2) * nScreenWidth + (2 * (nCurrentX + px) + 1 + 2)] = CHAR_SET[nCurrentPiece + 1];
						//SetConsoleTextAttribute(hConsole, 7);
					}
				}
			}
		}

		// Draw the next upcoming piece
		// 2 chars out from left, 2 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (2 * nScreenWidth) + 2 * nFieldWidth + 4], 12, L"Next piece:");
		screen[2 + (2 * nScreenWidth) + 2 * nFieldWidth + 4 + 11] = L' '; // remove \0 char
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetrominos[vCurrentPieces.back()][ConvertToRotatedIndex(px, py, 0)] == L'X')
				{
					// screen[((py + 6) * nScreenWidth) + (nFieldWidth + px + 6)] = CHAR_SET[vCurrentPieces.back() + 1];
					screen[((py + 4) * nScreenWidth) + (2 * (nFieldWidth + px) + 6)] = CHAR_SET[vCurrentPieces.back() + 1];
					screen[((py + 4) * nScreenWidth) + (2 * (nFieldWidth + px) + 1 + 6)] = CHAR_SET[vCurrentPieces.back() + 1];
				}
				else
				{
					// screen[((py + 6) * nScreenWidth) + (nFieldWidth + px + 6)] = L' ';
					screen[((py + 4) * nScreenWidth) + (2 * (nFieldWidth + px) + 6)] = L'·';
					screen[((py + 4) * nScreenWidth) + (2 * (nFieldWidth + px) + 1 + 6)] = L'·';
				}
			}
		}

		// Draw the score
		// ! buffer size must be 16 since every string has a hidden \0 terminating character at the end
		// 2 chars out from left, 9 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (9 * nScreenWidth) + 2 * nFieldWidth + 4], 16, L"Score: %-8d", nScore);
		screen[2 + (9 * nScreenWidth) + 2 * nFieldWidth + 4 + 15] = L' '; // remove \0 char

		// Draw the level number
		// 2 chars out from left, 10 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (10 * nScreenWidth) + 2 * nFieldWidth + 4], 16, L"Level: %-8d", nLevel);
		screen[2 + (10 * nScreenWidth) + 2 * nFieldWidth + 4 + 15] = L' '; // remove \0 char

		// Draw the lines cleared
		// 2 chars out from left, 11 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (11 * nScreenWidth) + 2 * nFieldWidth + 4], 16, L"Lines: %-8d", nLinesCleared);
		screen[2 + (11 * nScreenWidth) + 2 * nFieldWidth + 4 + 15] = L' '; // remove \0 char

		// Draw combo
		// 2 chars out from left, 12 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (12 * nScreenWidth) + 2 * nFieldWidth + 4], 16, L"Combo: %-8d", nCombo + 1);
		screen[2 + (12 * nScreenWidth) + 2 * nFieldWidth + 4 + 15] = L' '; // remove \0 char

		// Display the frame
		WriteConsoleOutputCharacterW(
			hConsole, 
			screen, 
			nScreenWidth * nScreenHeight, 
			{ 0, 0 }, 
			&dwBytesWritten);

		/// Timing
		end = std::chrono::steady_clock::now();
		miliseconds_elapsed = end - start;
		dDeltaTime = miliseconds_elapsed.count();

		// Add to accumulators
		if (dAccumulator_RemovalTime > -1)
		{
			dAccumulator_RemovalTime = min(dAccumulator_RemovalTime + dDeltaTime, fRemovalTime);
		}

		if (!bLinePause)
		{
			dAccumulator_LockDelay = min(dAccumulator_LockDelay + dDeltaTime, fLockDelay);
			dAccumulator_Gravity = min(dAccumulator_Gravity + dDeltaTime, fGravityTime);
			bForceDown = (dAccumulator_Gravity >= fGravityTime);

			for (int i = 0; i < INPUT_COUNT; i++)
			{
				if (input_keys[i].pressed)
				{
					input_keys[i].dAccumulator_KeyRepeatDelay = min(input_keys[i].dAccumulator_KeyRepeatDelay + dDeltaTime, input_keys[i].fKeyRepeatDelay);
				}
				if (input_keys[i].holding)
				{
					input_keys[i].dAccumulator_KeyRepeatRate = min(input_keys[i].dAccumulator_KeyRepeatRate + dDeltaTime, input_keys[i].fKeyRepeatRate);
				}
				else
				{
					input_keys[i].dAccumulator_KeyRepeatDelay = 0;
					input_keys[i].dAccumulator_KeyRepeatRate = 0;
				}
			}
		}

		// Wrap up
		end = std::chrono::steady_clock::now();
		miliseconds_elapsed = end - start;
		dDeltaTime = miliseconds_elapsed.count();

		dTimeToSleep = (1000.0 / TICKRATE) - dDeltaTime;
		if (dTimeToSleep > 0 && dTimeToSleep < 1000.0 / TICKRATE)
		{
			// std::this_thread::sleep_for<double, std::milli>(std::chrono::duration<double>(dTimeToSleep));
			std::this_thread::sleep_for(std::chrono::milliseconds((int) dTimeToSleep));
		}
	}

	CloseHandle(hConsole);
	std::cout << "Game over! Score: " << nScore << std::endl;
	std::cout << "Press <ENTER> to exit" << std::endl;
	std::cin.get();
}