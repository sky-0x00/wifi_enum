#include "stdafx.h"

#include "u_wifi_utils.h"
#include <map>

#pragma comment( lib, "wlanapi.lib" )

namespace wlan {

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
			{ state::not_ready, 			L"not_ready" },
			{ state::connected, 			L"connected" },
			{ state::adhoc_network_formed,	L"adhoc_network_formed" },
			{ state::disconnecting,			L"disconnecting" },
			{ state::disconnected,			L"disconnected" },
			{ state::associating,			L"associating" },
			{ state::discovering,			L"discovering" },
			{ state::authenticating,		L"authenticating" }
		};
		return map_state.at( state );
	}

	namespace native {

		DWORD handle_open( 
			_in		DWORD	dwClientVersion /*= 2*/,
			_out	PDWORD	pdwNegotiatedVersion,
			_out	PHANDLE	phClientHandle
		) {
			return ::WlanOpenHandle( dwClientVersion, nullptr, pdwNegotiatedVersion, phClientHandle );
		}

		DWORD handle_close(
			_in		HANDLE	hClientHandle
		) {
			return ::WlanCloseHandle( hClientHandle, nullptr );
		}

		DWORD enum_ifaces(
			_in		HANDLE	hClientHandle,
			_out	PWLAN_INTERFACE_INFO_LIST *ppInterfaceList
		) {
			return ::WlanEnumInterfaces( hClientHandle, nullptr, ppInterfaceList );
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

	void manager::enum_ifaces( 
		_out std::vector< iface > &ifaces 
	) const {

		PWLAN_INTERFACE_INFO_LIST iface_list = nullptr;
		const auto result = native::enum_ifaces( m_handle, &iface_list );

		if ( ERROR_SUCCESS != result ) {
			trace.out( trace::category::error, L"enum_ifaces(): %08x", result );
			throw std::exception( "", result );
		}

		trace.out( trace::category::normal, L"enum_ifaces(): ok, %u iface(s)", iface_list->dwNumberOfItems );
		ifaces.clear();
		ifaces.reserve( iface_list->dwNumberOfItems );

		for ( /*iface_list->dwIndex = 0*/; iface_list->dwIndex < iface_list->dwNumberOfItems; ++iface_list->dwIndex )
			ifaces.emplace_back( iface_list->InterfaceInfo[ iface_list->dwIndex ] );

		native::memory_free( iface_list );
	}
}