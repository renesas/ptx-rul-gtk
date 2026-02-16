
#include <stddef.h>
#include <stdbool.h>

#if !defined(_WIN32)
#define WINAPI

#define PVOID void*
#define LPVOID PVOID

#define BYTE uint8_t

#define USHORT uint16_t
#define PUSHORT USHORT*

#define ULONG unsigned long
#define PULONG ULONG*

#define WORD uint16_t
#define LPWORD WORD*

#define LONG long
#define LPLONG LONG*

#define DWORD uint32_t
#define LPDWORD DWORD*

#define CHAR char
#define PCHAR CHAR*

#define UCHAR unsigned char
#define PUCHAR UCHAR*

#define LPCTSTR char*
#define LPOVERLAPPED LPVOID
#define LPSECURITY_ATTRIBUTES LPVOID

#define BOOL bool

#define HANDLE void*

#endif // _WIN32
