// Pulled from https://stackoverflow.com/a/69786995

#include <Windows.h>
#include <string>
#include <iostream>
#include "ColorConversions.h"
#include <vector>
#include <chrono>
#include "ScrappyCharFuncs.h"

using sec = std::chrono::duration<double, std::ratio<1, 1>>;

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

size_t GetColorWStrFG(const Color& fgColor, std::wstring& fgStr)
{
    fgStr = L"\x1b[38;2;000;000;000m";
    swprintf(&fgStr[7], 13, L"%03d;%03d;%03dm", fgColor.r, fgColor.g, fgColor.b);
    return fgStr.size();
}

size_t GetColorWStrBG(const Color& bgColor, std::wstring& bgStr)
{
    bgStr = L"\x1b[48;2;000;000;000m";
    swprintf(&bgStr[7], 13, L"%03d;%03d;%03dm", bgColor.r, bgColor.g, bgColor.b);
    return bgStr.size();
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

    
    std::wstring wBlock = L"╬ ";
    std::wstring resetStr = GetColorWStr({ 204, 204, 204 }, { 12, 12, 12 });
    for (float s = 0; s <= 1; s += 0.05f)
    {
        for (int h = 0; h < 360; h++)
        {
            for (float v = 0; v <= 1; v += 0.05f)
            {
                int r = 0, g = 0, b = 0;
                HSVtoRGB_Int(&r, &g, &b, h, s, v);
                std::wstring startStr = GetColorWStr({ 0, 0, 0 }, { r, g, b });
                // 40 = 38 (for fg + bg codes) + 2 (for chars)
                WriteConsoleW(hOut, (startStr + wBlock).data(), 40, NULL, NULL);

            }
        }
    }
    WriteConsoleW(hOut, (resetStr).data(), 38, NULL, NULL);
    

    system("pause");
    system("cls");

    std::chrono::steady_clock::time_point t1, t2, t3;

    int bufferSizeUsed = 0;
    int height = 20;
    int width = 100;

    int lastAppliedRGB_FG = 0;
    int currentAppliedRGB_FG = 0;
    int lastAppliedRGB_BG = 0;
    int currentAppliedRGB_BG = 0;

    wchar_t* bigBufferC = new wchar_t[(38 + 1) * (width * height)];
    wchar_t charStr[] = L"░";
    int nextInsertIndex = 0;

    int i_counter = 0;
    int iterations = 60;
    double totalTime = 0;

    // getting font infoand updating it
    /* 
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi); // MUST BE DONE
    GetCurrentConsoleFontEx(hOut, false, &cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 1;
    cfi.dwFontSize.Y = 1;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    SetCurrentConsoleFontEx(hOut, NULL, &cfi);
    */

    do
    {
        t1 = std::chrono::steady_clock::now();

        SetConsoleCursorPosition(hOut, { 0, 0 });
        nextInsertIndex = 0;
        bufferSizeUsed = 0;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                lastAppliedRGB_FG = currentAppliedRGB_FG;
                lastAppliedRGB_BG = currentAppliedRGB_BG;

                int charPos = y * width + x;

                float h = ((float)charPos / (height * width)) * 360.0f;
                // h = ((int)(h / 30)) * 30.0f;
                float s = 1, v = 1;

                float vf = (rand() % 2048) / 2048.0f;

                int rf = 0, gf = 0, bf = 0; // for now, just black
                int rb = 0, gb = 0, bb = 0;
                HSVtoRGB_Int(&rb, &gb, &bb, h, s, v);
                HSVtoRGB_Int(&rf, &gf, &bf, h, s, vf);

                currentAppliedRGB_BG = RGBIntsToInt(rb, gb, bb);
                currentAppliedRGB_FG = RGBIntsToInt(rf, gf, bf);;

                std::wstring curCol;

                if (charPos == 0 || currentAppliedRGB_FG != lastAppliedRGB_FG)
                {
                    int colSize = GetColorWStrFG({ rf, gf, bf }, curCol);
                    ScrappyCharFuncs::ReplaceChars(
                        bigBufferC,
                        &curCol[0],
                        nextInsertIndex,
                        colSize
                    );
                    nextInsertIndex += colSize;
                }
                if (charPos == 0 || currentAppliedRGB_BG != lastAppliedRGB_BG)
                {
                    int colSize = GetColorWStrBG({ rb, gb, bb }, curCol);
                    ScrappyCharFuncs::ReplaceChars(
                        bigBufferC,
                        &curCol[0],
                        nextInsertIndex,
                        colSize
                    );
                    nextInsertIndex += colSize;
                }
                ScrappyCharFuncs::ReplaceChars(
                    bigBufferC,
                    charStr,
                    nextInsertIndex,
                    1
                );
                nextInsertIndex += 1;
            }
        }
        bufferSizeUsed = nextInsertIndex;

        t2 = std::chrono::steady_clock::now();

        WriteConsoleW(hOut, bigBufferC, bufferSizeUsed, NULL, NULL);

        t3 = std::chrono::steady_clock::now();

        std::chrono::duration<double> deltaTime1 = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::chrono::duration<double> deltaTime2 = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Prep took %F seconds\n").data(), deltaTime1);
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Writing took %F seconds\n").data(), deltaTime2);

        SetConsoleTitleA(("DT: " + std::to_string((deltaTime1 + deltaTime2).count()) + "; FPS: " + std::to_string(1 / (deltaTime1 + deltaTime2).count())).data());
        i_counter++;
        totalTime += deltaTime1.count() + deltaTime2.count();
    } while (i_counter < iterations);
    double avgFPS = iterations / totalTime;
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Average FPS over %d frames: %F\n").data(), iterations, avgFPS);
    system("pause");


    // system("cls");

    SetConsoleActiveScreenBuffer(hOut);

    WriteConsoleOutput(
        hOut,
        screen_data,
        { screen_width, screen_height },
        { 0, 0 },
        &sr_screen_size
    );

    system("pause");

    delete[] bigBufferC;
}