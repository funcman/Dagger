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
typedef unsigned int        DWORD;

#endif//DAGGER_TYPES_H
