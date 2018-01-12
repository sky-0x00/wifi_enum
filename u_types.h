#pragma once

#define _in
#define _out
#define _optional
#define char_null L'\0'

typedef unsigned char byte_t;
typedef unsigned short ushort_t;

typedef wchar_t char_t;
typedef char_t *str_t;
typedef const char_t *cstr_t;

enum class case_type {
	lower,
	upper
};

#include <string>
typedef std::wstring string_t;

#include <guiddef.h>
typedef GUID guid_t;

typedef void *handle_t;