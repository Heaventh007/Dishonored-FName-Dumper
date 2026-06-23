// Hello.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#define CREATE_BUFFER(x, y) CHAR x[y]; SecureZeroMemory(x, y);

#ifdef __cplusplus
extern "C" {
#endif
	NTSYSAPI
		EXPORTNUM(334)
		int
		NTAPI
		RtlVsprintf(
			IN		CHAR* Buffer,
			IN		const CHAR* Format,
			IN		va_list va
		);
}


char* va_cb(char* Buffer, char* Format, ...) {
	va_list ArgumentList;
	va_start(ArgumentList, Format);
	RtlVsprintf(Buffer, Format, ArgumentList);
	va_end(ArgumentList);
	return Buffer;
}

void DumpName2(const char* CleanedName, QWORD FString, bool one) {
	//char* DumpTemp = "game:\\FNames1.txt";
	CREATE_BUFFER(DumpTemp, 1024);

	srand((FString >> 24) ^ GetTickCount());

	va_cb(DumpTemp, "game:\\FNames1_%i.txt", rand() % 32);

	

	// Open a file in append mode
	FILE *fptr = fopen(DumpTemp, "a");
	if (fptr) {
		CREATE_BUFFER(DumpTemp2, 2048);
		va_cb(DumpTemp2, "%s (%I64X)\n", CleanedName, FString);

		fprintf(fptr, DumpTemp2);

		// Close the file
		fclose(fptr);
	}
}

void DumpName(const wchar_t* CleanedName, QWORD FString, bool one) {
	//char* DumpTemp = one ? "game:\\FNames1.txt" : "game:\\FNames0.txt";

	CREATE_BUFFER(DumpTemp, 1024);

	srand((FString >> 24) ^ GetTickCount());

	va_cb(DumpTemp, "game:\\FNames0_%i.txt", rand() % 32);


	// Open a file in append mode
	FILE *fptr = fopen(DumpTemp, "a");
	if (fptr) {
		CREATE_BUFFER(DumpTemp2, 2048);
		va_cb(DumpTemp2, "%ws (%I64X)\n", CleanedName, FString);

		fprintf(fptr, DumpTemp2);

		// Close the file
		fclose(fptr);
	}
}

Detour<void> FName0_Detour;
void FName0_Hook(QWORD* fName, wchar_t const * Name, int iEFindName, unsigned int idk) {
	FName0_Detour.CallOriginal(fName, Name, iEFindName, idk);

	if (fName)
		DumpName(Name, *fName, false);
}

Detour<void> FName1_Detour;
void FName1_Hook(QWORD* fName, char const * Name, int iEFindName, unsigned int idk) {
	FName1_Detour.CallOriginal(fName, Name, iEFindName, idk);

	if (fName)
		DumpName2(Name, *fName, true);
}

bool Unload = false;
void ThreadInit() {
	while (!Unload) {

		if (MmIsAddressValid((void*)0x8207AB59) && *XexExecutableModuleHandle) {
			//if ((*XexExecutableModuleHandle)->TimeDateStamp)

			if (*(DWORD*)0x8207AB59 == 'nonc' && *(DWORD*)0x830569D0 == 0x917F02C8) {
				FName0_Detour.SetupDetour(0x822E5030, FName0_Hook);
				FName1_Detour.SetupDetour(0x822E52A0, FName1_Hook);

				DbgPrint("Detours setup.\n");
				break;
			}
		}

	}

	//FName0_Detour.SetupDetour(0x822E5030, FName0_Hook);
	//FName1_Detour.SetupDetour(0x822E52A0, FName1_Hook);
}

HANDLE CreateSystemThread(void * pThreadFunc, void* pParameter, unsigned long dwStackSize) {
	HANDLE hThreadHandle = nullptr;

	if (ExCreateThread(&hThreadHandle, dwStackSize, nullptr, nullptr, LPTHREAD_START_ROUTINE(pThreadFunc), pParameter, 0x4000026) >= ERROR_SUCCESS) {
		return hThreadHandle;
	}

	return nullptr;
}

VOID OnAttachProcess() {
	CreateSystemThread(ThreadInit, 0, 0);
}

VOID OnDetachProcess() {
	Unload = true;

	FName0_Detour.TakeDownDetour();
	FName1_Detour.TakeDownDetour();

	Sleep(1000);
}

// This is the main entry point to your xex file.
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) OnAttachProcess(); // Called when your plugin loads.
	if (dwReason == DLL_PROCESS_DETACH) OnDetachProcess(); // Called when your plugin unloads.

	return TRUE; // Returns true, meaning the plugin successfully loaded, or unloaded.
}