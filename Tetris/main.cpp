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
const wchar_t CHAR_SET[11] = L" ABCDEFG=#";
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


void InitPieces(std::wstring (& tetrominos)[TETROMINO_COUNT])
{
	/// Create assets
	// TODO: update piece bounding box and rotation system to satisfy SRS
	// https://tetris.fandom.com/wiki/SRS
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
	// note that i've replaced all instances of wide strings and chars with their regular 1-byte counterparts
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
	int nPiecesDelivered = 0;

	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	CreateNewPiece(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

	int nTicksPerDrop = 20;
	int nDropTickCounter = 0;
	bool bForceDown = false;

	// ? is this efficient?
	std::vector<int> vLines;
	int nTicksPerLine = 10;
	int nLineTickCounter = 0;
	bool bLinePause = false;

	/// Input tracking
	// TODO better input mapping system
	const int INPUT_COUNT = 5;
	bool bKey[INPUT_COUNT];
	// rotate button locking to make rotations happen ONLY on initial key press
	bool bRotateHold = false;
	// drop button locking to make drops happen ONLY on initial key press
	bool bDropHold = false;

	while (!bGameOver)
	{
		/// GAME TIMING
		// 50 mspt -> 20 tps
		// TODO: implement delta-time to prevent slow-downs
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!bLinePause)
		{
			nDropTickCounter++;
		}
		bForceDown = (nDropTickCounter >= nTicksPerDrop);

		/// INPUT (no events, do this old-school style)
		// pulled from other console project vids
		for (int k = 0; k < INPUT_COUNT; k++)
		{
			/*
				loops thru array of 4 booleans representing the state of the keys
				uses the AsyncKeyState which checks if the button is pressed
				the function returns a short which acts as a BITFLAG
				according to https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate
				the most significant digit is 1 when the key is pressed; other bits may get set for other behavior,
				but we only want the first digit, so we bitmask it with 0x8000 to get a zero or non-zero (false or true)
				value for checking this specific bit
																		 R   L   D   U	   _		
				VK_RIGHT = 0x27
				VK_LEFT = 0x25
				VK_DOWN = 0x28
				VK_UP = 0x26
				VK_SPACE = 0x20
			*/
			// TODO: Move key mapping array out of here so it is only created once and can be remapped at runtime
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26\x20"[k])));
		}

		/// GAME LOGIC
		if (!bLinePause)
		{
			// if right key pressed
			nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
			// if left key pressed
			nCurrentX += (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? -1 : 0;
			// if down key pressed
			if (bKey[2])
			{
				if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				{
					nCurrentY += 1;
				}
				else
				{
					// manual force down when the piece can move no further down and the down key is pressed
					// bForceDown = true;
				}
			}
			// if rotate key pressed
			if (bKey[3])
			{
				nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
				bRotateHold = true;
			}
			else // rotate lock
			{
				bRotateHold = false;
			}
			// snap down key
			if (bKey[4])
			{
				if (!bDropHold)
				{
					while (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
					{
						nCurrentY++;
					}
					bForceDown = true;
					bDropHold = true;
				}
			}
			else // drop lock
			{
				bDropHold = false;
			}
		}

		/// Line removal
		if (!vLines.empty())
		{
			if (nLineTickCounter >= nTicksPerLine)
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
				bLinePause = false;
				nLineTickCounter = 0;
			}
			else
			{
				bLinePause = true;
				nLineTickCounter++;
			}
		}

		/// "gravity"
		if (bForceDown && !bLinePause)
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				nCurrentY++;
			}
			else
			{
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

				nPiecesDelivered++;
				nScore += 25;
				if (nPiecesDelivered % 10 == 0)
				{
					nTicksPerDrop -= nTicksPerDrop >= 10 ? 1 : 0;
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
							bLinePause = true;
						}
					}
				}

				// the more lines in a row, the much higher score you get
				nScore += vLines.empty() ? (1 << vLines.size()) : 0;

				/// choose next piece
				CreateNewPiece(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

				/// if next piece can't spawn, game over
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nDropTickCounter = 0;
		}

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
		// Draw the playing field
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				// we're adding + 2 to add some padding off from the very top-left of the console window
				screen[(y + 2) * nScreenWidth + (x + 2)] = CHAR_SET[pField[y * nFieldWidth + x]];
			}
		}

		// Since the current piece isn't part of the field yet, we draw it on top of the field after the field is drawn
		// DON'T DO THIS UNTIL THE LINE HAS CLEARED (if exists)
		// iterate thru entire piece
		if (!bLinePause)
		{
			for (int px = 0; px < 4; px++)
			{
				for (int py = 0; py < 4; py++)
				{
					if (tetrominos[nCurrentPiece][ConvertToRotatedIndex(px, py, nCurrentRotation)] == L'X')
					{
						// x TODO: find a more elegant/less voodoo number magic way of doing this
						// x nCurrentPiece + 65 lets us convert each piece (listed in order in the tetrominos array)
						// x to give us ABCDEFG in ASCII, which is what the pieces will be drawn as
						// x screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
						screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = CHAR_SET[nCurrentPiece + 1];
					}
				}
			}
		}

		// Draw the score
		// ! buffer size must be 16 since every string has a hidden \0 terminating character at the end
		// 2 chars out from left, 2 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (2 * nScreenWidth) + nFieldWidth + 2], 16, L"Score: %-8d", nScore);

		// Draw the next upcoming piece
		// 2 chars out from left, 4 rows down, 2 chars out from field
		swprintf_s(&screen[2 + (4 * nScreenWidth) + nFieldWidth + 2], 12, L"Next piece:");
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetrominos[vCurrentPieces.back()][ConvertToRotatedIndex(px, py, 0)] == L'X')
				{
					// x TODO: find a more elegant/less voodoo number magic way of doing this
					// x nCurrentPiece + 65 lets us convert each piece (listed in order in the tetrominos array)
					// x to give us ABCDEFG in ASCII, which is what the pieces will be drawn as
					// x screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
					screen[((py + 6) * nScreenWidth) + (nFieldWidth + px + 6)] = CHAR_SET[vCurrentPieces.back() + 1];
				}
				else
				{
					screen[((py + 6) * nScreenWidth) + (nFieldWidth + px + 6)] = L' ';
				}
			}
		}

		// Display the frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}
}