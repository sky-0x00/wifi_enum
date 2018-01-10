#pragma once

#include "u_types.h"
#include <vector>
#include <windows.h>
#include <wlanapi.h>

namespace wlan {

	typedef unsigned long version;

	struct iface {					// аналог WLAN_INTERFACE_INFO

		enum class state {			// аналог WLAN_INTERFACE_STATE (поле структуры WLAN_INTERFACE_INFO)
			not_ready				= wlan_interface_state_not_ready,
			connected				= wlan_interface_state_connected,
			adhoc_network_formed	= wlan_interface_state_ad_hoc_network_formed,
			disconnecting			= wlan_interface_state_disconnecting,
			disconnected			= wlan_interface_state_disconnected,
			associating				= wlan_interface_state_associating,
			discovering				= wlan_interface_state_discovering,
			authenticating			= wlan_interface_state_authenticating
		};

		GUID		guid;
		string_t	description;
		state		state;

		iface();
		iface( _in const WLAN_INTERFACE_INFO &info );

		cstr_t get_state_text() const;
	};

	class manager {
	public:
		manager( _in version version = 2 );
		~manager();

		void enum_ifaces( _out std::vector< iface > &ifaces ) const;

	private:
		handle_t m_handle;
	};
};
