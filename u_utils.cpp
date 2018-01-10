#include "stdafx.h"

#include "u_utils.h"
#include <memory>
#include <algorithm>
#include <sstream>
#include <iomanip>

/*static*/ string_t guid::get_text( 
	_in const GUID &guid, 
	_in const convert_params *p_convert_params /*= nullptr*/
) {
	static const convert_params convert_params__default = { case_type::upper, true };
	if ( !p_convert_params )
		p_convert_params = &convert_params__default;
	
	string_t result;
	const unsigned result_capacity = 4 + (sizeof(guid) << 1);

	if ( p_convert_params->is_append_brakets ) {
		result.reserve( 2 + result_capacity );
		result.push_back( L'{' );
	} else
		result.reserve( result_capacity );

	std::ios_base& (__cdecl *case_type)(std::ios_base &) = p_convert_params->case_type == case_type::upper ? std::uppercase : std::nouppercase;

	{
		std::wostringstream sout;
		sout << std::hex << std::setw( sizeof(guid.Data1) << 1 ) << std::setfill( L'0' ) << case_type << guid.Data1;
		result += sout.str();
	}
	result.push_back( L'-' );

	{
		std::wostringstream sout;
		sout << std::hex << std::setw( sizeof(guid.Data2) << 1 ) << std::setfill( L'0' ) << case_type << guid.Data2;
		result += sout.str();
	}
	result.push_back( L'-' );

	{
		std::wostringstream sout;
		sout << std::hex << std::setw( sizeof(guid.Data3) << 1 ) << std::setfill( L'0' ) << case_type << guid.Data3;
		result += sout.str();
	}
	result.push_back( L'-' );
	
	{
		std::wostringstream sout;
		for ( unsigned i = 0; i < 2; ++i )
			sout << std::hex << std::setw( sizeof(guid.Data4[i]) << 1 ) << std::setfill( L'0' ) << case_type << guid.Data4[i];
		result += sout.str();
	}
	result.push_back( L'-' );

	{
		std::wostringstream sout;
		for ( unsigned i = 2; i < sizeof(guid.Data4); ++i )
			sout << std::hex << std::setw( sizeof(guid.Data4[i]) << 1 ) << std::setfill( L'0' ) << case_type << guid.Data4[i];
		result += sout.str();
	}

	if ( p_convert_params->is_append_brakets )
		result.push_back( L'}' );

	return result;
}