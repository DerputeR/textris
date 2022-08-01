﻿// Pulled from https://stackoverflow.com/a/69786995

#include <Windows.h>
#include <string>
struct Color
{
    int r;
    int g;
    int b;
};

std::string GetColorStr(const Color& fgColor, const Color& bgColor)
{
    std::string fgStr = "\x1b[38;2;" + std::to_string(fgColor.r) + ";" + std::to_string(fgColor.g) + ";" + std::to_string(fgColor.b) + "m";
    std::string bgStr = "\x1b[48;2;" + std::to_string(bgColor.r) + ";" + std::to_string(bgColor.g) + ";" + std::to_string(bgColor.b) + "m";
    return fgStr + bgStr;
}

std::wstring GetColorWStr(const Color& fgColor, const Color& bgColor)
{
    std::wstring fgStr = L"\x1b[38;2;" + std::to_wstring(fgColor.r) + L";" + std::to_wstring(fgColor.g) + L";" + std::to_wstring(fgColor.b) + L"m";
    std::wstring bgStr = L"\x1b[48;2;" + std::to_wstring(bgColor.r) + L";" + std::to_wstring(bgColor.g) + L";" + std::to_wstring(bgColor.b) + L"m";
    return fgStr + bgStr;
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
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);

    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    /*
    for (int r = 0; r < 256; r++)
    {
        for (int g = 0; g < 256; g++)
        {
            for (int b = 0; b < 256; b++)
            {
                wchar_t wBlock[] = L"%lsTEST%ls";
                // wchar_t wBlock[] = L"TEST";
                std::wstring fgStr = GetFGColorWStr({ r,g,b }, { 0,0,0 });
                std::wstring bgStr = GetBGColorWStr({ r,g,b }, { 0,0,0 });

                // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
                // Below is a very raw, unsafe version of the above post
                int fSize = swprintf( 
                    nullptr,
                    0,
                    wBlock,
                    fgStr, bgStr
                ) + 1; // extra space for the would-be '\0'
                
                swprintf(
                    &wBlock[0],
                    fSize,
                    wBlock,
                    fgStr, bgStr
                ); // null char is still written, but now we know exactly where it 
                

                // ReplaceChars(screen_data, wchar_label, screen_width - (format_size - 1), format_size - 1);
                // wprintf((GetColorWStr({ r, g, b }, { 0, 0, 0 }) + L"██").data());
                // wprintf(L"%s", "██");
                LPDWORD d = 0;
                WriteConsole(hOut, wBlock, fSize, d, NULL);
            }
        }
    }

    */
    printf((GetColorStr({0, 255, 255}, {10, 5, 60}) + "Hello World\n" + GetColorStr({ 204, 204, 204 }, { 12, 12, 12 })).data());
    wprintf((GetColorWStr({ 0, 255, 255 }, { 10, 5, 60 }) + L"Hello World\n" + GetColorWStr({ 204, 204, 204 }, { 12, 12, 12 })).data());

    WriteConsoleW(hOut, (GetColorStr({ 0, 255, 255 }, { 10, 5, 60 }) + "Hello World\n").data(), 100, NULL, NULL);

    system("pause");
}