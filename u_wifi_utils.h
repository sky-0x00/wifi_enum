#pragma once

#include "u_types.h"
#include <vector>
#include <windows.h>
#include <wlanapi.h>

namespace wlan {

	typedef unsigned long version;

	struct iface {					// ������ WLAN_INTERFACE_INFO

		enum class state_t {		// ������ WLAN_INTERFACE_STATE (���� ��������� WLAN_INTERFACE_INFO)
			not_ready				= wlan_interface_state_not_ready,
			connected				= wlan_interface_state_connected,
			adhoc_network_formed	= wlan_interface_state_ad_hoc_network_formed,
			disconnecting			= wlan_interface_state_disconnecting,
			disconnected			= wlan_interface_state_disconnected,
			associating				= wlan_interface_state_associating,
			discovering				= wlan_interface_state_discovering,
			authenticating			= wlan_interface_state_authenticating
		};

		iface();
		iface( _in const WLAN_INTERFACE_INFO &info );

		cstr_t get_state_text() const;

		guid_t		guid;
		string_t	description;
		state_t		state;
	};

	struct network {

		enum class bss_type {			// basic service set (bss) network type - "���������"
			infrastructure	= dot11_BSS_type_infrastructure,
			adhoc			= dot11_BSS_type_independent,
			//any				= dot11_BSS_type_any		// ?
		};

		//network();
		network( _in const WLAN_AVAILABLE_NETWORK &info );
		
		bool is_connectable() const;
		char get__signal_level__rssi() const;

		string_t profile_name;
		string_t ssid;
		bss_type topology;
		unsigned bssid_count;									// 
		WLAN_REASON_CODE notconnactable_reasoncode;
		// TODO: PhyTypes - ��������� (�-�� WlanGetNetworkBssList)
		WLAN_SIGNAL_QUALITY signal_level__percentage;			// [0..100] - [-100..-50] dbm
		
		struct security {
			bool is_enabled;

			struct algorithm {
				DOT11_AUTH_ALGORITHM auth;
				DOT11_CIPHER_ALGORITHM cipher;
			} 
			algorithm;
		} 
		security;

		DWORD flags;									// ?
	};

	class manager {
	public:
		manager( _in version version = 2 );
		~manager();

		void enum_ifaces( 
			_out std::vector< iface > &ifaces 
		) const;
		void get_network_list( 
			_in const guid_t &iface_guid, 
			_in DWORD flags /*= WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES | WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES*/, 
			_out std::vector< network > &networks
		) const;

	private:
		handle_t m_handle;
	};
};
