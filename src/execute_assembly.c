#include <windows.h>
#include <oleauto.h>
#include "headers/clr.h"
#include "headers/guid.h"
#include "headers/tcg.h"
#include "headers/cpltest.h"
  
WINBASEAPI HRESULT MSCOREE$CLRCreateInstance(REFCLSID clsid, REFIID riid, LPVOID *ppInterface);
WINBASEAPI SAFEARRAY * OLEAUT32$SafeArrayCreate(VARTYPE vt, UINT cDims, SAFEARRAYBOUND *rgsabound);
WINBASEAPI HRESULT OLEAUT32$SafeArrayDestroy(SAFEARRAY *psa);
WINBASEAPI HRESULT OLEAUT32$SafeArrayGetLBound(SAFEARRAY *psa, UINT nDim, LONG *plLbound);
WINBASEAPI HRESULT OLEAUT32$SafeArrayGetUBound(SAFEARRAY *psa, UINT nDim, LONG *plLbound);
WINBASEAPI SAFEARRAY * OLEAUT32$SafeArrayCreateVector(VARTYPE vt, LONG lLbound, ULONG cElements);
WINBASEAPI HRESULT OLEAUT32$SafeArrayPutElement(SAFEARRAY *psa, LONG *rgIndices, void *pv);
WINBASEAPI BSTR OLEAUT32$SysAllocString(const OLECHAR *psz);

typedef struct {
	ICLRMetaHost* metaHost;
	ICLRRuntimeInfo* runtimeInfo;
	ICorRuntimeHost* runtimeHost;
	AppDomain* appDomain;
	Assembly* loadedAssembly;
	MethodInfo* entryPoint;
} AssemblyData;

HRESULT get_clr(AssemblyData* assemblyData) {
	// Uses CLRCreateInstance() to retrieve an instance of the .NET Common Language Runtime (CLR).
	
	return MSCOREE$CLRCreateInstance(&CLSID_CLRMetaHost, &IID_ICLRMetaHost, (LPVOID*)&assemblyData->metaHost);
}

HRESULT get_runtime(AssemblyData* assemblyData) {
	// Retrieves the runtime interface for .NET version v4.0.30319.
	
	return assemblyData->metaHost->lpVtbl->GetRuntime(assemblyData->metaHost, L"v4.0.30319", &IID_ICLRRuntimeInfo, (LPVOID*)&assemblyData->runtimeInfo);
}

HRESULT load_runtime(AssemblyData* assemblyData) {
	// Loads and initialises the hosted CLR in the current process.
	
	BOOL isLoadable;
	assemblyData->runtimeInfo->lpVtbl->IsLoadable(assemblyData->runtimeInfo, &isLoadable);
	if (!isLoadable) { return FALSE; }
	
	HRESULT result = assemblyData->runtimeInfo->lpVtbl->GetInterface(assemblyData->runtimeInfo, &CLSID_CorRuntimeHost, &IID_ICorRuntimeHost, (LPVOID*)&assemblyData->runtimeHost);
	if (result != S_OK) { return result; }
	
	return assemblyData->runtimeHost->lpVtbl->Start(assemblyData->runtimeHost);
}

HRESULT get_default_appdomain(AssemblyData* assemblyData) {
	// Gets the default AppDomain, which we will use to load and execute our assembly.
	
	IUnknown* appDomainUnknown = NULL;
	HRESULT result = assemblyData->runtimeHost->lpVtbl->GetDefaultDomain(assemblyData->runtimeHost, &appDomainUnknown);
	if (result != S_OK) { return result; }
	
	// We need to use QueryInterface() to retrieve the AppDomain pointer from the COM object that is returned by GetDefaultDomain().
	return appDomainUnknown->lpVtbl->QueryInterface(appDomainUnknown, &IID_AppDomain, (LPVOID*)&assemblyData->appDomain);
	
}

HRESULT load_assembly(char *raw_assembly, size_t assembly_len, AssemblyData* assemblyData) {
	// Load our assembly into memory.
	
	// In order to do so, we must first copy it into a SafeArray.
	SAFEARRAYBOUND sab;
	sab.lLbound = 0;
	sab.cElements = assembly_len;
	SAFEARRAY *assembly_sa = OLEAUT32$SafeArrayCreate(0x11, 1, &sab);
	
	DWORD i;
	PBYTE p;
	for(i = 0, p = assembly_sa->pvData; i < assembly_len; i++) {
		p[i] = raw_assembly[i];
	}
	
	// Then we can load it with the AppDomain::Load(byte[]) method.
	HRESULT result = assemblyData->appDomain->lpVtbl->Load_3(assemblyData->appDomain, assembly_sa, &assemblyData->loadedAssembly);
	
	// Once the assembly is loaded, we don't need the SafeArray anymore.
	OLEAUT32$SafeArrayDestroy(assembly_sa);
	return result;
}

HRESULT get_entrypoint(AssemblyData* assemblyData) {
	// Get the entrypoint from our loaded assembly. 
	
	return assemblyData->loadedAssembly->lpVtbl->EntryPoint(assemblyData->loadedAssembly, &assemblyData->entryPoint);
}

LONG get_parameters(AssemblyData* assemblyData) {
	// Get the required number of parameters from our loaded assembly's entrypoint function.
	
	SAFEARRAY* entrypoint_params;
	LONG lowerBound, upperBound;
	
	assemblyData->entryPoint->lpVtbl->GetParameters(assemblyData->entryPoint, &entrypoint_params);
	OLEAUT32$SafeArrayGetLBound(entrypoint_params, 1, &lowerBound);
	OLEAUT32$SafeArrayGetUBound(entrypoint_params, 1, &upperBound);
	return upperBound - lowerBound + 1;
}

SAFEARRAY *prepare_parameters(WCHAR *argv[], int argc, LONG paramCount) {
	// Load the parameters provided to the PICO into a SafeArray, preparing them to be passed to the entrypoint.
	
	SAFEARRAY *params;
	if (paramCount == 0) {
		// If no parameters are required, we simply can provide an empty array.
		SAFEARRAYBOUND params_sab;
		params_sab.lLbound = 0;
		params_sab.cElements = 0;
		params = OLEAUT32$SafeArrayCreate(0x11, 1, &params_sab);
	} else {
		// If parameters are required, must start building the SafeArray object.
		params = OLEAUT32$SafeArrayCreateVector(VT_VARIANT, 0, 1);
		
		// Create a variant to hold a 1-dimensional array of strings.
		VARIANT v_params;
		v_params.vt = (VT_ARRAY | VT_BSTR);
		v_params.parray = OLEAUT32$SafeArrayCreateVector(VT_BSTR, 0, argc);
		
		// Iterate through argv and add each argument to the variant.
		for (LONG i = 0; i < argc; i++) {
			OLEAUT32$SafeArrayPutElement(v_params.parray, &i, OLEAUT32$SysAllocString(argv[i]));
		}
		
		// If there were no user-supplied parameters, we must still add an empty string to keep the CLR happy.
		if (argc == 0) {
			LONG i = 0;
			OLEAUT32$SafeArrayPutElement(v_params.parray, &i, OLEAUT32$SysAllocString(L""));
		}
		
		// Add the variant containing our arguments to the SafeArray.
		LONG i = 0;
		OLEAUT32$SafeArrayPutElement(params, &i, &v_params);
		
		// Then do some cleanup.
		OLEAUT32$SafeArrayDestroy(v_params.parray);
	}
	
	return params;
}

HRESULT execute_assembly(AssemblyData* assemblyData, SAFEARRAY *params, VARIANT *retval) {
	// Pass the prepared parameters to the loaded assembly's entrypoint.
	
	VARIANT obj = { 0 };
	return assemblyData->entryPoint->lpVtbl->Invoke_3(assemblyData->entryPoint, obj, params, retval);
}

void release_assembly(AssemblyData* assemblyData) {
	if (assemblyData->entryPoint != NULL) { assemblyData->entryPoint->lpVtbl->Release(assemblyData->entryPoint); }
	if (assemblyData->loadedAssembly != NULL) { assemblyData->loadedAssembly->lpVtbl->Release(assemblyData->loadedAssembly); }
	if (assemblyData->appDomain != NULL) { assemblyData->appDomain->lpVtbl->Release(assemblyData->appDomain); }
	if (assemblyData->runtimeHost != NULL) { assemblyData->runtimeHost->lpVtbl->Release(assemblyData->runtimeHost); }
	if (assemblyData->runtimeInfo != NULL) { assemblyData->runtimeInfo->lpVtbl->Release(assemblyData->runtimeInfo); }
	if (assemblyData->metaHost != NULL) { assemblyData->metaHost->lpVtbl->Release(assemblyData->metaHost); }
}

HRESULT go_pico(char *raw_assembly, size_t assembly_len, WCHAR *argv[], int argc) {
	HRESULT result;
	AssemblyData assemblyData = { 0 };

	result = get_clr(&assemblyData);
	if (result != S_OK) { release_assembly(&assemblyData); return result; }
	
	result = get_runtime(&assemblyData);
	if (result != S_OK) { release_assembly(&assemblyData); return result; }
	
	result = load_runtime(&assemblyData);
	if (result != S_OK) { release_assembly(&assemblyData); return result; }
	
	result = get_default_appdomain(&assemblyData);
	if (result != S_OK) { release_assembly(&assemblyData); return result; }
	
	result = load_assembly(raw_assembly, assembly_len, &assemblyData);
	if (result != S_OK) { release_assembly(&assemblyData); return result; }
	
	result = get_entrypoint(&assemblyData);
	if (result != S_OK) { release_assembly(&assemblyData); return result; }
	
	VARIANT retval;
	LONG paramCount = get_parameters(&assemblyData);
	SAFEARRAY *params = prepare_parameters(argv, argc, paramCount);
	
	result = execute_assembly(&assemblyData, params, &retval);
	
	OLEAUT32$SafeArrayDestroy(params);
	release_assembly(&assemblyData);
	return result;
}

#ifdef CPLTESTS
void test_get_clr() {
	HRESULT result;
	AssemblyData assemblyData = { 0 };
	result = get_clr(&assemblyData);
	ASSERT(result == S_OK, "test_get_clr: Could not retrieve the CLR.");
}

void test_get_runtime() {
	HRESULT result;
	AssemblyData assemblyData = { 0 };
	result = get_clr(&assemblyData);
	ASSERT(result == S_OK, "test_get_runtime: Could not retrieve the CLR.");
	result = get_runtime(&assemblyData);
	ASSERT(result == S_OK, "test_get_runtime: Could not retrieve the runtime interface.");
}

void test_load_runtime() {
	HRESULT result;
	AssemblyData assemblyData = { 0 };
	result = get_clr(&assemblyData);
	ASSERT(result == S_OK, "test_load_runtime: Could not retrieve the CLR.");
	result = get_runtime(&assemblyData);
	ASSERT(result == S_OK, "test_load_runtime: Could not retrieve the runtime interface.");
	result = load_runtime(&assemblyData);
	ASSERT(result == S_OK, "test_load_runtime: Could not load an initialise the hosted CLR.");
}

HRESULT go_tests(char *raw_assembly, size_t assembly_len, WCHAR *argv[], int argc) {
	TESTFUNC tests[] = {
		(TESTFUNC) test_get_clr,
		(TESTFUNC) test_get_runtime,
		(TESTFUNC) test_load_runtime
	};
	runTests(tests, 3);
	return S_OK;
}
#endif

HRESULT go(char *raw_assembly, size_t assembly_len, WCHAR *argv[], int argc) {
	#ifdef CPLTESTS
	return go_tests(raw_assembly, assembly_len, argv, argc);
	#else
	return go_pico(raw_assembly, assembly_len, argv, argc);
	#endif
}
