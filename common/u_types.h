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

typedef void *handle_t, pvoid_t;
typedef const void *cpvoid_t;

namespace data {

	struct untyped;

template < typename type_t > 
	struct typed
	{
		type_t value;

		typed(
			_in type_t value
		) :
			value( value )
		{}

		untyped* get_untyped( 
			_out untyped &value 
		) {
			value.pdata = &value;
			value.size = sizeof(value);
		}
	};

	struct untyped
	{
		cpvoid_t pdata;
		unsigned size;

		untyped(
		) :
			pdata( nullptr ),
			size( 0 )
		{}
		template < typename type_t > untyped(
			_in const type_t &data
		) :
			pdata( &data ),
			size( sizeof(data) )
		{}
		template < typename type_t > untyped( 
			_in const typed< type_t > &typed 
		) :
			pdata( &typed.value ),
			size( sizeof(typed.value) )
		{}
	};
}