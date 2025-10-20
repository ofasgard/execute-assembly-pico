#include <windows.h>
#include "tcg.h"
#include "cpltest.h"

void runTests(TESTFUNC tests[], int testCount) {
	for (int i = 0; i < testCount; i++) {
		TESTFUNC test = tests[i];
		dprintf("Running test %i of %i", i+1, testCount);
		test();
	}
}
