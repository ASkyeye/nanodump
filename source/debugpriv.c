#include "../include/debugpriv.h"
#include "dinvoke.c"

BOOL enable_debug_priv(void)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    LOOKUPPRIVILEGEVALUEW pLookupPrivilegeValueW;
    BOOL ok;

    // find the address of LookupPrivilegeValueW dynamically
    pLookupPrivilegeValueW = (LOOKUPPRIVILEGEVALUEW)GetFunctionAddress(
        GetLibraryAddress(ADVAPI32),
        LookupPrivilegeValueW_SW2_HASH
    );
    if (!pLookupPrivilegeValueW)
    {
#ifdef DEBUG
#ifdef BOF
        BeaconPrintf(CALLBACK_ERROR,
#else
        printf(
#endif
            "Address of 'LookupPrivilegeValueW' not found\n"
        );
#endif
        return FALSE;
    }

    ok = pLookupPrivilegeValueW(
        NULL,
        SeDebugPrivilege,
        &tkp.Privileges[0].Luid
    );
    if (!ok)
    {
#ifdef DEBUG
#ifdef BOF
        BeaconPrintf(CALLBACK_ERROR,
#else
        printf(
#endif
            "Failed to call LookupPrivilegeValueW, error: %ld\n",
            GetLastError()
        );
#endif
        return FALSE;
    }

    NTSTATUS status = NtOpenProcessToken(
        NtCurrentProcess(),
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        &hToken
    );
    if(!NT_SUCCESS(status))
    {
#ifdef DEBUG
#ifdef BOF
        BeaconPrintf(CALLBACK_ERROR,
#else
        printf(
#endif
            "Failed to call NtOpenProcessToken, status: 0x%lx\n",
            status
        );
#endif
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    status = NtAdjustPrivilegesToken(
        hToken,
        FALSE,
        &tkp,
        sizeof(TOKEN_PRIVILEGES),
        NULL,
        NULL
    );
    NtClose(hToken); hToken = NULL;
    if (!NT_SUCCESS(status))
    {
#ifdef DEBUG
#ifdef BOF
        BeaconPrintf(CALLBACK_ERROR,
#else
        printf(
#endif
            "Failed to call NtAdjustPrivilegesToken, status: 0x%lx\n",
            status
        );
#endif
        return FALSE;
    }

    return TRUE;
}
