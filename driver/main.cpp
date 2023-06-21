#include "main.h"
#include <ntddk.h>

VOID driverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    DriverObject->DriverUnload = driverUnload;
    UNREFERENCED_PARAMETER(RegistryPath);
    KdPrint(("Kernel driver loaded\n"));
    return 0;
}
