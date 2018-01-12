#include "stdafx.h"

#include "u_wifi_utils.h"
#include "u_utils.h"
#include <map>
#include <cassert>
#include <sstream>

// Native Wifi: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms706556(v=vs.85).aspx
#pragma comment( lib, "wlanapi.lib" )

namespace wlan {

	namespace native {

		DWORD handle_open( 
			_in		DWORD	dwClientVersion /*= 2*/,
			_out	PDWORD	pdwNegotiatedVersion,
			_out	PHANDLE	phClient
		) {
			return ::WlanOpenHandle( dwClientVersion, nullptr, pdwNegotiatedVersion, phClient );
		}

		DWORD handle_close(
			_in		HANDLE	hClient
		) {
			return ::WlanCloseHandle( hClient, nullptr );
		}

		DWORD enum_ifaces(
			_in		HANDLE	hClient,
			_out	PWLAN_INTERFACE_INFO_LIST *ppInterfaceList
		) {
			return ::WlanEnumInterfaces( hClient, nullptr, ppInterfaceList );
		}
		DWORD get_network_list(
			_in		HANDLE		hClient,
			_in		const GUID	*pInterface,
			_in		DWORD		dwFlags,
			_out	PWLAN_AVAILABLE_NETWORK_LIST	*ppNetworkList
		) {
			return ::WlanGetAvailableNetworkList( hClient, pInterface, dwFlags, nullptr, ppNetworkList );
		}
		DWORD get_network_bsslist(
			_in				HANDLE			hClient,
			_in				const GUID		*pInterface,
			_in _optional	cstr_t			pszSsid,				// если NULL, то возвращаются информация обо всех bss на указанном интерфейсе
			_in	_optional	DOT11_BSS_TYPE	BssType,				// любое значение из bss_type, либо их or- объединение (валидно, если pszSsid != NULL)
			_in _optional	bool			bSecurityEnabled,		// (валидно, если pszSsid != NULL)
			_out			PWLAN_BSS_LIST	*ppWlanBssList
		) {
			if ( pszSsid ) 
			{
				DOT11_SSID Ssid;
				ssid::set_name( pszSsid, Ssid );				

				return ::WlanGetNetworkBssList( hClient, pInterface, &Ssid, BssType, bSecurityEnabled, nullptr, ppWlanBssList );
			} 
			
			return ::WlanGetNetworkBssList( hClient, pInterface, nullptr, BssType, bSecurityEnabled, nullptr, ppWlanBssList );
		}

		VOID memory_free(
			_in PVOID	pMemory
		) {
			::WlanFreeMemory( pMemory );
		}
		PVOID memory_allocate(
			_in	DWORD	dwMemorySize
		) {
			return ::WlanAllocateMemory( dwMemorySize );
		}
	}

	iface::iface(
	) :
		guid( GUID_NULL),
		state( state_t::not_ready )
	{}
	
	iface::iface( 
		_in const WLAN_INTERFACE_INFO &info 
	) :
		guid( info.InterfaceGuid ),
		description( info.strInterfaceDescription ),
		state( static_cast< enum class state_t >( info.isState ) )
	{}

	cstr_t iface::get_state_text(
	) const {
		const static std::map< enum class state_t, cstr_t > map_state = {
			{ state_t::not_ready, 				L"not_ready" },
			{ state_t::connected, 				L"connected" },
			{ state_t::adhoc_network_formed,	L"adhoc_network_formed" },
			{ state_t::disconnecting,			L"disconnecting" },
			{ state_t::disconnected,			L"disconnected" },
			{ state_t::associating,				L"associating" },
			{ state_t::discovering,				L"discovering" },
			{ state_t::authenticating,			L"authenticating" }
		};
		return map_state.at( state );
	}


	network::network( 
		_in const WLAN_AVAILABLE_NETWORK &info 
	) :
		profile_name( info.strProfileName ),
		ssid( string::convert( reinterpret_cast< const char* >(info.dot11Ssid.ucSSID), info.dot11Ssid.uSSIDLength ) ),
		topology( static_cast< enum class bss::type >( info.dot11BssType ) ),
		bssid_count( info.uNumberOfBssids ),
		notconnactable_reasoncode( info.bNetworkConnectable ? WLAN_REASON_CODE_SUCCESS : info.wlanNotConnectableReason ),
		signal_level__percentage( info.wlanSignalQuality ),
		security( { info.bSecurityEnabled != FALSE, { info.dot11DefaultAuthAlgorithm, info.dot11DefaultCipherAlgorithm } } ),
		flags( info.dwFlags )
	{
		assert( 
			(info.bNetworkConnectable && (info.wlanNotConnectableReason == WLAN_REASON_CODE_SUCCESS) ) ||
			(!info.bNetworkConnectable && (info.wlanNotConnectableReason != WLAN_REASON_CODE_SUCCESS) )
		);
		assert( info.wlanSignalQuality <= 100 );
		assert( info.dot11BssType != dot11_BSS_type_any );
	}

	bool network::is_connectable(
	) const {
		return WLAN_REASON_CODE_SUCCESS == notconnactable_reasoncode;
	}
	
	char network::get__signal_level__rssi(
	) const {
		const auto result = static_cast< char >((signal_level__percentage >> 1) - 100);
		assert( stdex::in_range< char >( result, { -100, -50+1 } ) );
		return result;
	}


	manager::manager(
		_in version version /*= 2*/
	) {
		struct s_version {
			wlan::version query, response;
		} s_version = { version, 0 };

		const auto result = native::handle_open( s_version.query, &s_version.response, &m_handle );
		
		if ( ERROR_SUCCESS == result )
			trace.out( trace::category::normal, L"c_tor(): ok, version(%i): %i, handle: %p", s_version.query, s_version.response, m_handle );
		else {
			trace.out( trace::category::error, L"c_tor(): %08x", result );
			throw std::exception( "", result );
		}
	}

	manager::~manager(
	) {
		const auto result = native::handle_close( m_handle );

		if ( ERROR_SUCCESS == result )
			trace.out( trace::category::normal, L"d_tor(): ok, handle: %p", m_handle );
		else {
			trace.out( trace::category::error, L"d_tor(): %08x, handle: %p", result, m_handle );
			//throw std::exception( "", result );
		}
	}

	handle_t manager::get_handle(
	) const {
		return m_handle;
	}

	void manager::enum_ifaces( 
		_out std::vector< iface > &ifaces 
	) const {

		PWLAN_INTERFACE_INFO_LIST pInterfaceList = nullptr;
		const auto result = native::enum_ifaces( m_handle, &pInterfaceList );

		if ( ERROR_SUCCESS != result ) {
			trace.out( trace::category::error, L"enum_ifaces(): %08x", result );
			throw std::exception( "", result );
		}

		trace.out( trace::category::normal, L"enum_ifaces(): ok, %u iface(s)", pInterfaceList->dwNumberOfItems );
		ifaces.clear();
		ifaces.reserve( pInterfaceList->dwNumberOfItems );

		for ( /*pInterfaceList->dwIndex = 0*/; pInterfaceList->dwIndex < pInterfaceList->dwNumberOfItems; ++pInterfaceList->dwIndex )
			ifaces.emplace_back( pInterfaceList->InterfaceInfo[ pInterfaceList->dwIndex ] );

		native::memory_free( pInterfaceList );
	}

	void manager::get_network_list(
		_in const guid_t &iface_guid,
		_in DWORD flags /*= WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES | WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES */,
		_out std::vector< network > &networks
	) const {

		PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = nullptr;
		const auto result = native::get_network_list( m_handle, &iface_guid, flags, &pNetworkList );

		if ( ERROR_SUCCESS != result ) {
			trace.out( trace::category::error, L"get_network_list(): %08x", result );
			throw std::exception( "", result );
		}

		trace.out( trace::category::normal, L"get_network_list(): ok, %u network(s)", pNetworkList->dwNumberOfItems );
		networks.clear();
		networks.reserve( pNetworkList->dwNumberOfItems );

		for ( /*pNetworkList->dwIndex = 0*/; pNetworkList->dwIndex < pNetworkList->dwNumberOfItems; ++pNetworkList->dwIndex )
			networks.emplace_back( pNetworkList->Network[ pNetworkList->dwIndex ] );

		native::memory_free( pNetworkList );
	}

	void manager::get_network_bsslist(
		_in const guid_t &iface_guid, 
		_out PWLAN_BSS_LIST	*ppWlanBssList
	) const {

		native::get_network_bsslist( m_handle, &iface_guid, nullptr, dot11_BSS_type_any, false, ppWlanBssList );
	}
	void manager::get_network_bsslist(
		_in const guid_t &iface_guid, 
		_in const network &network,
		_out PWLAN_BSS_LIST	*ppWlanBssList
	) const {
	
		native::get_network_bsslist( 
			m_handle, &iface_guid, 
			network.ssid.c_str(), static_cast< DOT11_BSS_TYPE >(network.topology), network.security.is_enabled, 
			ppWlanBssList
		);
	}
}