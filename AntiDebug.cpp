#include "Security.h"
#include <tchar.h>






bool Security::AntiDebug::IsDebugged() {
	if (IsDebuggerPresent()) {
		return true;
	}

   

    HANDLE hProcess = INVALID_HANDLE_VALUE;
    BOOL found = FALSE;

    hProcess = GetCurrentProcess();
    CheckRemoteDebuggerPresent(hProcess, &found);

    if (found)
    {
        return true;
    }


    __try {
        DebugBreak();
        return true;
    }
    __except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

    }
	return false;
}