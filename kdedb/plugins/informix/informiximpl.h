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
#ifndef INFORMIXIMPL_H
#define INFORMIXIMPL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qvaluelist.h>
#include <qstringlist.h>

#include <kdb/kdb.h>

#include <esqlc.h>


enum State
{
	Unused, Prepared, Allocated, Described, Declared, Opened, NoMoreData, Finished
};

typedef char Name[19];

//namespace KDB
//{

class InformixImpl : public KShared {

public:
	InformixImpl();
	InformixImpl( const QString & sql );
	~InformixImpl();

	int count_blobs( char * descriptorname );
	void blobs();
	int udts();
	bool close();

	inline bool isHoldCursor();
	inline bool isScrollCursor();
	inline int type();
	inline int noOfRows();
	inline int incrementNoOfRows();
	inline int noInputFields();
	inline int noOutputFields();
	inline int noInputBlobs();
	inline int noOutputBlobs();
	inline State state();
	inline Name & statementid();
	inline Name & inputDescriptorid();
	inline Name  & outputDescriptorid();
	inline Name  & cursorid();
	inline QString sql();

	inline void setIsHoldCursor( bool );
	inline void setIsScrollCursor( bool );
	inline void setType( int );
	inline void setNoOfRows( int );
	inline void setNoInputFields( int );
	inline void setNoOutputFields( int );
	inline void setNoInputBlobs( int );
	inline void setNoOutputBlobs( int );
	inline void setState( State );
	inline void setStatementid( Name & );
	inline void setInputDescriptorid( Name & );
	inline void setOutputDescriptorid( Name & );
	inline void setCursorid( Name & );

private:
	bool m_isHoldCursor;
	bool m_isScrollCursor;
	int m_type;
	int m_noOfRows;
	int m_noOfColumns;
	int m_noInputFields;
	int m_noOutputFields;
	int m_noInputBlobs;
	int m_noOutputBlobs;
	State m_state;
	Name & m_statementid;
	Name & m_inputDescriptorid;
	Name & m_outputDescriptorid;
	Name & m_cursorid;
	QString m_sql;
};

//}

#endif
