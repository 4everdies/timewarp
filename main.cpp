#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

DWORD GetProcId(const char* procName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_stricmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
        CloseHandle(hSnap);
    }
    return procId;
}

bool InjectDLL(DWORD processID, const std::string& dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) return false;

    void* location = VirtualAllocEx(hProcess, 0, dllPath.size() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!location) {
        CloseHandle(hProcess);
        return false;
    }

    WriteProcessMemory(hProcess, location, dllPath.c_str(), dllPath.size() + 1, 0);
    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"),
        location, 0, 0);
    if (hThread)
    {
        WaitForSingleObject(hThread, 5000);
        CloseHandle(hThread);
    }
    VirtualFreeEx(hProcess, location, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return true;
}

std::string GetExePath()
{
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string path = exePath;
    size_t pos = path.find_last_of("\\/");
    return (pos != std::string::npos) ? path.substr(0, pos) : path;
}

int main()
{
    std::string exeDir = GetExePath();
    std::string dllPath = exeDir + "\\ctw.dll";
    std::cout << "looking for minecraft" << std::endl;
    DWORD pid = 0;
    const char* targets[] = { "javaw.exe", "java.exe" };
    while (pid == 0)
    {
        for (int i = 0; i < 2; i++)
        {
            pid = GetProcId(targets[i]);
            if (pid != 0) break;
        }
        if (pid == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }

    std::cout << "injecting" << std::endl;

    if (InjectDLL(pid, dllPath))
        std::cout << "timewarp has been injected" << std::endl;
    else
        std::cout << "injection failed" << std::endl;
    std::cout << "this window will close in 5 seconds" << std::endl;
    Sleep(5000);
    return 0;
}
