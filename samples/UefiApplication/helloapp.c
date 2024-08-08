/*++

Copyright (c) Alex Ionescu.  All rights reserved.

Module Name:

    shvos.c

Abstract:

    This module implements the OS-facing UEFI stubs for SimpleVisor.

Author:

    Alex Ionescu (@aionescu) 29-Aug-2016 - Initial version

Environment:

    Kernel mode only.

--*/

//
// Basic UEFI Libraries
//
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

//
// Boot and Runtime Services
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// Shell Library
//
#include <Library/ShellLib.h>

// 
// Custom Driver Protocol 
// 
#include "../UefiDriver/drvproto.h"
EFI_GUID gEfiSampleDriverProtocolGuid = EFI_SAMPLE_DRIVER_PROTOCOL_GUID;

//
// We run on any UEFI Specification
//
extern CONST UINT32 _gUefiDriverRevision = 0;

//
// Our name
//
CHAR8 *gEfiCallerBaseName = "ShellSample";

EFI_STATUS
EFIAPI
UefiUnload (
    IN EFI_HANDLE ImageHandle
    )
{
    // 
    // This code should be compiled out and never called 
    // 
    ASSERT(FALSE);
}

EFI_STATUS
WaitForKeyStroke(
    OUT  EFI_INPUT_KEY* Key
)
{
    EFI_STATUS  Status;
    UINTN       Index;

    while (TRUE) {
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, Key);
        if (!EFI_ERROR(Status)) {
            break;
        }

        if (Status != EFI_NOT_READY) {
            continue;
        }

        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
    }
    return Status;
}

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
    )
{
    EFI_STATUS efiStatus;
    SHELL_FILE_HANDLE fileHandle;
    UINT8 buffer[4];
    UINTN readSize;
    EFI_SAMPLE_DRIVER_PROTOCOL* sampleProtocol;

    Print(L"Press any key to continue...\n");
    EFI_INPUT_KEY keyInput;
    efiStatus = WaitForKeyStroke(&keyInput);
    if (EFI_ERROR(efiStatus))
    {
        Print(L"Failed to get keystroke: %lx\n", efiStatus);
        goto Exit;
    }

    // 
    // Print stuff out 
    // 

    Print(L"Hello World UEFI Application and Driver [https://eavesdropper.dev]\n");
    Print(L"Developer: [un4ckn0wl3z]\n\n");

    fileHandle = NULL;
    Print(L"My handle is %lx and System Table is at %p\n",
          ImageHandle, SystemTable);

    //
    // Initialize the shell library
    //
    efiStatus = ShellInitialize();
    if (EFI_ERROR(efiStatus))
    {
        Print(L"Failed to initialize shell: %lx\n", efiStatus);
        goto Exit;
    }

    //
    // Open ourselves
    //
    efiStatus = ShellOpenFileByName(L"fs1:\\UefiApplication.efi",
                                    &fileHandle,
                                    EFI_FILE_MODE_READ,
                                    0);
    if (EFI_ERROR(efiStatus))
    {
        Print(L"Failed to open ourselves: %lx\n", efiStatus);
        fileHandle = NULL;
        goto Exit;
    }

    //
    // Read 4 bytes at the top (MZ header)
    //
    readSize = sizeof(buffer);
    efiStatus = ShellReadFile(fileHandle, &readSize, &buffer);
    if (EFI_ERROR(efiStatus))
    {
        Print(L"Failed to read ourselves: %lx\n", efiStatus);
        goto Exit;
    }

    //
    // Print it
    //
    Print(L"Data: %lx\n", *(UINT32*)buffer);

    // 
    // Check if the sample driver is loaded 
    // 
    efiStatus = gBS->LocateProtocol(&gEfiSampleDriverProtocolGuid, NULL, &sampleProtocol);
    if (EFI_ERROR(efiStatus))
    {
        Print(L"Failed to locate our driver: %lx\n", efiStatus);
        goto Exit;
    }

    // 
    // Print the value and exit 
    // 
    Print(L"Sample driver is loaded: %lx\n", sampleProtocol->SampleValue);

Exit:
    //
    // Close our file handle
    //
    if (fileHandle != NULL)
    {
        ShellCloseFile(&fileHandle);
    }

    //
    // Sample complete!
    //
    return efiStatus;
}

