#include <Windows.h>
#include "../../OpenTTD.hpp"



INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    openttd::OpenTTD openTTD;
    openTTD.run();
    return 0;
}