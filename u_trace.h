#pragma once

#include "u_types.h"
#include <list>

class trace {
public:
	class channel {
	public:
		enum class type {		// элементы бинарной маски
			debug = 1
		};
		virtual void out( _in cstr_t text ) const = 0;
			
		//class debug: public channel {
		//	virtual void out( _in cstr_t text );
		//};
		class debug;
	};
	class channel::debug: public channel {
	public:
		virtual void out( _in cstr_t text ) const;
	};

	class config {
	public:
		class channels {
		public:
			channels( _in unsigned mask );
			unsigned get_mask() const;
			bool check_type( _in channel::type channel_type ) const;
		private:
			const unsigned m_mask;
		};
		
		config( _in const channels &channels );
		const channels& get_channels() const;

	private:
		const channels m_channels;
	};

	enum class category {
		normal,
		//warning,
		error
	};

public:
	trace( _in const config *p_config = nullptr );
	~trace();
	void out( _in category type, _in cstr_t format, _in ... );

private:
	void process_config( _in const config &config );
	void process_config__channels( _in const config::channels &channels );

	void text__set_type( _in category type );
	void text__format_v( _in cstr_t format, _in const va_list &ap );
	void text__out() const;

	std::list< channel* > m_channels;
	string_t m_text;
};


