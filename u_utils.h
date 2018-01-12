#pragma once

#include "u_types.h"
//#include <guiddef.h>
#include <windows.h>
#include <wlanapi.h>

namespace stdex 
{
	template < typename type_t >
	bool in_range( 
		_in type_t value, 
		_in const std::pair< type_t, type_t > &range 
	) {
		return (value >= range.first) && (value < range.second);
	}
}

struct guid {
	struct convert_params {
		case_type case_type;
		bool is_append_brakets;
	};
	static string_t get_text( _in const guid_t &guid, _in const convert_params *p_convert_params = nullptr );
};

namespace string 
{
	const unsigned length_unknown = UINT_MAX;

	string_t convert( _in const char *str, _in unsigned length = length_unknown );
}

namespace ssid
{
	void set_name( _in cstr_t name, _out DOT11_SSID &ssid );
}

namespace address
{
	struct mac
	{
		struct convert_params {
			case_type case_type;
		};

		static string_t to_string( _in const DOT11_MAC_ADDRESS &address, _in const convert_params *p_convert_params = nullptr );
		
		mac();
		mac( _in const DOT11_MAC_ADDRESS &address );
		void assign( _in const DOT11_MAC_ADDRESS &address );
		string_t to_string( _in const convert_params *p_convert_params = nullptr );

		DOT11_MAC_ADDRESS data;
	};
}