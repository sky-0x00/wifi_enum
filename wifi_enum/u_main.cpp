// wifi_enum.cpp: определ€ет точку входа дл€ консольного приложени€.
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
	
	wlan::manager wlan_manager;

	std::vector< wlan::iface > ifaces;
	wlan_manager.enum_ifaces( ifaces );

	wprintf_s( L"WiFi interface(s): %u\n", ifaces.size() );
	unsigned i_iface = 0;

	for ( const auto &iface : ifaces )
	{
		// DOT11_OPERATION_MODE_NETWORK_MONITOR - режим мониторинга, DOT11_OPERATION_MODE_EXTENSIBLE_STATION - рабочий режим
		//wlan_manager.set_iface__operation_mode( iface.guid, DOT11_OPERATION_MODE_NETWORK_MONITOR );
		//wlan_manager.set_iface__operation_mode( iface.guid, DOT11_OPERATION_MODE_EXTENSIBLE_STATION );

		wprintf_s( L"%u:\n  name: %s\n  guid: %s\n  state: %s\n", ++i_iface, iface.description.c_str(), guid::get_text( iface.guid ).c_str(), iface.get_state_text() );

		std::vector< wlan::network > networks;
		wlan_manager.get_network_list( iface.guid, 0/*WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES | WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES*/, networks );

		wprintf_s( L"\n  network(s): %u", networks.size() );
		unsigned i_network = 0;

		for ( const auto &network : networks )
		{
			//PWLAN_BSS_LIST pBssList = nullptr;
			//wifi.get_network_bsslist( iface.guid, network, &pBssList );
			//wlan::native::memory_free( pBssList );

			wprintf_s( L"\n  %u:\n    ssid: %s", ++i_network, network.name.ssid.c_str() );
			if ( !network.name.profile.empty() )
				wprintf_s( L" (profile: %s)", network.name.profile.c_str() );

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

			{
				std::vector< wlan::network::bss > bss_list;
				wlan_manager.get_network_bsslist( iface.guid, network, bss_list );
				//unsigned i_bss = 0;

				//OID_DOT11_BEACON_PERIOD

				wprintf_s( L"\n    bss_list: %u", bss_list.size() );
				for ( const auto &bss : bss_list )
					//wprintf_s( L"\n    %u:\n      %s", ++i_bss, bss.id.to_string().c_str() );
					wprintf_s( L"\n      %s", bss.id.to_string().c_str() );
			}

			wprintf_s( L"\n    flags: 0x%x", network.flags );
		}
	}
	wprintf_s( L"\n" );
	
    return 0;
}

