// wifi_enum.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include "u_wifi_utils.h"
#include "u_utils.h"

#define BOOL_CHAR( is_true )	((is_true) ? L'Y' : L'N')

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
	unsigned i_iface = 0;

	for ( const auto &iface : ifaces )
	{
		wprintf_s( L"%u:\n  name: \"%s\"\n  guid: %s\n  state: %s\n", ++i_iface, iface.description.c_str(), guid::get_text( iface.guid ).c_str(), iface.get_state_text() );

		std::vector< wlan::network > networks;
		wifi.get_network_list( iface.guid, 0/*WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES | WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES*/, networks );

		wprintf_s( L"\n  networks(s): %u", networks.size() );
		unsigned i_network = 0;

		for ( const auto &network : networks )
		{
			PWLAN_BSS_LIST BssList = nullptr;
			wifi.get_network_bsslist( iface.guid, network, &BssList );

			wprintf_s( L"\n  %u:\n    ssid: %s", ++i_network, network.ssid.c_str() );
			if ( !network.profile_name.empty() )
				wprintf_s( L" (profile: %s)", network.profile_name.c_str() );

			{
				auto get__network_topology__name = []( _in wlan::network::bss::type network_topology ) -> cstr_t
				{
					switch ( network_topology )
					{
						case wlan::network::bss::type::infrastructure:
							return L"infrastructure";
						case wlan::network::bss::type::adhoc:
							return L"adhoc";
						default:
							trace.out( trace::category::error, L"network.topology: ? (%i)", static_cast< int >( network_topology ) );
							throw std::exception();
					}
				};			
				wprintf_s( L"\n    topology: %s", get__network_topology__name( network.topology ) );
			}

			wprintf_s( L"\n    is_connactable: " );
			if ( network.is_connectable() )
				wprintf_s( L"Y" );
			else
				wprintf_s( L"N (code: %i)", network.notconnactable_reasoncode );

			wprintf_s( L"\n    rx_level: %hu%% (%hi dbm)", network.signal_level__percentage, network.get__signal_level__rssi() );

			{
				auto get__algorithm_name__auth = []( _in DOT11_AUTH_ALGORITHM algorithm ) 
				{
					switch ( algorithm )
					{
						case DOT11_AUTH_ALGO_80211_OPEN:
							return L"80211_OPEN";
						case DOT11_AUTH_ALGO_80211_SHARED_KEY:
							return L"80211_SHARED_KEY";
						case DOT11_AUTH_ALGO_WPA:
							return L"WPA";
						case DOT11_AUTH_ALGO_WPA_PSK:
							return L"WPA_PSK";
						//case DOT11_AUTH_ALGO_WPA_NONE:
						//	return L"WPA_NONE";
						case DOT11_AUTH_ALGO_RSNA:
							return L"RSNA (WPA2)";
						case DOT11_AUTH_ALGO_RSNA_PSK:
							return L"RSNA_PSK (WPA2_PSK)";
						case DOT11_AUTH_ALGO_IHV_START:
							return L"IHV_START";
						case DOT11_AUTH_ALGO_IHV_END:
							return L"IHV_END";
						default:
							trace.out( trace::category::error, L"network.security.algorithm.auth: ? (%i)", static_cast< int >( algorithm ) );
							throw std::exception();
					}
				};
				auto get__algorithm_name__cipher = []( _in DOT11_CIPHER_ALGORITHM algorithm ) 
				{
					switch ( algorithm )
					{
						case DOT11_CIPHER_ALGO_NONE:
							return L"NONE";
						case DOT11_CIPHER_ALGO_WEP40:
							return L"WEP40";
						case DOT11_CIPHER_ALGO_TKIP:
							return L"TKIP";
						case DOT11_CIPHER_ALGO_CCMP:
							return L"CCMP";
						case DOT11_CIPHER_ALGO_WEP104:
							return L"WEP104";
						case DOT11_CIPHER_ALGO_WPA_USE_GROUP:		// также DOT11_CIPHER_ALGO_RSN_USE_GROUP
							return L"WPA/RSN USE_GROUP";
						case DOT11_CIPHER_ALGO_IHV_START:
							return L"IHV_START";
						case DOT11_CIPHER_ALGO_IHV_END:
							return L"IHV_END";
						default:
							trace.out( trace::category::error, L"network.security.algorithm.cipher: ? (%i)", static_cast< int >( algorithm ) );
							throw std::exception();
					}
				};

				wprintf_s( L"\n    security: %c\n      auth: %s\n      cipher: %s", 
					BOOL_CHAR( network.security.is_enabled ), 
					get__algorithm_name__auth( network.security.algorithm.auth ), 
					get__algorithm_name__cipher( network.security.algorithm.cipher ) 
				);
			}

			wprintf_s( L"\n    flags: 0x%x", network.flags );
		}
	}
	wprintf_s( L"\n" );
	
    return 0;
}

