// Pulled from https://stackoverflow.com/a/69786995

#include <Windows.h>
#include <string>
#include <iostream>
#include "ColorConversions.h"
#include <vector>
#include <chrono>
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
    // std::wstring fgStr = L"\x1b[38;2;" + std::to_wstring(fgColor.r) + L";" + std::to_wstring(fgColor.g) + L";" + std::to_wstring(fgColor.b) + L"m";
    std::wstring fgStr = L"\x1b[38;2;000;000;000m";
    swprintf(&fgStr[0], 20, L"\x1b[38;2;%03d;%03d;%03dm", fgColor.r, fgColor.g, fgColor.b);
    // std::wstring bgStr = L"\x1b[48;2;" + std::to_wstring(bgColor.r) + L";" + std::to_wstring(bgColor.g) + L";" + std::to_wstring(bgColor.b) + L"m";
    std::wstring bgStr = L"\x1b[48;2;000;000;000m";
    swprintf(&bgStr[0], 20, L"\x1b[48;2;%03d;%03d;%03dm", bgColor.r, bgColor.g, bgColor.b);
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

    std::wstring screenBuffer_chars;
    std::wstring screenBuffer_colors;
    std::wstring screenBuffer_all;

    size_t totalColorStringSize = 0;

    for (int x = 0; x < 256; x++)
    {
        std::wstring wBlock = L"██";
        std::wstring startStr = GetColorWStr({ x, x, x }, { x, x, x });
        screenBuffer_all += startStr + wBlock;
        totalColorStringSize += startStr.size(); // += 38 each time
    }
    // this is even weirder shit
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (screenBuffer_all).data(), screenBuffer_all.size(), NULL, NULL);

    /*
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
    */

    // worst case; testing a 1920x1080, 1x1 pixel char, no same colors next to each other horizontally
    std::wstring big_buffer((38 + 1) * (1920 * 1080), L' ');
    std::chrono::steady_clock::time_point t1, t2, t3;
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "The 1080p buffer is %zd chars long, consuming %zd bytes\n").data(), big_buffer.size(), sizeof(big_buffer[0]) * big_buffer.size());
    
    t1 = std::chrono::steady_clock::now();

    /// STRING REPLACE TEST - pass (does not reallocate as long as replacement size <= orig size)
    // std::string originalStr = "Heeeeearara!!eraraee";
    // originalStr.replace(0, 4, "EE!!");


    // print this ridiculous thing
    
    
    int colorStrSize_noNull = 38;
    int bufferSizeUsed = 0;
    int height = 20;
    int width = 80;

    big_buffer = std::wstring((38 + 1) * (width * height), L' ');


    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int charPos = y * width + x;

            float h = ((float) charPos / (height * width)) * 360.0f;
            float s = 1, v = 1;
            int r = 0, g = 0, b = 0;
            HSVtoRGB_Int(&r, &g, &b, h, s, v);

            std::wstring charStr = L"░";
            std::wstring colorStr = GetColorWStr({ 0, 0, 0 }, { r, g, b });
            
            big_buffer.replace(charPos * (colorStrSize_noNull + 1), (colorStrSize_noNull + 1), colorStr + charStr);

            bufferSizeUsed += 1 + colorStrSize_noNull;
        }
    }
    t2 = std::chrono::steady_clock::now();

    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (big_buffer).data(), bufferSizeUsed, NULL, NULL);

    t3 = std::chrono::steady_clock::now();

    std::chrono::duration<double> deltaTime1 = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::chrono::duration<double> deltaTime2 = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Prep took %F seconds\n").data(), deltaTime1);
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Writing took %F seconds\n").data(), deltaTime2);

    SetConsoleTitleA(("DT: " + std::to_string((deltaTime1 + deltaTime2).count()) + "; FPS: " + std::to_string(1 / (deltaTime1 + deltaTime2).count())).data());

    system("pause");
}