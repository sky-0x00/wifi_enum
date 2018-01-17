#include "stdafx.h"

#include "u_utils.h"
#include <memory>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cassert>

/*static*/ string_t guid::get_text( 
	_in const guid_t &guid, 
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

string_t string::convert( 
	_in const char *str,
	_in unsigned length /*= length_unknown*/
) {
	assert( str );	
	
	auto get_char = []( _in unsigned char ch ) -> char_t
	{
		if ( ch < 0x80 )					// ascii-
			return ch;

		if ( ch >= 0xC0 )					// 'À'..'ß', 'à'...'ÿ'
			return ch + 0x350;

		switch (ch)
		{
			case 0xA8:						// '¨'
				return ch + 0x359;
			case 0xB8:						// '¸'
				return ch + 0x399;
		}

		trace.out( trace::category::error, L"string::convert( ch: '0x%02X' ): not_supported", ch );
		throw std::exception( "" );
	};

	string_t result;

	if ( length_unknown != length )
	{
		result.reserve( length );
		for ( ; *str; ++str )
		{
			if ( 0 == length-- )
				break;
			const auto ch = get_char( *str );
			result.push_back( ch );
		}
	} else 
	{
		for ( ; *str; ++str )
		{
			const auto ch = get_char( *str );
			result.push_back( ch );			
		}
	}
	
	return result;
}

void ssid::set_name( 
	_in cstr_t name, 
	_out DOT11_SSID &ssid 
) {
	if ( !name )
	{
		ssid.uSSIDLength = 0;
		return;
	}

	for ( ssid.uSSIDLength = 0; name[ ssid.uSSIDLength ]; )
	{
		ssid.ucSSID[ ssid.uSSIDLength ] = static_cast< UCHAR >( name[ ssid.uSSIDLength ] );
		if ( ++ssid.uSSIDLength == DOT11_SSID_MAX_LENGTH )
		{
			trace.out( trace::category::warning, L"ssid name \"%s\" is trancated to %u chars", name, ssid.uSSIDLength );
			break;
		}
	}
}

address::mac::mac(
) {
	std::fill( std::begin(data), std::end(data), 0 );
}
address::mac::mac(
	_in const DOT11_MAC_ADDRESS &address
) {
	assign( address );
}
void address::mac::assign(
	_in const DOT11_MAC_ADDRESS &address
) {
	std::copy( std::cbegin(address), std::cend(address), std::begin(data) );
}

/*static*/ string_t address::mac::to_string( 
	_in const DOT11_MAC_ADDRESS &address,
	_in const convert_params *p_convert_params /*= nullptr*/
) {
	static const convert_params convert_params__default = { case_type::upper };
	if ( !p_convert_params )
		p_convert_params = &convert_params__default;

	char_t format[] = L":%02X";
	if ( case_type::upper != p_convert_params->case_type )
		format[ _countof(format) - 1 ] = L'x';

	char_t buffer[ 3 * sizeof(address) ], *p_buffer = buffer;

	auto offset = swprintf_s( buffer, format + 1, address[0] );
	assert( 2 == offset );

	for ( unsigned i = 0; ++i < sizeof(address); )
	{
		p_buffer += offset;
		offset = swprintf_s( p_buffer, _countof(buffer) - (p_buffer - buffer), format, address[i] );
		assert( 3 == offset );
	}

	return buffer;
}
string_t address::mac::to_string( 
	_in const convert_params *p_convert_params /*= nullptr*/
) const
{
	return to_string( data, p_convert_params );
}