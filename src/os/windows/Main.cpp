#include <Windows.h>
#include "../../OpenTTD.hpp"
#if WITH_DEBUG
int main()
#else
INT CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    return openttd::gameMain();
}