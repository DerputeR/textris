#pragma once
namespace ScrappyCharFuncs
{
	void ReplaceChars(char* string, char* replace, int start, int len);
	void ReplaceChars(wchar_t* string, wchar_t* replace, int start, int len);

	void ReplaceCharsFill(char* string, char fill, int start, int len);
	void ReplaceCharsFill(wchar_t* string, wchar_t fill, int start, int len);
}