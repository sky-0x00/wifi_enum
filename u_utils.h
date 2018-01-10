#pragma once

#include "u_types.h"
#include <guiddef.h>

struct guid {
	struct convert_params {
		case_type case_type;
		bool is_append_brakets;
	};
	static string_t get_text( _in const GUID &guid, _in const convert_params *p_convert_params = nullptr );
};
