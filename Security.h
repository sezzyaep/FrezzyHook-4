#pragma once
#include <Windows.h>
#include <iostream>


#include "Tlhelp32.h"
#include <wchar.h>

namespace Security {
	namespace AntiDebug {
		bool IsDebugged();
	}
}
