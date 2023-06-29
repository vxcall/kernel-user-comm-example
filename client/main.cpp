#include <windows.h>
#include <stdio.h>
#include "shared.h"

int Error(const char* message) {
    printf("%s (error=%u)\n", message, GetLastError());
    return 1;
}

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <thread id> <priority>\n", argv[0]);
        return 0;
    }
    int tid = atoi(argv[1]);
    int priority = atoi(argv[2]);

    HANDLE hDevice = CreateFileW(L"\\\\.\\Booster", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (hDevice == INVALID_HANDLE_VALUE)
        return Error("Failed to open device");

    ThreadData data;
    data.ThreadId = tid;
    data.Priority = priority;

    DWORD returned;
    BOOL success = WriteFile(hDevice, &data, sizeof(data), &returned, nullptr);
    if (!success)
        return Error("Priority change failed");

    printf("Priority change succeeded!\n");
    CloseHandle(hDevice);
}