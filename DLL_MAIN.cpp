//#include "protect/protect.hpp"
#include "C_Check.h"
#include "DLL_MAIN.h"
#include "Hooks.h"
#include "netvar_manager.h"
#include "License.h"
#include "render.h"
#include <thread>
#include <chrono>
#include "scripting/java func/js_functions.hpp"
#pragma comment( lib, "Winhttp.lib" )
#pragma comment( lib, "ws2_32.lib" )
#include "Hwid.h"
#include "Security.h"

__forceinline void discord_main();

typedef void(*CallRun_t)(UINT_PTR a, UINT_PTR b, UINT_PTR c);
CallRun_t CallRun;

CNetVarManager netvars;

void InitializeNetvars() {
	netvars.Initialize();
}

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;


bool is_dir(const TCHAR* dir) {
	DWORD flag = GetFileAttributes(dir);
	if (flag == 0xFFFFFFFFUL) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			return false;
	}
	if (!(flag & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;
}

void Show() {
	_old_out = GetStdHandle(STD_OUTPUT_HANDLE);
	_old_err = GetStdHandle(STD_ERROR_HANDLE);
	_old_in = GetStdHandle(STD_INPUT_HANDLE);
	::AllocConsole() && ::AttachConsole(GetCurrentProcessId());
	_out = GetStdHandle(STD_OUTPUT_HANDLE);
	_err = GetStdHandle(STD_ERROR_HANDLE);
	_in = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(_out,
		ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
	SetConsoleMode(_in,
		ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
		ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
}
void Hide() {
	if (_out && _err && _in) {
		FreeConsole();
		if (_old_out)
			SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
		if (_old_err)
			SetStdHandle(STD_ERROR_HANDLE, _old_err);
		if (_old_in)
			SetStdHandle(STD_INPUT_HANDLE, _old_in);
	}
}
bool Write(const char* fmt, ...) {
	if (!_out)
		return false;
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	_vsnprintf_s(buf, 1024, fmt, va);
	va_end(va);
	return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
}

class CLoader {
public:
	CLoader() { this->m_bInitialized = false; }
	bool m_bInitialized;
	std::string szUserName;
	std::string szDaysLeft;
	int m_iDaysLeft;
	DWORD m_iProcessID;
};

#include "discord.h"
Discord* g_Discord;

__forceinline void discord_main()
{
	g_Discord->Initialize();
	g_Discord->Update();
}

typedef CLoader* (__cdecl* CLoaderGetHandle)();

bool WriteLine(std::string text) {
	time_t systime = time(0);
	tm* ctime = localtime(&systime);
	string c = ctime->tm_sec > 9 ? to_string(ctime->tm_sec) : "0" + to_string(ctime->tm_sec);
	string m = ctime->tm_min > 9 ? to_string(ctime->tm_min) : "0" + to_string(ctime->tm_min);
	string h = ctime->tm_hour > 9 ? to_string(ctime->tm_hour) : "0" + to_string(ctime->tm_hour);
	string stime = h + ":" + m + ":" + c;
	return Write(("[ FrezzyHook | " + stime + " ]: " + text + "\n").c_str());
}
#define Debug false
#define NOAUTH true
#define _SHOWLOG

DWORD FindProcessId(const std::string& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

unsigned long __stdcall CheatMain(void* base)
{
	discord_main();
#ifdef _SHOWLOG 
	Show();
	WriteLine("Loading FrezzyHook Debug");
#endif
	//static bool Debug = true;
	static bool IPS = false;
	if (!NOAUTH) {
		while (!IPS) {
			IPS = C_Check::main();
			this_thread::sleep_for(2s);
		}
	}
#ifdef _SHOWLOG 
	if (NOAUTH)
		WriteLine("Authentication Disabled");
	else
		WriteLine("Successfully Connected with Loader and Driver");
#endif
	if (!NOAUTH) {
		HWID::AuthenticateInjection();
	}
#ifdef _SHOWLOG 
	if (!NOAUTH)
		WriteLine("Successfully Authenticated Injection");

#endif

#ifdef _SHOWLOG 
	WriteLine("Localizing with Loader");
#endif

#ifdef _SHOWLOG 
	WriteLine("Getting Module Handles");
#endif
	while (!(csgo->Init.Window = FindWindowA(hs::Valve001::s().c_str(), NULL)))
		this_thread::sleep_for(200ms);
	while (!GetModuleHandleA(hs::serverbrowser_dll::s().c_str()))
		this_thread::sleep_for(200ms);
	while (!GetModuleHandleA(hs::client_dll::s().c_str()))
		this_thread::sleep_for(200ms);
	while (!GetModuleHandleA(hs::engine_dll::s().c_str()))
		this_thread::sleep_for(200ms);
	//
#ifdef _SHOWLOG 
	WriteLine("Initializing Interfaces");
#endif
	I::Setup();
#ifdef _SHOWLOG 
	WriteLine("Initializing Netvars");
#endif
	InitializeNetvars();
#ifdef _SHOWLOG 
	WriteLine("Initializing Hooks");
#endif
	H::Hook();

#ifdef _SHOWLOG 


	WriteLine("Loaded FrezzyHook Debug");
	WriteLine("Fixed by FrezzyTerror");
	WriteLine(".............. lol");
	while (true) {
		csgo->ConnectedToInternet = HWID::ConnectedToInternet();
		this_thread::sleep_for(17s);
	}

#else

	if (Debug) {
		while (true) {
			csgo->ConnectedToInternet = HWID::ConnectedToInternet();
			this_thread::sleep_for(15s);
		}
		/*
		while (true) {
			this_thread::sleep_for(1s);

		}
		*/

	}
	else
	{

		while (true) {
			csgo->ConnectedToInternet = HWID::ConnectedToInternet();
			this_thread::sleep_for(17s);
		}

		Show();
		this_thread::sleep_for(200ms);
		auto sysc = std::chrono::system_clock::now();


		time_t systime = time(0);
		tm* ctime = localtime(&systime);
		string c = ctime->tm_sec > 9 ? to_string(ctime->tm_sec) : "0" + to_string(ctime->tm_sec);
		string m = ctime->tm_min > 9 ? to_string(ctime->tm_min) : "0" + to_string(ctime->tm_min);
		string h = ctime->tm_hour > 9 ? to_string(ctime->tm_hour) : "0" + to_string(ctime->tm_hour);
		string stime = h + ":" + m + ":" + c;
		WriteLine(("[" + stime + "] - Detected Reverse Engineering").c_str());
		this_thread::sleep_for(4s);

		H::UnHook();

		SetWindowLongPtr(csgo->Init.Window, GWL_WNDPROC, (LONG_PTR)csgo->Init.OldWindow);
		FreeLibraryAndExitThread(csgo->Init.Dll, 0);
	}
#endif
	/*
	if (csgo->DoUnload)
		H::UnHook();

	interfaces.engine->ClientCmd_Unrestricted(hs::clear::s().c_str(), 0);
	interfaces.engine->ClientCmd_Unrestricted(hs::unload_message::s().c_str(), 0);
	SetWindowLongPtr(csgo->Init.Window, GWL_WNDPROC, (LONG_PTR)csgo->Init.OldWindow);
	//FreeLibraryAndExitThread(csgo->Init.Dll, 0);
	*/
	
	return 0;
}




bool __stdcall DllMain(void* base, uint32_t reason_to_call, void* reserved) {
	if (reason_to_call == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(CheatMain), reserved, 0, nullptr);
	return TRUE;
}