#pragma once


#if SP_EXPORT
#define SP_DLL_EXPORT __declspec(dllexport)
#else
#define SP_DLL_EXPORT __declspec(dllimport)
#endif