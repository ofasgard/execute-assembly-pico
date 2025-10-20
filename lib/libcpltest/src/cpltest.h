#define ASSERT(expr, message) ((expr) ? (void) (0) : dprintf("Assertion failed: %s (%s)", #expr, #message))

typedef void (*TESTFUNC)();
void runTests(TESTFUNC tests[], int testCount);
