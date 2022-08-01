// Pulled from https://stackoverflow.com/a/69786995

#include <Windows.h>
#include <string>
#include <iostream>
#include "ColorConversions.h"
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

    // SetConsoleCP(65001);
    // SetConsoleOutputCP(65001);


    for (int x = 0; x < 256; x++)
    {
        std::wstring wBlock = L"██";
        std::wstring startStr = GetColorWStr({ x, x, x }, { x, x, x });
        std::wstring resetStr = GetColorWStr({ 0, 0, 0 }, { 0, 0, 0 });
        // this is some weird shit
        wprintf((startStr + resetStr).data());
        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (wBlock).data(), 2, NULL, NULL);
    }
        
    for (float s = 0; s <= 1; s += 0.05f)
    {
        for (int h = 0; h < 360; h++)
        {
            for (float v = 0; v <= 1; v += 0.05f)
            {
                int r = 0, g = 0, b = 0;
                HSVtoRGB_Int(&r, &g, &b, h, s, v);


                std::wstring wBlock = L"╬ ";
                std::wstring startStr = GetColorWStr({ 0, 0, 0 }, { r, g, b });
                std::wstring resetStr = GetColorWStr({ 204, 204, 204 }, { 12, 12, 12 });
                wprintf((startStr).data());
                WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (wBlock).data(), 2, NULL, NULL);
                wprintf((resetStr).data());

            }
        }
    }
    
    //for (int r = 0; r < 256; r++)
    //{
    //    for (int g = 0; g < 256; g++)
    //    {
    //        for (int b = 0; b < 256; b++)
    //        {
    //            std::wstring wBlock = L"██";
    //            std::wstring startStr = GetColorWStr({ r, g, b }, { 0, 0, 0 });
    //            // std::wstring resetStr = GetColorWStr({ r, g, b }, { 0, 0, 0 });
    //            // this is some weird shit
    //            wprintf((startStr + wBlock).data());
    //            // wprintf((startStr + wBlock + resetStr).data());
    //            WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (wBlock).data(), 2, NULL, NULL);

    //        }
    //    }
    //}
    
    
    printf((GetColorStr({0, 255, 255}, {10, 5, 60}) + "Hello World\n" + GetColorStr({ 204, 204, 204 }, { 12, 12, 12 })).data());
    printf((GetColorStr({ 0, 255, 255 }, { 10, 5, 60 }) + "Hello World\n" + GetColorStr({ 204, 204, 204 }, { 12, 12, 12 })).data());

    wprintf((GetColorWStr({ 0, 255, 255 }, { 10, 5, 60 }) + L"Hello World\n" + GetColorWStr({ 204, 204, 204 }, { 12, 12, 12 })).data());
    wprintf((GetColorWStr({ 0, 255, 255 }, { 10, 5, 60 }) + L"██e\n" + GetColorWStr({ 204, 204, 204 }, { 12, 12, 12 })).data());

    system("pause");
}