// Pulled from https://stackoverflow.com/a/69786995

#include <Windows.h>
#include <string>
#include <iostream>
#include "ColorConversions.h"
#include <vector>
#include <chrono>
#include "ScrappyCharFuncs.h"
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
    // fgStr = L"\x1b[38;2;000;000;000m";
    // swprintf(&fgStr[0], 20, L"\x1b[38;2;%03d;%03d;%03dm", fgColor.r, fgColor.g, fgColor.b);
    fgStr = L"\x1b[38;2;" + std::to_wstring(fgColor.r) + L";" + std::to_wstring(fgColor.g) + L";" + std::to_wstring(fgColor.b) + L"m";
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
    std::chrono::steady_clock::time_point p1, p2, p3;
    using sec = std::chrono::duration<double, std::ratio<1,1>>;
    double dt1, dt2;

    double callTimes = 0;
    double callTimes2 = 0;

    std::wstring testCall;
    std::wstring testCall2;

    for (int i = 0; i < 5000; i++)
    {
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;

        // Test Call
        p1 = std::chrono::steady_clock::now();
        size_t testCallSize = GetColorWStrFG({ r,g,b }, testCall);
        p2 = std::chrono::steady_clock::now();
        size_t testCallSize2 = GetColorWStrBG({ r,g,b }, testCall2);
        p3 = std::chrono::steady_clock::now();

        dt1 = std::chrono::duration_cast<sec>(p2 - p1).count();
        dt2 = std::chrono::duration_cast<sec>(p3 - p2).count();

        callTimes += dt1;
        callTimes2 += dt2;
    }

    callTimes /= 5000;
    callTimes2 /= 5000;

    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);

    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    // SetConsoleCP(65001);
    // SetConsoleOutputCP(65001);

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

    std::chrono::steady_clock::time_point t1, t2, t3;

    int colorStrSize_noNull = 38;
    int bufferSizeUsed = 0;
    int height = 30;
    int width = 100;

    int lastAppliedRGB = 0;
    int currentAppliedRGB = 0;
    int nextInsertPoint = 0;

    wchar_t* bigBufferC = new wchar_t[(38 + 1) * (width * height)];
    wchar_t* currentChar = new wchar_t[39];

    t1 = std::chrono::steady_clock::now();

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            lastAppliedRGB = currentAppliedRGB;

            int charPos = y * width + x;

            float h = ((float) charPos / (height * width)) * 360.0f;
            h = ((int)(h / 30)) * 30.0f;
            float s = 1, v = 1;
            int r = 0, g = 0, b = 0;
            HSVtoRGB_Int(&r, &g, &b, h, s, v);

            currentAppliedRGB = RGBIntsToInt(r, g, b);
            if (currentAppliedRGB != lastAppliedRGB)
            {
                std::wstring colorStr = GetColorWStr({ 0, 0, 0 }, { r, g, b });
                ScrappyCharFuncs::ReplaceChars(
                    currentChar,
                    &colorStr[0],
                    0,
                    38
                );
                currentChar[38] = L'░';
                ScrappyCharFuncs::ReplaceChars(
                    bigBufferC,
                    currentChar,
                    nextInsertPoint,
                    (colorStrSize_noNull + 1)
                );
                nextInsertPoint += colorStrSize_noNull + 1;
            }
            else
            {
                currentChar[0] = L'░';
                ScrappyCharFuncs::ReplaceChars(
                    bigBufferC,
                    currentChar,
                    nextInsertPoint,
                    1
                );
                nextInsertPoint += 1;
            }

            
            
            /*ScrappyCharFuncs::ReplaceChars(
                bigBufferC,
                &(colorStr + charStr)[0],
                charPos * (colorStrSize_noNull + 1),
                (colorStrSize_noNull + 1)
            );*/
            // bufferSizeUsed += 1 + colorStrSize_noNull;
        }
    }
    bufferSizeUsed = nextInsertPoint;
    t2 = std::chrono::steady_clock::now();

    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), bigBufferC, bufferSizeUsed, NULL, NULL);
    // wprintf(bigBufferC);

    t3 = std::chrono::steady_clock::now();

    std::chrono::duration<double> deltaTime1 = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::chrono::duration<double> deltaTime2 = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Prep took %F seconds\n").data(), deltaTime1);
    printf((GetColorStr({ 204, 204, 204 }, { 12, 12, 12 }) + "Writing took %F seconds\n").data(), deltaTime2);

    SetConsoleTitleA(("DT: " + std::to_string((deltaTime1 + deltaTime2).count()) + "; FPS: " + std::to_string(1 / (deltaTime1 + deltaTime2).count())).data());

    system("pause");

    delete[] bigBufferC;
}