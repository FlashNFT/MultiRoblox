#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <stdio.h>
#include "gui.h"

HWND g_hwnd;
HWND g_title;
HWND g_textbox;

// Definicoes para NtQuerySystemInformation
#define SystemHandleInformation 16
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004
#define OB_TYPE_MUTANT 17

typedef struct _SYSTEM_HANDLE_ENTRY {
    ULONG ProcessId;
    BYTE ObjectTypeNumber;
    BYTE Flags;
    USHORT Handle;
    PVOID Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_ENTRY, *PSYSTEM_HANDLE_ENTRY;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG HandleCount;
    SYSTEM_HANDLE_ENTRY Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef NTSTATUS(NTAPI* pNtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef NTSTATUS(NTAPI* pNtQueryObject)(
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
);

// Verifica se um processo eh RobloxPlayerBeta.exe
BOOL IsRobloxProcess(DWORD processId)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return FALSE;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (pe.th32ProcessID == processId) {
                CloseHandle(hSnap);
                return (_wcsicmp(pe.szExeFile, L"RobloxPlayerBeta.exe") == 0);
            }
        } while (Process32NextW(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return FALSE;
}

// Thread que monitora e fecha mutex do Roblox
DWORD WINAPI MutexMonitorThread(LPVOID lpParam)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return 1;

    pNtQuerySystemInformation NtQuerySystemInformation =
        (pNtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    pNtQueryObject NtQueryObject =
        (pNtQueryObject)GetProcAddress(hNtdll, "NtQueryObject");

    if (!NtQuerySystemInformation || !NtQueryObject) return 1;

    while (1) {
        Sleep(2000);

        ULONG bufferSize = 0x10000;
        PSYSTEM_HANDLE_INFORMATION handleInfo = NULL;
        NTSTATUS status;

        // Alocar buffer para informacoes de handles
        do {
            handleInfo = (PSYSTEM_HANDLE_INFORMATION)malloc(bufferSize);
            if (!handleInfo) break;

            status = NtQuerySystemInformation(SystemHandleInformation, handleInfo, bufferSize, NULL);
            if (status == (NTSTATUS)STATUS_INFO_LENGTH_MISMATCH) {
                free(handleInfo);
                handleInfo = NULL;
                bufferSize *= 2;
            }
        } while (status == (NTSTATUS)STATUS_INFO_LENGTH_MISMATCH);

        if (!handleInfo || status != 0) {
            if (handleInfo) free(handleInfo);
            continue;
        }

        DWORD myPid = GetCurrentProcessId();

        for (ULONG i = 0; i < handleInfo->HandleCount; i++) {
            SYSTEM_HANDLE_ENTRY entry = handleInfo->Handles[i];

            // Pular nosso proprio processo
            if (entry.ProcessId == myPid) continue;

            // Verificar apenas Mutant (mutex) type
            if (entry.ObjectTypeNumber != OB_TYPE_MUTANT) continue;

            // Verificar se eh processo do Roblox
            if (!IsRobloxProcess(entry.ProcessId)) continue;

            // Abrir o processo
            HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, entry.ProcessId);
            if (!hProcess) continue;

            // Duplicar o handle para poder inspecionar
            HANDLE hDup = NULL;
            if (DuplicateHandle(hProcess, (HANDLE)(ULONG_PTR)entry.Handle,
                GetCurrentProcess(), &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS)) {

                // Consultar o nome do objeto
                BYTE nameBuffer[1024];
                POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nameBuffer;

                if (NtQueryObject(hDup, ObjectNameInformation, nameInfo, sizeof(nameBuffer), NULL) == 0) {
                    if (nameInfo->Name.Buffer && wcsstr(nameInfo->Name.Buffer, L"ROBLOX_singletonEvent")) {
                        // Fechar o handle no processo do Roblox!
                        HANDLE hTemp = NULL;
                        DuplicateHandle(hProcess, (HANDLE)(ULONG_PTR)entry.Handle,
                            GetCurrentProcess(), &hTemp, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
                        if (hTemp) CloseHandle(hTemp);
                    }
                }
                CloseHandle(hDup);
            }
            CloseHandle(hProcess);
        }

        free(handleInfo);
    }

    return 0;
}

int main()
{
    // Criar o mutex antecipadamente (metodo classico)
    HANDLE hMutex = CreateMutex(0, 1, L"ROBLOX_singletonEvent");

    // Iniciar thread de monitoramento que fecha mutex em processos Roblox existentes
    HANDLE hThread = CreateThread(NULL, 0, MutexMonitorThread, NULL, 0, NULL);
    if (hThread) CloseHandle(hThread);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    return WinMain(hInstance, NULL, NULL, SW_SHOWDEFAULT);
}
