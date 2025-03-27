#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <PID> <DLL_PATH>\n", argv[0]);
        return 1;
    }

    char* end; DWORD pid = strtoul(argv[1], &end, 10);

    if (*end != '\0' || pid == 0) {
        printf("Invalid PID\n");
        return 1;
    }

    wchar_t dllPath[MAX_PATH];
    if (mbstowcs(dllPath, argv[2], MAX_PATH) == (size_t)-1) {
        printf("Invalid DLL path\n");
        return 1;
    }

    HANDLE processHandle = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid
    );

    if (processHandle == NULL) {
        printf("func(OpenProcess failed). Reason: %lu\n", GetLastError());
        return 1;
    }

    SIZE_T pathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);
    LPVOID remoteBuffer = VirtualAllocEx(
        processHandle, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE
    );
    
    if (remoteBuffer == NULL) {
        printf("func(VirtualAllocEx) failed. Reason: %lu\n", GetLastError());
        CloseHandle(processHandle);
        return 1;
    }

    if (!WriteProcessMemory(
        processHandle, remoteBuffer, dllPath, pathSize, NULL
    )) {
        printf("func(WriteProcessMemory) failed. Reason: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    HMODULE kernel32 = GetModuleHandleW(L"Kernel32");
    if (kernel32 == NULL) {
        printf("func(GetModuleHandle) failed. Reason: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    PTHREAD_START_ROUTINE loadLibraryAddr = (PTHREAD_START_ROUTINE)
        GetProcAddress(kernel32, "LoadLibraryW");
    if (loadLibraryAddr == NULL) {
        printf("func(GetProcAddress) failed. Reason: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    HANDLE threadHandle = CreateRemoteThread(
        processHandle, NULL, 0, loadLibraryAddr, remoteBuffer, 0, NULL
    );
    if (threadHandle == NULL) {
        printf("func(CreateRemoteThread) failed. Reason: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    WaitForSingleObject(threadHandle, INFINITE);

    VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
    CloseHandle(threadHandle);
    CloseHandle(processHandle);

    return 0;
}
