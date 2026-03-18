#include <windows.h>
#include "gui.h"

HWND g_hwnd;
HWND g_title;
HWND g_textbox;

int main()
{
    HANDLE hMutex = CreateMutex(0, 1, L"ROBLOX_singletonEvent");
    if (hMutex == NULL)
    {
        MessageBoxA(NULL, "Falha ao criar o Mutex.", "MultiRoblox", MB_ICONERROR);
        return 1;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);

    return WinMain(hInstance, NULL, NULL, SW_SHOWDEFAULT);
}
