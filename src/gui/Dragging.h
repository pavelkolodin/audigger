/*
 *  pavelkolodin@gmail.com
 */

#ifndef _Dragging_H_
#define _Dragging_H_

#include <cstdlib>

template < typename T_cls, typename T_state, typename T_coord >
class Dragging
{		
	public:
		Dragging ( T_cls *_obj, T_coord _threshold, T_coord _min, T_coord _max )
    	: m_object(_obj)
		, m_func(NULL)
		, m_presscoord( 0 )
		, m_offset ( 0 )
		, m_min ( _min )
		, m_max ( _max )
		, m_threshold( _threshold )
		, m_dragging( false )
		{ }

		//virtual ~Dragging ( );

		void start(void (T_cls::*_f)( T_state, T_coord ), T_state _state, T_coord _presscoord, T_coord _offset )
		{
			m_func = _f;
			m_state = _state;
			m_presscoord = _presscoord;
			m_offset = _offset;
			m_dragging = false;
		}

		void update( T_coord _curr_coord )
		{
			if ( m_func )
			{
				if ( ! m_dragging )
					m_dragging = ( std::abs(m_presscoord - _curr_coord) > m_threshold );

				if ( m_dragging )
				{
					T_coord coord = _curr_coord - m_offset;
					if ( coord < m_min ) coord = m_min;
					if ( coord > m_max ) coord = m_max;
					(m_object->*m_func) (m_state, coord);
				}
			}
		}

		bool isDragging( ) { return m_dragging; }
		T_coord offset() { return m_offset; }
		T_state state() { return m_state; }

		void stop( void (T_cls::*_f)( T_state ) = 0 )
		{
			if ( 0 != _f && m_dragging )
				(m_object->*_f)( m_state );

			m_func = NULL;
			m_dragging = false;
		}

	private:
		T_cls *m_object;
		void (T_cls::*m_func)(T_state, T_coord);
		T_coord m_presscoord;
		T_coord m_offset;
		T_state m_state;
		T_coord m_min, m_max;
		T_coord m_threshold;
    	bool m_dragging;
};


#endif

