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
    std::wstring fgStr = L"\x1b[38;2;000;000;000m";
    swprintf(&fgStr[7], 13, L"%03d;%03d;%03dm", fgColor.r, fgColor.g, fgColor.b);
    std::wstring bgStr = L"\x1b[48;2;000;000;000m";
    swprintf(&bgStr[7], 13, L"%03d;%03d;%03dm", bgColor.r, bgColor.g, bgColor.b);
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

    int width = 102;
    int height = 20;
    int sWidth = 102;
    int sHeight = 30;
    COORD srBufferSize{ width, height };
    COORD srBufferSize2{ sWidth, sHeight };
    SMALL_RECT sr{ 0, 0, width - 1, height - 1 };
    SMALL_RECT sr2{ 0, 0, sWidth - 1, sHeight - 1 };

    // reset console size to pre-defined dimensions
    // ? doesn't work for some reason
    SetConsoleScreenBufferSize(hOut, srBufferSize2);
    SetConsoleWindowInfo(hOut, true, &sr2);

    int i_counter = 0;
    int iterations = 0;
    double totalTime = 0;
    std::chrono::steady_clock::time_point t1, t2, t3;

    // SetConsoleCP(65001);
    // SetConsoleOutputCP(65001);

    // ! Char by char WriteConsole 24-bit color showcase
    i_counter = 0;
    iterations = 0;
    if (iterations > 0)
    {
        WriteConsoleW(hOut, L"\x1b[?1049h", 9, NULL, NULL);
        std::wstring wBlock = L"╬ ";
        std::wstring resetStr = GetColorWStr({ 204, 204, 204 }, { 12, 12, 12 });
        int wc_charpos = 0;
        for (float s = 0; s <= 1; s += 0.05f)
        {
            for (int h = 0; h < 360; h++)
            {
                for (float v = 0; v <= 1; v += 0.05f)
                {
                    //wc_charpos++;
                    //if (wc_charpos >= width * height)
                    //{
                    //    wc_charpos = 0;
                    //    SetConsoleCursorPosition(hOut, { 0, 0 });
                    //}
                    int r = 0, g = 0, b = 0;
                    HSVtoRGB_Int(&r, &g, &b, h, s, v);
                    std::wstring startStr = GetColorWStr({ 0, 0, 0 }, { r, g, b });
                    // 40 = 38 (for fg + bg codes) + 2 (for chars)
                    WriteConsoleW(hOut, (startStr + wBlock).data(), 40, NULL, NULL);

                }
            }
        }
        WriteConsoleW(hOut, (resetStr).data(), 38, NULL, NULL);
        WriteConsoleW(hOut, L"\x1b[?1049l", 9, NULL, NULL);

        system("pause");
        system("cls");
    }


    // ! Future tests setup
    int bufferSizeUsed = 0;

    int lastAppliedRGB_FG = 0;
    int currentAppliedRGB_FG = 0;
    int lastAppliedRGB_BG = 0;
    int currentAppliedRGB_BG = 0;

    wchar_t* bigBufferC = new wchar_t[(38 + 1) * (width * height)];
    wchar_t charStr[] = L"░";
    std::wstring curCol;
    int nextInsertIndex = 0;

 

    // ! WriteConsole Benchmark
    // very slow, but 24-bit color!
    i_counter = 0;
    iterations = 0;
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
        SetConsoleCursorPosition(hOut, { 0, (short)height });
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Prep took %F seconds\n").data(), deltaTime1);
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Writing took %F seconds\n").data(), deltaTime2);

        SetConsoleTitleA(("DT: " + std::to_string((deltaTime1 + deltaTime2).count()) + "; FPS: " + std::to_string(1 / (deltaTime1 + deltaTime2).count())).data());
        i_counter++;
        totalTime += deltaTime1.count() + deltaTime2.count();
    } while (i_counter < iterations);
    double avgFPS = iterations / totalTime;
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Average FPS over %d frames: %F\n").data(), iterations, avgFPS);
    delete[] bigBufferC;
    system("pause");

    system("cls");

    // ! Alternate WriteConsole Benchmark 1
    // wchar_t* currentChar = new wchar_t[38 + 1];
    wchar_t* currentChar = new wchar_t[19 + 1];
    i_counter = 0;
    iterations = 0;
    totalTime = 0;
    do
    {
        SetConsoleCursorPosition(hOut, { 0, 0 });

        t1 = std::chrono::steady_clock::now();
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int charPos = y * width + x;
                // currentChar[38] = charStr[0];
                currentChar[19] = L' ';

                float h = ((float) (charPos % width) / width) * 360.0f;
                // h = ((int)(h / 30)) * 30.0f;
                float s = (float) i_counter / iterations;
                float v = (float)(charPos / width) / height;

                float vf = (rand() % 2048) / 2048.0f;

                int rf = 0, gf = 0, bf = 0; // for now, just black
                int rb = 0, gb = 0, bb = 0;

                // test strobing
                //if (i_counter % 2 == 0)
                //{
                //    v = 0;
                //    s = 0;
                //}


                HSVtoRGB_Int(&rb, &gb, &bb, h, s, v);
                HSVtoRGB_Int(&rf, &gf, &bf, h, s, vf);

                //curCol = GetColorWStr({ rf, gf, bf }, { rb, gb, bb });
                GetColorWStrBG({ rb, gb, bb }, curCol);

                //ScrappyCharFuncs::ReplaceChars(
                //    currentChar,
                //    &curCol[0],
                //    0,
                //    38
                //);
                ScrappyCharFuncs::ReplaceChars(
                    currentChar,
                    &curCol[0],
                    0,
                    19
                );

                // WriteConsoleW(hOut, currentChar, 39, NULL, NULL);
                WriteConsoleW(hOut, currentChar, 20, NULL, NULL);
            }
        }
        t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime1 = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        totalTime += deltaTime1.count();
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Full write took %F seconds\n").data(), deltaTime1);

        SetConsoleTitleA(("DT: " + std::to_string((deltaTime1).count()) + "; FPS: " + std::to_string(1 / (deltaTime1).count())).data());
        i_counter++;

    } while (i_counter < iterations);
    avgFPS = iterations / totalTime;
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Average FPS over %d frames: %F\n").data(), iterations, avgFPS);
    delete[] currentChar;
    system("pause");

    // ! Alternate WriteConsole Benchmark 2
    bigBufferC = new wchar_t[(19 + 1) * (width * height)];
    i_counter = 0;
    iterations = -1;
    totalTime = 0;
    int angle = 0;
    const double pi = 3.14159265358979323846;
    do
    {
        SetConsoleCursorPosition(hOut, { 0, 0 });
        int nextIndex = 0;

        t1 = std::chrono::steady_clock::now();
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int charPos = y * width + x;

                float h = ((float)((charPos + i_counter) % width) / width) * 360.0f;
                // h = ((int)(h / 30)) * 30.0f;
                float v;
                if (iterations > 0)
                    v = (float)(i_counter) / iterations;
                else
                    v = 0.5 * ((std::sin((angle/180.0) * pi)) + 1);
                float s = (float)(charPos / width) / height;

                int rb = 0, gb = 0, bb = 0;

                HSVtoRGB_Int(&rb, &gb, &bb, h, s, v);

                GetColorWStrBG({ rb, gb, bb }, curCol);

                ScrappyCharFuncs::ReplaceChars(
                    bigBufferC,
                    &curCol[0],
                    nextIndex,
                    19
                );
                nextIndex += 19;
                bigBufferC[nextIndex++] = L' ';
            }
        }
        WriteConsoleW(hOut, bigBufferC, nextIndex, NULL, NULL);

        t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime1 = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        totalTime += deltaTime1.count();
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Full write took %F seconds\n").data(), deltaTime1);

        SetConsoleTitleA(("DT: " + std::to_string((deltaTime1).count()) + "; FPS: " + std::to_string(1 / (deltaTime1).count())).data());
        if (iterations == -1)
            i_counter = (i_counter + 1) % (width * height);
        else
            i_counter++;
        angle = (angle + 1) % 360;

    } while (iterations == -1 || i_counter < iterations);
    avgFPS = iterations / totalTime;
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Average FPS over %d frames: %F\n").data(), iterations, avgFPS);
    delete[] bigBufferC;
    system("pause");

    // ! WriteConsoleOut Benchmark
    // ? this prints weirdly and causes a crash when resizing...
    SetConsoleActiveScreenBuffer(hOut);
    CHAR_INFO* screen_data = new CHAR_INFO[width * height];
    i_counter = 0;
    iterations = 60;
    totalTime = 0;
    do
    {
        t1 = std::chrono::steady_clock::now();

        for (int i = 0; i < width * height; i++)
        {
            screen_data[i].Char.UnicodeChar = L'a' + i;
            screen_data[i].Attributes = i;
        }
        WriteConsoleOutputW(
            hOut,
            screen_data,
            { (short)width, (short)height },
            { 0, 0 },
            &sr
        );

        i_counter++;
        t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime1 = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        totalTime += deltaTime1.count();
        SetConsoleCursorPosition(hOut, { 0, (short)height });
        printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Time to WriteConsoleOutput: %F\n").data(), deltaTime1.count());
        SetConsoleTitleA(("DT: " + std::to_string((deltaTime1).count()) + "; FPS: " + std::to_string(1 / (deltaTime1).count())).data());
    }
    while (i_counter < iterations);
    avgFPS = iterations / totalTime;
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Average FPS over %d frames: %F\n").data(), iterations, avgFPS);
    delete[] screen_data;

    system("pause");
    // system("cls");

    t2 = std::chrono::steady_clock::now();

    wchar_t* screenChars = new wchar_t[width * height];
    for (int i = 0; i < width * height; i++)
    {
        screenChars[i] = L"█▓▒░▒▓"[i%6];
    }
    DWORD dw;
    WriteConsoleOutputCharacterW(hOut, screenChars, width * height, { 0, 0 }, &dw);

    t3 = std::chrono::steady_clock::now();
    std::chrono::duration<double> deltaTime2 = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    SetConsoleCursorPosition(hOut, { 0, (short)height });
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Time to WriteConsoleOutputCharacter: %F\n").data(), deltaTime2.count());
    SetConsoleTitleA(("DT: " + std::to_string((deltaTime2).count()) + "; FPS: " + std::to_string(1 / (deltaTime2).count())).data());
    delete[] screenChars;
    system("pause");

    
    
}