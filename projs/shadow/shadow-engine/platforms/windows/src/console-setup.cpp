#include "console-setup.h"

#ifdef WIN32
#include <Windows.h>

#endif

void InitConsole() {
    #ifdef WIN32
    SetConsoleOutputCP(CP_UTF8);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 14;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, LF_FACESIZE, L"Lucida Console");
    if (SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi) == 0) {
        // handle error
    }
    #endif
}