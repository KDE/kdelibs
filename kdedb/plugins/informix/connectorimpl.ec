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

#include <kdebug.h>
#include <kdb/kdb.h>
#include <kdb/dbengine.h>
#include <kdb/exception.h>

#include "handlerimpl.h"
#include "informiximpl.h"
#include "connectorimpl.h"


ConnectorImpl::ConnectorImpl()
{
	kdDebug(20016) << "ConnectorImpl::ConnnectorImpl" << endl;
	ifmx_hdl = 0L;
}


ConnectorImpl::ConnectorImpl( const ConnectorImpl & c )
	: Connector( c )
{
  	kdDebug(20016) << "ConnectorImpl::ConnectorImpl" << endl;

    setHost( c.host() );
    setPort( c.port() );
    setUser( c.user() );
    setPassword( c.password() );
    setConnected( c.isConnected() );
}


ConnectorImpl::~ConnectorImpl()
{
	//kdDebug(20016) << "ConnectorImpl::~ConnectorImpl" << endl;
}


bool ConnectorImpl::connect()
{
	int isconnected = false;
	static long connection_number = 0;

	EXEC SQL BEGIN DECLARE SECTION;
		char * esql_database_name = 0L;
		char * esql_connection_name = 0L;
		char * esql_user_name = 0L;
		char * esql_password = 0L;
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "ConnectorImpl::connect start" << endl;

	esql_database_name = (char *) "test";
	sprintf( esql_connection_name, "conn_%09ld", connection_number++ );
	esql_user_name = (char *)user().latin1();
	esql_password = (char *)password().latin1();

	kdDebug(20016) << "ConnectorImpl::connect" << " " << esql_database_name << " " << esql_connection_name << " " << esql_user_name << " " << esql_password << endl;

	EXEC SQL CONNECT TO :esql_database_name 
	                 AS :esql_connection_name
				   USER	:esql_user_name 
				  USING	:esql_password
				   WITH CONCURRENT TRANSACTION;

	if( sqlca.sqlcode == 0 )
	{
		kdDebug(20016) << "ConnectorImpl::connect" << " Connect succeeded - connection name " << esql_connection_name << endl;

		isconnected = true;
		setConnected( true );
		connectionid = esql_connection_name;
	}
	else
	{
		kdDebug(20016) << "ConnectorImpl::connect" << " Connect failed - sqlcode = " << sqlca.sqlcode << endl;
		isconnected = false;
		setConnected( false );
	}

	kdDebug(20016) << "ConnectorImpl::connect end" << " Returning " << isconnected << endl;
	
	return( isconnected );
}


void ConnectorImpl::close()
{
	EXEC SQL BEGIN DECLARE SECTION;
		char * esql_connection_name;
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "ConnectorImpl::close start" << endl;

	//esql_connection_name = "test_connect";
	esql_connection_name = const_cast<char *>( connectionid.latin1() );

	EXEC SQL DISCONNECT :esql_connection_name;

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::close" << " Close failed - sqlcode = " << sqlca.sqlcode << endl;
	}

	kdDebug(20016) << "ConnectorImpl::close end" << endl; 
}


KDB::Connector * ConnectorImpl::clone()
{
  	kdDebug(20016) << "ConnectorImpl::clone start" << endl;

    ConnectorImpl * connection = new ConnectorImpl( *this );

  	kdDebug(20016) << "ConnectorImpl::clone end" << endl;

    return connection;
}


QStringList ConnectorImpl::databases() 
{
    QStringList lst;
    KDB::RowList::Iterator it;

	kdDebug(20016) << "ConnectorImpl::databases start" << endl;

    KDB::RowList l = resultQuery( "select name from sysmaster:\"informix\".sysdatabases" );

    for( it = l.begin(); it != l.end(); ++it ) 
	{
        lst << (*it)[0];
    }

	kdDebug(20016) << "ConnectorImpl::databases end" << endl;

    return lst;
}


QStringList ConnectorImpl::tables() 
{
	QStringList t;

	kdDebug(20016) << "ConnectorImpl::tables start" << endl;

	kdDebug(20016) << "ConnectorImpl::tables end" << endl;

	return t;
}


KDB::RowList ConnectorImpl::fields( const QString & tableName ) 
{
	QString x;
	KDB::RowList r;

	x = tableName;
	kdDebug(20016) << "ConnectorImpl::fields start" << endl;

	kdDebug(20016) << "ConnectorImpl::fields end" << endl;

	return r;
}


KDB::DataType ConnectorImpl::nativeToKDB( const QString & type )
{
	QString x;
	KDB::DataType t;
	x = type;
	t = (KDB::DataType)1;
	kdDebug(20016) << "ConnectorImpl::nativeToKDB start" << endl;

	kdDebug(20016) << "ConnectorImpl::nativeToKDB end" << endl;

	return t;
}


QString ConnectorImpl::KDBToNative( KDB::DataType type )
{
	type = type;

	kdDebug(20016) << "ConnectorImpl::KDBToNative start" << endl;

	kdDebug(20016) << "ConnectorImpl::KDBToNative end" << endl;

	return QString::null;
}


//static 
KDB::DataType ConnectorImpl::NToK( const QString & type )
{
	QString x;
	KDB::DataType t;
	x = type;
	t = (KDB::DataType)1;

	kdDebug(20016) << "ConnectorImpl::NToK start" << endl;

	kdDebug(20016) << "ConnectorImpl::NToK end" << endl;

	return t;
}


//static 
QString ConnectorImpl::KToN( KDB::DataType type )
{
	type = type;

	kdDebug(20016) << "ConnectorImpl::KToN start" << endl;

	kdDebug(20016) << "ConnectorImpl::KToN end" << endl;

	return QString::null;
}


bool ConnectorImpl::createDatabase( const QString & name ) 
{
	QString x;
	x = name;

	kdDebug(20016) << "ConnectorImpl::createDatabase start" << endl;

	kdDebug(20016) << "ConnectorImpl::createDatabase end" << endl;

	return true;
}


bool ConnectorImpl::dropDatabase( const QString & name ) 
{
	QString x;
	x = name;

	kdDebug(20016) << "ConnectorImpl::dropDatabase start" << endl;

	kdDebug(20016) << "ConnectorImpl::dropDatabase end" << endl;

	return true;
}


bool ConnectorImpl::createTable( const KDB::Table & tab ) 
{
	QString x = tab.name();
	kdDebug(20016) << "ConnectorImpl::createTable start" << endl;

	kdDebug(20016) << "ConnectorImpl::createTable end" << endl;

	return true;
}


bool ConnectorImpl::dropTable( const QString & name ) 
{
	QString x;
	x = name;

	kdDebug(20016) << "ConnectorImpl::dropTable start" << endl;

	kdDebug(20016) << "ConnectorImpl::dropTable end" << endl;

	return true;
}


bool ConnectorImpl::setCurrentDatabase( const QString & name )
{
	QString x;
	x = name;

	kdDebug(20016) << "ConnectorImpl::setCurrentDatabase start" << endl;

	kdDebug(20016) << "ConnectorImpl::setCurrentDatabase end" << endl;

	return true;
}


KDB_ULONG ConnectorImpl::execute( const QString & sql )
{
	KDB_ULONG row_count = 0;

	kdDebug(20016) << "ConnectorImpl::execute start" << " Sql = " << sql << endl;

	if( isConnected() == false )
	{
		return 0;
	}

	InformixImpl * ifmx_hdl = new InformixImpl( sql );

	//ifmx_hdl = ifmxptr;

	row_count = prepare( sql );

	kdDebug(20016) << "ConnectorImpl::execute end" << " Row count = " << row_count << endl;
	
	return row_count;
}


KDB::Handler * ConnectorImpl::query( const QString & sql ) 
{
	kdDebug(20016) << "ConnectorImpl::query start" << " Sql = " << sql << endl;

	execute( sql );

	if( DBENGINE->error() ) 
	{
		return 0L;
	}

	HandlerImpl * res = new HandlerImpl( ifmx_hdl );

	kdDebug(20016) << "ConnectorImpl::query end" << endl;
	
	return res;
}


KDB::RowList ConnectorImpl::resultQuery( const QString & sql )
{
	kdDebug(20016) << "ConnectorImpl::resultQuery start" << " Sql = " << sql << endl;

	HandlerImpl * res = static_cast<HandlerImpl *>( query( sql ) );

	KDB::RowList l = res->rows();

	delete res;

	kdDebug(20016) << "ConnectorImpl::resultQuery end" << endl;
	
	return l;
}


int ConnectorImpl::preparse()
{
	int idx = 0;
	int style = 0;
	//int count = 0;
	int laststyle = 0;
	char ch;
	char end_quote = '\0';
	char * src = 0L;
	char * dst = 0L;

	kdDebug(20016) << "ConnectorImpl::preparse start" << endl;

	strcpy( src, ifmx_hdl->sql().latin1() );
	strcpy( dst, ifmx_hdl->sql().latin1() );

	while ((ch = *src++) != '\0')
	{
		if (ch == end_quote)
			end_quote = '\0';
		else if (end_quote != '\0')
		{
			*dst++ = ch;
			continue;
		}
		else if (ch == '\'' || ch == '\"')
			end_quote = ch;
		else if (ch == '{')
			end_quote = '}';
		else if (ch == '-' && *src == '-')
		{
			end_quote = '\n';
		}
		if (ch == '?')
		{
			// X/Open standard	 
			*dst++ = '?';
			idx++;
			style = 3;
		}
		else
		{
			// Perhaps ':=' PL/SQL construct or dbase:table in Informix 
			// Or it could be :2 or :22 as part of a DATETIME/INTERVAL 
			*dst++ = ch;
			continue;
		}
		//if (laststyle && style != laststyle)
			//croak("Can't mix placeholder styles (%d/%d)", style, laststyle);
		laststyle = style;
	}
	if (end_quote != '\0')
	{
		switch (end_quote)
		{
		case '\'':
			//warn("Incomplete single-quoted string\n");
			break;
		case '\"':
			//warn("Incomplete double-quoted string (delimited identifier)\n");
			break;
		case '}':		//R_CURLY:
			//warn("Incomplete bracketed {...} comment\n");
			break;
		case '\n':
			//warn("Incomplete double-dash comment\n");
			break;
		default:
			//assert(0);
			break;
		}
	}
	*dst = '\0';

	kdDebug(20016) << "ConnectorImpl::preparse end" << " Index = " << idx << endl;
	
	return idx;
}


void ConnectorImpl::deleteStatement()
{
	// TODO To be implemented

	kdDebug(20016) << "ConnectorImpl::deleteStatement start" << endl;

	kdDebug(20016) << "ConnectorImpl::deleteStatement end" << endl;

}


bool ConnectorImpl::allocateSpace( int input_param_count )
{
	EXEC SQL BEGIN DECLARE SECTION;
		int count = input_param_count;
		char * descriptorid = ifmx_hdl->inputDescriptorid();
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "ConnectorImpl::allocateSpace start" << endl;

	// TODO Make sure we have the right connection

	if( input_param_count > ifmx_hdl->noInputFields() )
	{
		if( ifmx_hdl->noInputFields() > 0 )
		{
			EXEC SQL DEALLOCATE DESCRIPTOR :descriptorid;

			ifmx_hdl->setNoInputFields( 0 );

			if( sqlca.sqlcode < 0 )
			{
				kdDebug(20016) << "ConnectorImpl::allocateSpace" << " Deallocation of descriptor failed - sqlcode = " << sqlca.sqlcode << endl;
				return( false );
			}
		}

		EXEC SQL ALLOCATE DESCRIPTOR :descriptorid WITH MAX :count;

		if( sqlca.sqlcode < 0 )
		{
			kdDebug(20016) << "ConnectorImpl::allocateSpace" << " Allocation of descriptor failed - sqlcode = " << sqlca.sqlcode << endl;
			return( false );
		}

		ifmx_hdl->setNoInputFields( input_param_count );
	}

	kdDebug(20016) << "ConnectorImpl::allocateSpace end" << endl;

	return( true );
}


int ConnectorImpl::countDescriptors()
{
	int count;
	Sqlda * sqlda_struct;

	EXEC SQL BEGIN DECLARE SECTION;
		char * statementid = ifmx_hdl->statementid();
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "ConnectorImpl::countDescriptors start" << endl;

	EXEC SQL DESCRIBE :statementid INTO sqlda_struct;

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::countDescriptors" << " Describe statement failed - sqlcode = " << sqlca.sqlcode << endl;
		return -1;
	}

	count = sqlda_struct->sqld;

	// TODO Handle esql version 501 - 6 SQLFreeMem
	free( sqlda_struct );

	kdDebug(20016) << "ConnectorImpl::countDescriptors end" << " Count = " << count << endl; 

	return( count );
}


bool ConnectorImpl::open()
{
	EXEC SQL BEGIN DECLARE SECTION;
		char * cursorid = ifmx_hdl->cursorid();
		char * input_descriptorid = ifmx_hdl->inputDescriptorid();
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "ConnectorImpl::open start" << endl;

	if( ifmx_hdl->state() == Opened || ifmx_hdl->state() == Finished )
	{
		ifmx_hdl->close();
	}

	ifmx_hdl->setState( Declared );

	if( ifmx_hdl->noInputFields() > 0 )
	{
		EXEC SQL OPEN :cursorid USING SQL DESCRIPTOR :input_descriptorid;
	}
	else
	{
		EXEC SQL OPEN :cursorid;
	}

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::open" << " Opened failed - sqlcode = " << sqlca.sqlcode << endl;
		return false;
	}

	ifmx_hdl->setState( Opened );
	ifmx_hdl->setNoOfRows( 0 );

	kdDebug(20016) << "ConnectorImpl::open end" << endl;

	return true;
}

bool ConnectorImpl::exec()
{
	kdDebug(20016) << "ConnectorImpl::exec start" << endl;
	
	kdDebug(20016) << "ConnectorImpl::exec end" << endl;
	
	return false;
}


/*int ConnectorImpl::execute( const QString & sql )
{
	bool status = false;
	int row_count = 0;

	kdDebug(20016) << "ConnectorImpl::execute start" << " Sql = " << sql << endl;

	if( prepare( sql ) == false )
	{
		return false;
	}

	if( isConnected() == false )
	{
		return false;
	}

	// TODO Check for correct connection active

	if( type == SQ_SELECT )
	{
		status = open();
	}
	else if( type == SQ_EXECPROC && no_output_fields > 0 )
	{
		status = open();
	}
	else
	{
		status = exec();
	}

	if( status == false && sqlca.sqlcode < 0 )
	{
		row_count = sqlca.sqlcode;
	}
	else
	{
		if( ( sqlca.sqlcode == 0 || sqlca.sqlcode == SQLNOTFOUND ) 
		&& sqlca.sqlerrd[2] >= 0 )
		{
			row_count = sqlca.sqlerrd[2];
		}
	}

	kdDebug(20016) << "ConnectorImpl::execute end" << " Row count = " << row_count << endl;
	
	return row_count;
}*/


int ConnectorImpl::prepare( const QString & sql )
{
	bool status = false;
	int nudts = 0;
	int input_count = 0;
	int row_count = 0;

	EXEC SQL BEGIN DECLARE SECTION;
		int descriptor_count;
		char * statement = (char *) ifmx_hdl->sql().latin1();
		char * statementid = ifmx_hdl->statementid();
		char * descriptorid = ifmx_hdl->inputDescriptorid();
		char * cursorid = ifmx_hdl->cursorid();
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "CoonectorImpl::prepare start" << " Sql=" << sql << endl;

	if( ( ifmx_hdl->sql() == QString::null ) || ( ifmx_hdl->sql().length() == 0 ) )  
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " Sql is null or empty" << endl;
		return false;	// Prepare failed
	}

	/*if( isConnected() == false )
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " Not connected" << endl;
		return false;
	}*/

	// TODO Make sure we have the connection we want

	//initializeStatementHandle();
	//statementid = statementid;
	//descriptorid = output_descriptorid;
	//cursorid = cursorid;
	//sql = sql.latin1();

	// TODO Check for hold/scroll cursors	

	// Pre-parse the sql statement to find out the number of input parameters
	input_count = preparse();

	// Now allocate sufficient space for that number of input parameters
	if( allocateSpace( input_count ) == false )
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " Unable to allocate space for descriptor" << endl;
		deleteStatement();
		return false;
	}

	EXEC SQL PREPARE :statementid FROM :statement;

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " ~prepare of sql statement failed - sqlcode = " << sqlca.sqlcode << endl;
		deleteStatement();
		return false;
	}

	ifmx_hdl->setState( Prepared );

	descriptor_count = countDescriptors();

	// sql descriptors have have WITH MAX count of at least one (-470)
	if( descriptor_count == 0 )
	{
		descriptor_count = 1;
	}

	EXEC SQL ALLOCATE DESCRIPTOR :descriptorid WITH MAX :descriptor_count;

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " Allocation of descriptor failed - sqlcode = " << sqlca.sqlcode << endl;
		deleteStatement();
		return false;
	}

	ifmx_hdl->setState( Allocated );

	EXEC SQL DESCRIBE :statementid USING SQL DESCRIPTOR :descriptorid;

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " Describe statement failed - sqlcode = " << sqlca.sqlcode << endl;
		deleteStatement();
		return false;
	}

	ifmx_hdl->setState( Described );
	ifmx_hdl->setType( sqlca.sqlcode );

	if( ifmx_hdl->type() == 0 )
	{
		ifmx_hdl->setType( SQ_SELECT );
	}
	
	EXEC SQL GET DESCRIPTOR :descriptorid :descriptor_count = COUNT;

	if( sqlca.sqlcode < 0 )
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " Descriptor get failed - sqlcode = " << sqlca.sqlcode << endl;
		deleteStatement();
		return false;
	}

	ifmx_hdl->setNoOutputFields( descriptor_count );

	if( ifmx_hdl->type() == SQ_SELECT )
	{
		//status = declare();
	}
	else if( ifmx_hdl->type() == SQ_EXECPROC && descriptor_count > 0 )
	{
		//status = declare();
	}
	else if( ifmx_hdl->type() == SQ_INSERT && descriptor_count > 0 )
	{
		//nudts = ifmx_hdl->udts();
		ifmx_hdl->blobs();

		if( ifmx_hdl->noOutputBlobs() > 0 || nudts > 0 )
		{
			Name tmpname;
			strcpy( tmpname, ifmx_hdl->inputDescriptorid() );
			strcpy( ifmx_hdl->inputDescriptorid(), ifmx_hdl->outputDescriptorid() );
			strcpy( ifmx_hdl->outputDescriptorid(), tmpname );
			ifmx_hdl->setNoInputFields( descriptor_count );
		}

		status = true;
	}
	else if( ifmx_hdl->type() == SQ_UPDATE && descriptor_count > 0 )
	{
		static int count = 0;

		if( count == 0 )
		{
			count++;
		}
		
		ifmx_hdl->blobs();

		if( ifmx_hdl->noOutputBlobs() > 0 )
		{
			Name tmpname;
			strcpy( tmpname, ifmx_hdl->inputDescriptorid() );
			strcpy( ifmx_hdl->inputDescriptorid(), ifmx_hdl->outputDescriptorid() );
			strcpy( ifmx_hdl->outputDescriptorid(), tmpname );
			ifmx_hdl->setNoInputFields( descriptor_count );
		}

		status = true;
	}
	else
	{
		kdDebug(20016) << "ConnectorImpl::prepare" << " No match for type = " << ifmx_hdl->type() << endl;
		
		EXEC SQL DEALLOCATE DESCRIPTOR :descriptorid;
		
		ifmx_hdl->setState( Prepared );
		status = true;
	}

	if( ifmx_hdl->type() == SQ_SELECT )
	{
		status = open();
	}
	else if( ifmx_hdl->type() == SQ_EXECPROC && ifmx_hdl->noOutputFields() > 0 )
	{
		status = open();
	}
	else
	{
		status = exec();
	}

	if( status == false && sqlca.sqlcode < 0 )
	{
		row_count = sqlca.sqlcode;
	}
	else
	{
		if( ( sqlca.sqlcode == 0 || sqlca.sqlcode == SQLNOTFOUND ) 
		&& sqlca.sqlerrd[2] >= 0 )
		{
			row_count = sqlca.sqlerrd[2];
		}
	}

	kdDebug(20016) << "ConnectorImpl::execute end" << " Row count = " << row_count << endl;
	
	return row_count;
}
