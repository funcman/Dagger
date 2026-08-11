#ifndef DAGGER_TYPES_H
#define DAGGER_TYPES_H

#ifdef _WIN32
#    ifdef DBase_EXPORTS
#        define DAGGER_API __declspec(dllexport)
#    else
#        define DAGGER_API __declspec(dllimport)
#    endif
#else
#    define DAGGER_API
#endif

typedef unsigned char       BYTE;
typedef unsigned short int  WORD;
#ifdef _WIN32
// Match the Windows SDK (minwindef.h) exactly; unsigned int would clash with it.
typedef unsigned long       DWORD;
#else
typedef unsigned int        DWORD;
#endif
typedef unsigned int        UINT;

#endif//DAGGER_TYPES_H
