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

#include <kdb/exception.h>
#include <kdb/dbengine.h>

#include "esqlc.h"

#include "handlerimpl.h"
#include "informiximpl.h"
#include "connectorimpl.h"


HandlerImpl::HandlerImpl( InformixImpl * ptr )
	:ifmx_hdl( ptr )
{
    kdDebug(20016) << "HandlerImpl::HandlerImpl start" << endl;

    //m_fields = informix_fetch_fields(res);
    //numFields = informix_num_fields(res);

    kdDebug(20016) << "HandlerImpl::HandlerImpl end" << endl;
}


HandlerImpl::~HandlerImpl()
{
	//kdDebug(20016) << "HandlerImpl::~HandlerImpl start" << endl;

    //informix_free_result(res);
	
	//kdDebug(20016) << "HandlerImpl::~HandlerImpl end" << endl;
}


KDB_ULONG HandlerImpl::count() const
{
	kdDebug(20016) << "HandlerImpl::count" << " Count = " << endl;

	return ifmx_hdl->noOfRows();
}


KDB::Row HandlerImpl::record(KDB_ULONG pos) const
{
	kdDebug(20016) << "HandlerImpl::record start" << endl;

    // TODO: transform it into a direct request to the backend if possible
    if( m_rows.isEmpty() )
	{
        rows(); 	//load the rows if not already done
	}

	kdDebug(20016) << "HandlerImpl::record stop" << endl;

    return m_rows[pos];
}


KDB::RowList HandlerImpl::rows() const
{
	EXEC SQL BEGIN DECLARE SECTION;
		int index;
		long length;
		long column_type;
		long column_length;
		long column_indicator;
		long extypeid;
		char column_name[19];
		varchar column_data[256];
		char * result;
		char * cursorid = ifmx_hdl->cursorid();
		char * descriptorid = ifmx_hdl->outputDescriptorid();
		lvarchar * lvar = 0;
		loc_t blob;
		dec_t decval;
	EXEC SQL END DECLARE SECTION;

	EXEC SQL BEGIN DECLARE SECTION;
		varchar longchar[32767];
	EXEC SQL END DECLARE SECTION;

	kdDebug(20016) << "HandlerImpl::rows start" << endl;

    if ( m_rows.isEmpty() ) 
	{
        while( sqlca.sqlcode == 0 && sqlca.sqlcode != NoMoreData )
		{
            KDB::Row f;

			if( ifmx_hdl->state() == NoMoreData )
			{
				sqlca.sqlcode = SQLNOTFOUND;
				break;
			}

			ifmx_hdl->blobs();

			EXEC SQL FETCH :cursorid USING SQL DESCRIPTOR :descriptorid;

			if( sqlca.sqlcode != 0 )
			{
				kdDebug(20016) << "HandlerImpl::rows" << " Fetch failed - sqlcode = " << sqlca.sqlcode << endl;
			
				if( sqlca.sqlcode == SQLNOTFOUND )
				{
					//close( ifmx_hdl );
				}

				break;
			}
			
			ifmx_hdl->incrementNoOfRows();

			for( index = 1; index <= ifmx_hdl->noOutputFields(); index++ )
			{
				EXEC SQL GET DESCRIPTOR :descriptorid 
   					     		  VALUE :index
							            :column_type = TYPE,
						                :column_length = LENGTH,
                        	            :column_indicator = INDICATOR,
                            	        :column_name = NAME;

				// Check to see if the data is null
				if( column_indicator == -1 )
				{
					kdDebug(20016) << "HandlerImpl::rows" << " Null value - column name = " << column_name << endl;
					result = column_data;
					length = 0;
					result[length] = '\0';
				}
				else
				{
					switch( column_type )
					{
						case SQLINT:
						case SQLSERIAL:
						case SQLSMINT:
						//case SQLDATA:
						case SQLDTIME:
						case SQLINTERVAL:
						case SQLBOOL:
						//case SQLSERIALS:
						//case SQLINTS:
						
							EXEC SQL GET DESCRIPTOR :descriptorid VALUE :index :column_data = DATA;

							result = column_data;
							length = byleng( result, strlen( result ) );
							result[length] = '\0';
							break;

						case SQLFLOAT:
						case SQLSMFLOAT:
						case SQLDECIMAL:
						case SQLMONEY:

							EXEC SQL GET DESCRIPTOR :descriptorid VALUE :index :decval = DATA;

							//strcpy( column_data, decgen( &decval, 0 ) );
							result = column_data;
							length = strlen( result );
							break;

/*						case SQLUDTFIXED:

							int error = 0;
							char cb = 'C';
							EXEC SQL BEGIN DECLARE SECTION;
								fixed binary ifx_lo_t bclob;
							EXEC SQL END DECLARE SECTION;

							EXEC SQL GET DESCRIPTOR :descriptorid VALUE :index :extypeid = EXTYPEID;

							result = NULL;

							switch( extypeid )
							{
								case XID_BLOB:
									cb = 'B';
									// fallthrough

								case XID_CLOB:
									int LO_fd;
									ifx_lo_stat_t * LO_stat;
									ifx_int8_t size;

									EXEC SQL GET DESCRIPTOR :descriptorid VALUE :index :bclob = DATA;

									LO_fd = ifx_lo_open( &bclob, LO_RDONLY, &error );	

									if( LO_fd == - 1 )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Error opening clob" << endl;
									}
									if( if_lo_stat( LO_fd, &LO_stat ) < 0 )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Error getting clob" << endl;
									}
									if( ifx_lo_stat_size( LO_stat, &size ) != 0 )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Error getting clob size" << endl;
									}
									if( ifx_int8tolong( &size, &length ) != 0  )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Error converting clob size to length" << endl;
									}
									if( ifx_lo_close( LO_fd ) != 0 )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Error closing clob" << endl;
									}
									if( ifx_lo_to_buffer( &bclob, length, &result, &error ) < 0 )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Error copying from clob" << endl;
									}
									break;

								default:
									kdDebug(20016) << "HandlerImpl::rows" << "Extended types not supported" << endl;
							}*/

							case SQLLVARCHAR:
							case SQLCHAR:
							case SQLNCHAR:
							case SQLTEXT:
							case SQLBYTES:

							default:
								kdDebug(20016) << "HandlerImpl::rows" << " Unknown type " << ifmx_hdl->type() << endl;
								length = 0;
								result = column_data;
								result[length] = '\0';
								break;
					}

					if( sqlca.sqlcode < 0 )
					{
						kdDebug(200016) << "HandlerImpl::rows" << " Descriptor get failed - sqlcode = " << sqlca.sqlcode << endl;
						*result = '\0';
					}

					if( result != column_data )
					{
						if( result != longchar )
						{
							switch( column_type )
							{
								case SQLLVARCHAR:
								
									if( ifx_var_dealloc( &lvar ) < 0 )
									{
										kdDebug(20016) << "HandlerImpl::rows" << " Unable to free lvarchar" << endl;
									}
									break;

								case SQLBYTES:
								case SQLTEXT:
									break;

								default:
									//free( result );
									break;
							}
						}
					}
				}

		Value v(QString::fromLocal8Bit(result,length));
               	f << s;
			}
				
        	m_rows << f;
    	}
	}

	kdDebug(20016) << "HandlerImpl::rows end" << endl;

    return m_rows ;
}


QStringList HandlerImpl::fields() const
{
    QStringList f;

	kdDebug(20016) << "HandlerImpl::fields start" << endl;

/*    for (unsigned int i = 0; i < numFields; i++) 
	{
        f << m_fields[i].name;
    }*/

	kdDebug(20016) << "HandlerImpl::fields end" << endl;
	
    return f;
}


QString HandlerImpl::nativeType( const QString & fieldName ) const
{
    unsigned int i = 0;
    QString res;

    kdDebug(20016) << k_funcinfo << endl;
	kdDebug(20016) << "HandlerImpl::nativeType start" << endl;

	i = i;
	res = fieldName;

/*    for ( ; i < numFields; ++i) 
	{
        if (fieldName == m_fields[i].name)
            break;
    }


    if (i == numFields) 
	{
        DBENGINE->pushError( new KDB::ObjectNotFound(this, fieldName));
        return QString::null;
    }

    QString res;

    switch (m_fields[i].type) 
	{
        case FIELD_TYPE_TINY:
            res = "TINYINT";
            break;
        case FIELD_TYPE_SHORT:
            res = "SMALLINT";
            break;
        case FIELD_TYPE_LONG:
            res = "INTEGER";
            break;
        case FIELD_TYPE_INT24:
            res = "MEDIUMINT";
            break;
        case FIELD_TYPE_LONGLONG:
            res = "BIGINT";
            break;
        case FIELD_TYPE_DECIMAL:
            res = "NUMERIC";
            break;
        case FIELD_TYPE_FLOAT:
            res = "FLOAT";
            break;
        case FIELD_TYPE_DOUBLE:
            res = "DOUBLE";
            break;
        case FIELD_TYPE_TIMESTAMP:
            res = "TIMESTAMP";
            break;
        case FIELD_TYPE_DATE:
            res = "DATE";
            break;
        case FIELD_TYPE_TIME:
            res = "TIME";
            break;
        case FIELD_TYPE_DATETIME:
            res = "DATETIME";
            break;
        case FIELD_TYPE_YEAR:
            res = "YEAR";
            break;
        case FIELD_TYPE_STRING:
            // which kind of string???
            res = "VARCHAR";
            break;
        case FIELD_TYPE_BLOB:
            res = "BLOB";
            break;
        case FIELD_TYPE_SET:
            res = "SET";
            break;
        case FIELD_TYPE_ENUM:
            res = "ENUM";
            break;
        default:
            break;
    }*/

	kdDebug(20016) << "HandlerImpl::nativeType end" << endl;

    return res;
}


KDB::DataType HandlerImpl::kdbDataType(const QString &fieldName) const
{
    kdDebug(20016) << k_funcinfo << endl;
	kdDebug(20016) << "HandlerImpl::kdbDataType start" << endl;

	kdDebug(20016) << "HandlerImpl::kdbDataType end" << endl;

    return ConnectorImpl::NToK(nativeType(fieldName));
}
