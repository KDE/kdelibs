/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Scott Newton <sknewton@ihug.co.nz>

   Code based on DBD::Informix:
   Portions Copyright 1994-1995 Tim Bunce
   Portions Copyright 1995-1996 Alligator Descartes
   Portions Copyright 1994      Bill Hailes
   Portions Copyright 1996      Terry Nightingale
   Portions Copyright 1996-1999 Jonathan Leffler
   Portions Copyright 2000	    Informix Software Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <kdb/exception.h>
#include <kdb/dbengine.h>
#include <kdb/kdb.h>
#include <kdebug.h>

#include "informiximpl.h"

//using namespace KDB;

InformixImpl::InformixImpl()
{
	static long cursor_no = 0;

	kdDebug(20016) << "InformixImpl::InformixImpl start" << endl;

	m_isHoldCursor = false;
	m_isScrollCursor = false;
	m_type = 0;
	m_noOfRows = 0;
	m_noInputFields = 0;
	m_noOutputFields = 0;
	m_noInputBlobs = 0;
	m_noOutputBlobs = 0;
	m_state = Unused;
	sprintf( m_statementid, "s_%09ld", cursor_no );
	sprintf( m_inputDescriptorid, "i_%09ld", cursor_no );
	sprintf( m_outputDescriptorid, "o_%09ld", cursor_no );
	sprintf( m_cursorid, "c_%09ld", cursor_no );
	m_sql = QString::null;

	kdDebug(20016) << "InformixImpl::InformixImpl end" << endl;
}


InformixImpl::InformixImpl( const QString & s )
	: m_sql( s )
{
	static long cursor_no = 0;

	kdDebug(20016) << "InformixImpl::InformixImpl start" << endl;

	m_isHoldCursor = false;
	m_isScrollCursor = false;
	m_type = 0;
	m_noOfRows = 0;
	m_noInputFields = 0;
	m_noOutputFields = 0;
	m_noInputBlobs = 0;
	m_noOutputBlobs = 0;
	m_state = Unused;
	sprintf( m_statementid, "s_%09ld", cursor_no );
	sprintf( m_inputDescriptorid, "i_%09ld", cursor_no );
	sprintf( m_outputDescriptorid, "o_%09ld", cursor_no );
	sprintf( m_cursorid, "c_%09ld", cursor_no );

	kdDebug(20016) << "InformixImpl::InformixImpl end" << endl;
}


InformixImpl::~InformixImpl()
{
}


int InformixImpl::count_blobs( char * descriptorname )
{
	int no_of_blobs = 0;

	EXEC SQL BEGIN DECLARE SECTION;
		int column_no;
		int column_type;
		char * descriptorid = descriptorname;
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "InformixImpl::count_blobs start" << endl;

	for( column_no = 1; column_no <= m_noInputFields; column_no++ )
	{
		EXEC SQL GET DESCRIPTOR :descriptorid VALUE :column_no :column_type = TYPE;

		if( column_type == SQLBYTES || column_type == SQLTEXT )
		{
			no_of_blobs++;
		}
	}

	kdDebug(20016) << "InformixImpl::count_blobs end" << " No of blobs = " << no_of_blobs << endl;

	return no_of_blobs;
}


void InformixImpl::blobs()
{
	int no_of_columns;

	EXEC SQL BEGIN DECLARE SECTION;
		int column_no;
		int column_type;
		loc_t blob;
		char * descriptorid = m_outputDescriptorid;
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "InformixImpl::blobs start" << endl;
	
	no_of_columns = m_noOutputFields;
	
	m_noOutputBlobs = count_blobs( descriptorid );

	if( m_noOutputBlobs == 0 )
	{
		return;
	}

	kdDebug(20016) << "InformixImpl::blobs" << " WARNING: Blobs found" << endl;

	// TODO Blob location routines
/*	if( blob_locate( &blob, blob_binding ) != 0 )
	{
		kdDebug(20016) << "InformixImpl::blobs" << " Unable to allocate memory for blob" << endl;
		return
	}*/

	for( column_no = 1; column_no <= no_of_columns; column_no++ )
	{
		EXEC SQL GET DESCRIPTOR :descriptorid VALUE :column_no :column_type = TYPE;

		if( column_type == SQLBYTES || column_type == SQLTEXT )
		{
			EXEC SQL SET DESCRIPTOR :descriptorid VALUE :column_no DATA = :blob;
		}
	}

	kdDebug(20016) << "InformixImpl::blobs end" << endl;
}


int InformixImpl::udts()
{
	int nudts = 0;

	EXEC SQL BEGIN DECLARE SECTION;
		int column_no;
		int column_type;
		char * descriptorid = m_outputDescriptorid;
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "InformixImpl::udts start" << endl;

	for( column_no = 1; column_no <= m_noOutputFields; column_no++ )
	{
		EXEC SQL GET DESCRIPTOR :descriptorid VALUE :column_no :column_type = TYPE;

		// TODO Advanced data types
		/*if( ISCOMPLEXTYPE( column_type ) || ISUDTTYPE( column_type ) || ISDISTINCTTYPE( column_type ) )
		{
			coltype = SQLCHAR;

			EXEC SQL GET DESCRIPTOR :descriptorid VALUE :column_no TYPE = :column_type, LENGTH = 256;
			
			nudts++;
		}*/
	}
	
	kdDebug(20016) << "InformixImpl::udts start" << " No of udts = " << nudts << endl;
	
	return nudts;
}


bool InformixImpl::close()
{
	EXEC SQL BEGIN DECLARE SECTION;
		char * cursorid = cursorid;
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "InformixImpl::close start" << endl;

	if( m_state == Opened || m_state == Finished
	||  m_state == NoMoreData )
	{
		EXEC SQL CLOSE :cursorid;

		if( sqlca.sqlcode < 0 )
		{
			kdDebug(20016) << "InformixImpl::close" << " Close failed - sqlcode = " << sqlca.sqlcode << endl;
			return false;
		}

		if( m_state != NoMoreData )
		{
			m_state = Declared;
		}
	}

	kdDebug(20016) << "InformixImpl::close end" << endl;

	return true;
}


inline bool InformixImpl::isHoldCursor()
{
	return m_isHoldCursor;
}

	
inline bool InformixImpl::isScrollCursor()
{
	return m_isScrollCursor;
}


inline int InformixImpl::type()
{
	return m_type;
}


inline int InformixImpl::noOfRows()
{
	return m_noOfRows;
}


inline int InformixImpl::incrementNoOfRows()
{
	return m_noOfRows++;
}


inline int InformixImpl::noInputFields()
{
	return m_noInputFields;
}


inline int InformixImpl::noOutputFields()
{
	return m_noOutputFields;
}


inline int InformixImpl::noInputBlobs()
{
	return m_noInputBlobs;
}


inline int InformixImpl::noOutputBlobs()
{
	return m_noOutputBlobs;
}


inline State InformixImpl::state()
{
	return m_state;
}


inline Name & InformixImpl::statementid()
{
	return m_statementid;
}


inline Name & InformixImpl::inputDescriptorid()
{
	return m_inputDescriptorid;
}


inline Name & InformixImpl::outputDescriptorid()
{
	return m_outputDescriptorid;
}


inline Name & InformixImpl::cursorid()
{
	return m_cursorid;
}


inline QString InformixImpl::sql()
{
	return m_sql;
}


inline void InformixImpl::setIsHoldCursor( bool c )
{
	m_isHoldCursor = c;
}


inline void InformixImpl::setIsScrollCursor( bool c )
{
	m_isScrollCursor = c;
}


inline void InformixImpl::setType( int c )
{
	m_type = c;
}


inline void InformixImpl::setNoOfRows( int c )
{
	m_noOfRows = c;
}


inline void InformixImpl::setNoInputFields( int c )
{
	m_noInputFields = c;
}


inline void InformixImpl::setNoOutputFields( int c )
{
	m_noOutputFields = c;
}


inline void InformixImpl::setNoInputBlobs( int c )
{
	m_noInputBlobs = c;
}


inline void InformixImpl::setNoOutputBlobs( int c )
{
	m_noOutputBlobs = c;
}


inline void InformixImpl::setState( State c )
{
	m_state = c;
}


inline void InformixImpl::setStatementid( Name & c )
{
	strcpy( m_statementid, c );
}


inline void InformixImpl::setInputDescriptorid( Name & c )
{
	strcpy( m_inputDescriptorid, c );
}


inline void InformixImpl::setOutputDescriptorid( Name & c )
{
	strcpy( m_outputDescriptorid, c );
}


inline void InformixImpl::setCursorid( Name & c )
{
	strcpy( m_cursorid, c );
}
