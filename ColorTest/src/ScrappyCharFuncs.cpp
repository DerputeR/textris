#include "ScrappyCharFuncs.h"

namespace ScrappyCharFuncs
{
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

	void ReplaceCharsFill(char* string, char fill, int start, int len)
	{
		for (int i = 0; i < len; i++)
		{
			string[i + start] = fill;
		}
	}

	void ReplaceCharsFill(wchar_t* string, wchar_t fill, int start, int len)
	{
		for (int i = 0; i < len; i++)
		{
			string[i + start] = fill;
		}
	}
}