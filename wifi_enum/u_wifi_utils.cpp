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

		typedef DWORD status;

		status handle_open( 
			_in		DWORD	dwClientVersion /*= 2*/,
			_out	PDWORD	pdwNegotiatedVersion,
			_out	PHANDLE	phClient
		) {
			return ::WlanOpenHandle( dwClientVersion, nullptr, pdwNegotiatedVersion, phClient );
		}

		status handle_close(
			_in		HANDLE	hClient
		) {
			return ::WlanCloseHandle( hClient, nullptr );
		}

		status enum_ifaces(
			_in		HANDLE	hClient,
			_out	PWLAN_INTERFACE_INFO_LIST *ppInterfaceList
		) {
			return ::WlanEnumInterfaces( hClient, nullptr, ppInterfaceList );
		}
		status get_network_list(
			_in		HANDLE		hClient,
			_in		const GUID	*pInterface,
			_in		DWORD		dwFlags,
			_out	PWLAN_AVAILABLE_NETWORK_LIST	*ppNetworkList
		) {
			return ::WlanGetAvailableNetworkList( hClient, pInterface, dwFlags, nullptr, ppNetworkList );
		}
		status get_network_bsslist(
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

		status set_iface(
			_in	HANDLE				hClient,
			_in	const GUID			*pInterface,
			_in	WLAN_INTF_OPCODE	OpCode,
			_in	const VOID			*pData,
			_in	DWORD				dwDataSize
		) {
			return ::WlanSetInterface( hClient, pInterface, OpCode, dwDataSize, const_cast<PVOID>(pData), nullptr );
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
		state( state::not_ready )
	{}
	
	iface::iface( 
		_in const WLAN_INTERFACE_INFO &info 
	) :
		guid( info.InterfaceGuid ),
		description( info.strInterfaceDescription ),
		state( static_cast< enum class state >( info.isState ) )
	{}

	cstr_t iface::get_state_text(
	) const {
		const static std::map< enum class state, cstr_t > map_state = {
			{ state::not_ready, 				L"not_ready" },
			{ state::connected, 				L"connected" },
			{ state::adhoc_network_formed,	L"adhoc_network_formed" },
			{ state::disconnecting,			L"disconnecting" },
			{ state::disconnected,			L"disconnected" },
			{ state::associating,				L"associating" },
			{ state::discovering,				L"discovering" },
			{ state::authenticating,			L"authenticating" }
		};
		return map_state.at( state );
	}


	network::network( 
		_in const WLAN_AVAILABLE_NETWORK &info 
	) :
		name( { 
			string::convert( reinterpret_cast< const char* >(info.dot11Ssid.ucSSID), info.dot11Ssid.uSSIDLength ),
			info.strProfileName 
		}),
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
		assert( (info.dot11BssType == dot11_BSS_type_infrastructure) || (info.dot11BssType == dot11_BSS_type_independent) );
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
		const auto status = native::get_network_list( m_handle, &iface_guid, flags, &pNetworkList );

		if ( ERROR_SUCCESS != status ) {
			trace.out( trace::category::error, L"get_network_list(): %08x", status );
			throw std::exception( "", status );
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
	) const 
	{
		const auto status = native::get_network_bsslist( m_handle, &iface_guid, nullptr, dot11_BSS_type_any, false, ppWlanBssList );
		assert( ERROR_SUCCESS == status );
	}
	void manager::get_network_bsslist(
		_in const guid_t &iface_guid, 
		_in const network &network,
		_out PWLAN_BSS_LIST	*ppWlanBssList
	) const 
	{	
		const auto status = native::get_network_bsslist( 
			m_handle, &iface_guid, 
			network.name.ssid.c_str(), static_cast< DOT11_BSS_TYPE >(network.topology), network.security.is_enabled, 
			ppWlanBssList
		);
		assert( ERROR_SUCCESS == status );
	}

	void manager::get_network_bsslist(
		_in const guid_t &iface_guid, 
		_in const network &network,
		_out std::vector< network::bss > &bss_list
	) const 
	{
		bss_list.clear();
		PWLAN_BSS_LIST pBssList = nullptr;

		const auto status = native::get_network_bsslist( 
			m_handle, &iface_guid, 
			network.name.ssid.c_str(), static_cast< DOT11_BSS_TYPE >(network.topology), network.security.is_enabled, 
			&pBssList
		);
		assert( ERROR_SUCCESS == status );

		auto count = pBssList->dwNumberOfItems;
		for ( bss_list.reserve( count ); count--; )
		{
			const auto &BssEntry = pBssList->wlanBssEntries[count];
			assert( (BssEntry.dot11BssType == dot11_BSS_type_infrastructure) || (BssEntry.dot11BssType == dot11_BSS_type_independent) );
			assert( network.topology == static_cast< enum class network::bss::type >( BssEntry.dot11BssType ) );

			network::bss bss_entry; //= 
			//{
			//	/*topology =*/ static_cast< enum class network::bss::type >(network.topology),
			//	/*id =*/ address::mac(BssEntry.dot11Bssid),
			//	/*phy =*/ struct network::bss::phy({ BssEntry.uPhyId })
			//};
			bss_entry.topology = network.topology;
			bss_entry.id.assign( BssEntry.dot11Bssid );
			bss_entry.phy = { BssEntry.uPhyId };
			
			bss_list.push_back( bss_entry );
		}

		native::memory_free( pBssList );
	}

	/*protected*/ void manager::set_iface( 
		_in const guid_t &iface_guid, 
		_in iface::opcode iface_opcode,
		_in const data::untyped &param
	) const
	{
		assert( param.pdata && ( param.size > 0 ) );
		const auto status = native::set_iface( m_handle, &iface_guid, static_cast< WLAN_INTF_OPCODE >( iface_opcode ), param.pdata, param.size );
		assert( ERROR_SUCCESS == status );
	}
	void manager::set_iface__operation_mode( 
		_in const guid_t &iface_guid, 
		_in unsigned value 
	) const
	{
		const data::untyped param( value );
		set_iface( iface_guid, iface::opcode::current_operation_mode, param );
	}
}