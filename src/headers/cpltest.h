#define ASSERT(expr, message) ((expr) ? (void) (0) : dprintf("Assertion failed: %s (%s)", #expr, #message))
#define MAXTESTS 256

typedef void (*TESTFUNC)();

typedef struct {
	TESTFUNC *functions;
	size_t size;
	size_t capacity;
} TESTFUNCS;

TESTFUNCS initTests();
void addTest(TESTFUNCS *tests, TESTFUNC newTest);
void runTests(TESTFUNCS *tests);
void freeTests(TESTFUNCS *tests);
