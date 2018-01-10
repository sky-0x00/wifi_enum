// wifi_enum.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include "u_wifi_utils.h"
#include "u_utils.h"

struct args {
	enum class action {
		none
	};

	static action parse( _in unsigned argc, _in cstr_t argv[] );
};

args::action args::parse( 
	_in unsigned argc, 
	_in cstr_t argv[] 
) {
	return action::none;
}

int wmain(
	_in unsigned argc,
	_in cstr_t argv[]
) {
	//trace.out( trace::type::normal, L"begin" );

	const auto action = args::parse( argc, argv );
	
	wlan::manager wifi;
	std::vector< wlan::iface > ifaces;
	wifi.enum_ifaces( ifaces );

	wprintf_s( L"WiFi interface(s): %u\n", ifaces.size() );
	unsigned i = 0;
	for ( const auto &iface : ifaces )
		wprintf_s( L"%u:\n  name: \"%s\"\n  guid: %s\n  state: %s\n\n", ++i, iface.description.c_str(), guid::get_text( iface.guid ).c_str(), iface.get_state_text() );
	
    return 0;
}

