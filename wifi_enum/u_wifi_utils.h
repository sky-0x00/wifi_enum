#pragma once

#include "u_utils.h"
#include <vector>
#include <windows.h>
#include <wlanapi.h>

#define aas

namespace wlan {

	typedef unsigned long version;

	struct iface {					// аналог WLAN_INTERFACE_INFO

		enum class opcode {
			autoconf_enabled			= wlan_intf_opcode_autoconf_enabled,
			background_scan_enabled		= wlan_intf_opcode_background_scan_enabled,
			radio_state					= wlan_intf_opcode_radio_state,
			bss_type					= wlan_intf_opcode_bss_type,
			media_streaming_mode		= wlan_intf_opcode_media_streaming_mode,
			current_operation_mode		= wlan_intf_opcode_current_operation_mode
		};
		
		enum class state {		// аналог WLAN_INTERFACE_STATE (поле структуры WLAN_INTERFACE_INFO)
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
		state		state;
	};

	struct network {

		struct bss {

			enum class type {			// basic service set (bss) network type - "топология"
				infrastructure	= dot11_BSS_type_infrastructure,
				adhoc			= dot11_BSS_type_independent,
				//any			= dot11_BSS_type_any		// ?
			};
			struct phy {
				/*enum class type {
				};*/
				unsigned long id;
				//type type;
			};

			type topology;
			address::mac id;
			phy phy;
		};

		//network();
		network( _in const WLAN_AVAILABLE_NETWORK &info );
		
		bool is_connectable() const;
		char get__signal_level__rssi() const;

		struct {
			string_t ssid, profile;
		} 
		name;
		bss::type topology;
		unsigned bssid_count;									// 
		WLAN_REASON_CODE notconnactable_reasoncode;
		// TODO: PhyTypes - структура (ф-ия WlanGetNetworkBssList)
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

		handle_t get_handle() const;

		void enum_ifaces( 
			_out std::vector< iface > &ifaces 
		) const;
		void get_network_list( 
			_in const guid_t &iface_guid, 
			_in DWORD flags /*= WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES | WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES*/, 
			_out std::vector< network > &networks
		) const;
		void get_network_bsslist(
			_in const guid_t &iface_guid, 
			_out PWLAN_BSS_LIST	*ppWlanBssList
		) const;
		void get_network_bsslist(
			_in const guid_t &iface_guid, 
			_in const network &network,
			_out PWLAN_BSS_LIST	*ppWlanBssList
		) const;
		void get_network_bsslist(
			_in const guid_t &iface_guid, 
			_in const network &network,
			_out std::vector< network::bss > &bss_list
		) const;
		
		void set_iface__operation_mode( 
			_in const guid_t &iface_guid, 
			_in unsigned value
		) const;

	protected:
		void set_iface( 
			_in const guid_t &iface_guid, 
			_in iface::opcode iface_opcode,
			_in const data::untyped &param
		) const;

	private:
		handle_t m_handle;
	};
};
