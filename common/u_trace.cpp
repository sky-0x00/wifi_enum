#include "stdafx.h"

#include "u_trace.h"
#include <map>
#include <windows.h>
#include <varargs.h>
#include <memory>

trace::config::config(
	_in const channels &channels
) :
	m_channels( channels )
{}

const trace::config::channels& trace::config::get_channels(
) const {
	return m_channels;
}

bool trace::config::channels::check_type( 
	_in channel::type channel_type 
) const {
	return m_mask & static_cast<unsigned>( channel_type );
}

trace::config::channels::channels( 
	_in unsigned mask 
) :
	m_mask( mask )
{}

unsigned trace::config::channels::get_mask(
) const {
	return m_mask;
}

/*virtual*/ void trace::channel::debug::out( 
	_in cstr_t text 
) const {
	OutputDebugString( text );
	OutputDebugString( L"\n" );
}


trace::trace( 
	_in const config *p_config /*= nullptr*/ 
) {
	{
		static const config config_default( config::channels( static_cast<unsigned>( channel::type::debug ) ) );
		if ( !p_config )
			p_config = &config_default;
		process_config( *p_config );
	}
	/* ... */
}

void trace::out( 
	_in category type,
	_in cstr_t format, 
	_in ...
) {
	text__set_type( type );

	va_list ap;
	va_start( ap, format );
	text__format_v( format, ap );
	va_end( ap );

	text__out();

	m_text.clear();
}

void trace::text__set_type( 
	_in category type 
) {
	static const std::map< trace::category, char_t > map_chars = {
		{ category::normal, char_null },
		//{ type::error, L'W' },
		{ category::error, L'E' }
	};

	const char_t map_char = map_chars.at( type );
	if ( char_null == map_char )
		return;

	m_text += string_t(1, map_char) + L"> ";
}

void trace::text__format_v( 
	_in cstr_t format, 
	_in const va_list &ap 
) {
	char_t buffer[0x100];
	const auto result = vswprintf_s( buffer, format, ap );
	if ( result > 0 )
		m_text += buffer;
}

void trace::process_config( 
	_in const config &config 
) {
	process_config__channels( config.get_channels() );
}
void trace::process_config__channels( 
	_in const config::channels &channels 
) {
	if ( channels.check_type( channel::type::debug ) )
		m_channels.push_back( new channel::debug() );
	/* ... */
}

void trace::text__out(
) const {
	for ( auto p_channel : m_channels )
		p_channel->out( m_text.c_str() );
}

trace::~trace(
) {
	for ( auto &p_channel : m_channels )
		delete p_channel;
}