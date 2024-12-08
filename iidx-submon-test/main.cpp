#include <iostream>
#include <windows.h>
#include <thread>

int main(int argc, char* argv[]) {
	HMODULE hmod = LoadLibrary(L"iidx-submon.dll");
	int ret = 0;

	if (hmod) {
		FARPROC proc = GetProcAddress(hmod, "iidx_submon_init");
		if (proc) {
			ret = ((int(*)(void))proc)();
			printf("iidx_submon_init() returns %d\n", ret);
		}
	}

	// Ctrl+C will crash the thread, use cin below
	if (ret > 0) {
		bool _b = true;
		while (_b) { 
			Sleep(1000);

			if (std::cin.get()) {
				if (hmod) {
					FARPROC proc = GetProcAddress(hmod, "iidx_submon_finl");
					if (proc) {
						((void(*)(void))proc)();
						printf("iidx_submon_finl()\n");
					}
				}
				_b = false;
			}
		}
	}

	return 1;
}