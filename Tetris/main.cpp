#include <iostream>
#include <string>

/// <summary>
/// https://www.youtube.com/watch?v=8OK8_tHeCIA&list=PLrOv9FMX8xJE8NgepZR1etrsU63fDDGxO
/// Current time: 8:17
/// </summary>

std::string tetrominos[7];
// TODO make this customizable at runtime
int nFieldWidth = 12;
int nFieldHeight = 18;
// we will store the elements of the playing field in an array of unsigned characters, allocated dynamically
unsigned char* pField = nullptr;


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
	// Create assets
	tetrominos[0].append("..X.");
	tetrominos[0].append("..X.");
	tetrominos[0].append("..X.");
	tetrominos[0].append("..X.");

	tetrominos[1].append("..X.");
	tetrominos[1].append(".XX.");
	tetrominos[1].append(".X..");
	tetrominos[1].append("....");

	tetrominos[2].append(".X..");
	tetrominos[2].append(".XX.");
	tetrominos[2].append("..X.");
	tetrominos[2].append("....");

	tetrominos[3].append("....");
	tetrominos[3].append(".XX.");
	tetrominos[3].append(".XX.");
	tetrominos[3].append("....");

	tetrominos[4].append("..X.");
	tetrominos[4].append(".XX.");
	tetrominos[4].append("..X.");
	tetrominos[4].append("....");
	
	tetrominos[5].append("....");
	tetrominos[5].append(".XX.");
	tetrominos[5].append("..X.");
	tetrominos[5].append("..X.");

	tetrominos[6].append("....");
	tetrominos[6].append(".XX.");
	tetrominos[6].append(".X..");
	tetrominos[6].append(".X..");
}