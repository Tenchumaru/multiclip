#pragma once

#include "targetver.h"
#ifdef _DEBUG
#	ifndef DEBUG
#		define DEBUG
#	endif
#elif defined(DEBUG)
#	error inconsistent DEBUG directives
#elif !defined(NDEBUG)
#	define NDEBUG
#endif
#ifndef STRICT
#	define STRICT
#endif

// Enable allocation tracking.
#include <cstdlib>
#define _CRT_MAP_ALLOC

// Platform SDK include directives
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <tchar.h> // CRT; must appear before strsafe.h
#include <strsafe.h>
#include <commctrl.h>

// CRT debugging
#include <crtdbg.h>
#ifdef _DEBUG
inline void* operator new(size_t size, char const* fileName, int line) {
	void* p = ::operator new(size, _NORMAL_BLOCK, fileName, line);
	if(p == NULL) {
		throw 0;
	}
	return p;
}
inline void* operator new[](size_t size, char const* fileName, int line) {
	void* p = ::operator new[](size, _NORMAL_BLOCK, fileName, line);
	if(p == NULL) {
		throw 0;
	}
	return p;
}
inline void operator delete(void* p, char const* fileName, int line) {
	::operator delete(p, _NORMAL_BLOCK, fileName, line);
}
inline void __cdecl operator delete[](void* p, char const* fileName, int line) {
	::operator delete[](p, _NORMAL_BLOCK, fileName, line);
}
#	define DEBUG_NEW new(__FILE__, __LINE__)
#	define new DEBUG_NEW
#	define _VERIFYE _ASSERTE
#else
#	define _VERIFYE(expr) ((void)(expr))
#endif
#define ASSERT _ASSERTE
#define VERIFY _VERIFYE
