#include <windows.h>
#include "tcg.h"
#include "cpltest.h"

WINBASEAPI LPVOID WINAPI KERNEL32$VirtualAlloc (LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
WINBASEAPI WINBOOL WINAPI KERNEL32$VirtualFree (LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);

TESTFUNCS initTests() {
	TESTFUNCS tests = { 0 };
	tests.functions = KERNEL32$VirtualAlloc(NULL, sizeof(TESTFUNC) * MAXTESTS, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	tests.size = 0;
	tests.capacity = MAXTESTS;
	return tests;
}

void addTest(TESTFUNCS *tests, TESTFUNC newTest) {
	if (tests->size < tests->capacity) {
		tests->functions[tests->size] = newTest;
		tests->size++;
	} else {
		dprintf("Failed to add test because the max capacity of %d has been reached!", tests->capacity);
	}
}

void runTests(TESTFUNCS *tests) {
	for (int i = 0; i < tests->size; i++) {
		TESTFUNC test = tests->functions[i];
		dprintf("Running test %i of %i", i+1, tests->size);
		test();
	}
}

void freeTests(TESTFUNCS *tests) {
	KERNEL32$VirtualFree(tests->functions, 0, MEM_RELEASE);
}
