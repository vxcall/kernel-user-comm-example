#include <ntifs.h>
#include "shared.h"

void BoosterUnload(PDRIVER_OBJECT DriverObject);

NTSTATUS BoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS BoosterWrite(PDEVICE_OBJECT, PIRP Irp);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    DriverObject->DriverUnload = BoosterUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = BoosterCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoosterCreateClose;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = BoosterWrite;

    UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\Booster");

    PDEVICE_OBJECT DeviceObject;
    NTSTATUS status = IoCreateDevice(
            DriverObject,
            0,
            &devName,
            FILE_DEVICE_UNKNOWN,
            0,
            FALSE,
            &DeviceObject);

    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to create device object (0x%08X)\n", status));
        return status;
    }
    KdPrint(("Successfully created device object\n"));

    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Booster");
    status = IoCreateSymbolicLink(&symLink, &devName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject); // undo every step
        return status;
    }
    KdPrint(("Successfully created symbolic link\n"));

    return STATUS_SUCCESS;
}

void BoosterUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Booster");
    IoDeleteSymbolicLink(&symLink); // delete symbolic link
    IoDeleteDevice(DriverObject->DeviceObject); // delete device object
}

NTSTATUS BoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS; // The value this request should be completed with.
    Irp->IoStatus.Information = 0;         // polymophic member. different things in different request. Create and Close are fine with 0.
    IoCompleteRequest(Irp, IO_NO_INCREMENT); // IO_NO_INCREMENT means zero.
    return STATUS_SUCCESS;
}

NTSTATUS BoosterWrite(PDEVICE_OBJECT, PIRP Irp) {
    auto status = STATUS_SUCCESS;
    ULONG_PTR information = 0;
    // retrieving IO_STACK_LOCATION
    auto irpSp = IoGetCurrentIrpStackLocation(Irp);
    do {
        // Checking the data size
        if (irpSp->Parameters.Write.Length < sizeof(ThreadData)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        // UserBuffer is typeof void pointer, so need to be cast
        auto data = static_cast<ThreadData*>(Irp->UserBuffer);
        // if data is null or, priority is in invalid range
        if (data == nullptr || data->Priority < 1 || data->Priority > 31) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        PETHREAD thread;
        status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &thread);
        if (!NT_SUCCESS(status))
            break;
        auto oldPriority = KeSetPriorityThread(thread, data->Priority);
        KdPrint(("Priority change for thread %u from %u to %u\n", data->ThreadId, oldPriority, data->Priority));
        ObDereferenceObject(thread);
        information = sizeof(data);
    } while (false);
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}