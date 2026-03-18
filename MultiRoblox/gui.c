#include <windows.h>
#include "gui.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MULTIROBLOX_CLASS";

    if (!RegisterClass(&wc))
    {
        MessageBoxA(NULL, "Falha ao registrar a classe da janela.", "MultiRoblox", MB_ICONERROR);
        return 1;
    }

    g_hwnd = CreateWindowEx(
        0, "MULTIROBLOX_CLASS", "MultiRoblox",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 455,
        NULL, NULL, hInstance, NULL
    );

    g_title = CreateWindowA(
        "STATIC", "MultiRoblox 1.0.0",
        WS_VISIBLE | WS_CHILD,
        10, 10, 365, 20,
        g_hwnd, NULL, hInstance, NULL
    );

    g_textbox = CreateWindowA(
        "EDIT",
        "Bem-vindo ao MultiRoblox!\r\n\r\n"
        "COMO USAR:\r\n"
        "1. Abra este programa\r\n"
        "2. Abra quantas instancias do Roblox quiser\r\n"
        "3. O programa monitora e libera o mutex automaticamente\r\n\r\n"
        "NOTA: Funciona mesmo se o Roblox ja estiver aberto!\r\n"
        "Mantenha este programa aberto enquanto usar multiplas instancias.",
        WS_BORDER | WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_READONLY,
        10, 40, 365, 365,
        g_hwnd, NULL, hInstance, NULL
    );

    if (g_hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(g_hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
