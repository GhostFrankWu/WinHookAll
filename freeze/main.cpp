#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <thread>

typedef LONG(NTAPI* _NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI* _NtResumeProcess)(IN HANDLE ProcessHandle);
HHOOK g_Hook = NULL;
HHOOK g_MouseHook = NULL;
HANDLE ProcessHandle = 0;
ULONGLONG press_a = 0;
ULONGLONG press_b = 200;
HMODULE ntdll = 0;

void EnableDebugPriv(){
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkp;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);
	CloseHandle(hToken);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam){
	if (nCode == 0) return 1;
	return CallNextHookEx(g_MouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN){
		LPKBDLLHOOKSTRUCT pKbs = (LPKBDLLHOOKSTRUCT)lParam;
		// https://www.millisecond.com/support/docs/v6/html/language/scancodes.htm
		if (pKbs->scanCode == 0x2a) { //LShift
			press_a = GetTickCount64();
		}
		else if (pKbs->scanCode == 0x36) { //RShift
			press_b = GetTickCount64();
		}
		if (max(press_a, press_b) - min(press_a, press_b) < 100) {
			((_NtResumeProcess)GetProcAddress(ntdll, "NtResumeProcess"))(ProcessHandle);
			exit(0);
		}
	}

	if (nCode >= 0)  return 1; 
	else return CallNextHookEx(g_Hook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD FindProcessId(const std::wstring& processName){
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);
	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)return 0;
	Process32First(processesSnapshot, &processInfo);
	while (Process32Next(processesSnapshot, &processInfo)){
		if (!processName.compare(processInfo.szExeFile)){
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	CloseHandle(processesSnapshot);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	TCHAR szWindowClass[] = L" ";
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX),CS_HREDRAW | CS_VREDRAW,WndProc, 0,0,hInstance,
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION)), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1), NULL,szWindowClass,LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION)) };
	if (!RegisterClassEx(&wcex))  return 1;

	EnableDebugPriv();
	g_Hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
	g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
	ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, FindProcessId(L"winlogon.exe"));
	ntdll = GetModuleHandle(L"ntdll");
	if (ntdll) {
		MSG msg;
		((_NtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess"))(ProcessHandle);
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return (int)msg.wParam;
	}
}