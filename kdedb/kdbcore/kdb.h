/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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

/**
 * @libdoc the KDE Data access library
 *
 * @sect Data Access Object Model
 *
 * The class hierarchy of Data access objects is contained in the namespace KDB.
 * All objects are QObjects, and event notification will be performed through QT
 * signal/slot mechanism
 *
 * @sect Functionality provided
 *
 * The kdbcore library provides primitives for accessing databases, and it
 * represent an 'abstraction layer' over the different database systems.
 * The implementation of the abstraction layer is done in DBMS specific 'plugins',
 * that are dynamically loadable libraries, loaded at runtime by the engine itself.
 *
 * Through a plugin is possible to open connections to different database servers.
 * Each Connection in turn provide access to the different databases present in
 * the server.
 *
 * A Database is a collection of Tables and stored Queries, and can be used to
 * open recordset based on entered queries (not stored ones).
 *
 * A Table object can be used to obtain table informations, modifying table
 * structure and open updatable recordsets on the content.
 *
 * A Query is a saved SQL statement that can be executed to retrieve the
 * corresponding recordset. One-table queries produce updatable recordsets.
 * A Query object can also be used to build SQL statement giving a list of
 * fields, tables and filter conditions.
 *
 * @sect The KDB type system
 *
 * Each DBMS implements its own type system. Most (but not all) of these type
 * systems are a superset of the SQL92 specification. The goal of the KDB core
 * library is to give the programmer a unified way to handle all that diversity
 * transparently.
 *
 * KDB defines a set of supported C++/QT types ( see @ref KDB::DataType enum ) that
 * are mapped at plugin level with the native types of the DBMS. Those types
 * that does not fit naturally with the native C++ types are converted either to QString
 * or to QByteArray . This way, all the power of the underlying DBMS can be used
 * by an expert programmer, but the novice (and all those people that don't need
 * or don't want to use esoteric types) has the ability of working with well
 * known C++ objects and have a transparent support for these.
 *
 * For example: the @ref KDB::Field object implements all the conversion operators
 * for the standard types. The implementation for a specific field type will throw
 * an exception if an incorrect conversion is attempted.
 *
 * @sect Dynamic capability system
 *
 * Not each DBMS implements the same set of features. Some don't use
 * transactions, or stored procedures, or other specific feature. Being a
 * general purpose library, KDB provides a dynamic access to those features
 * through the capability system.
 *
 * Each Plugin object can be queried to know if a specific capability is implemented
 * or to obtain a list of implemented capabilities. A list of currently supported
 * capabilities is available in the enum @ref KDB::capability.
 *
 * There are two kinds of capability: the ones that can be used/requested through
 * a method of some base object (like transactions, that apply to a Database)
 * and those that can be represented by a separate object in the hierarchy (like
 * stored procedures).
 *
 * The first kind of capability is implemented within the interface of the base
 * object that can implement it (like @ref KDB::Connection::beginTransaction ).
 * this implementation will throw an exception if the plugin does not support
 * such a capability
 *
 * The second type of capability is implemented by the Plugin object, through
 * the @ref KDB::Plugin::createObject method. With this method the programmer
 * can obtain a KDB::Capability subclass that can be casted to the specific type
 *
 * @sect Other useful notes
 *
 * Memory is managed internally by the library itself. Every object is created within
 * the library and either is destroyed when the library is unloaded or is a reference
 * counted object, that is unloaded when the last reference to it goes out of scope.
 *
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */

#ifndef KDB_H
#define KDB_H "$Id$"

#ifndef KDB_ULONG
#define KDB_ULONG unsigned long
#endif

#define QUERYTABLE "__KDBQueries"


// contains QByteArray definition
#include <qcstring.h>
#include <qvaluelist.h>

class Value;

namespace KDB {

    typedef QValueList< Value > Row;
    typedef QValueList< KDB::Row > RowList;

    /**
     * These are all the datatypes natively supported
     * by libkdbcore. Conversion to and from QT/C++ native types
     * is provided by the library.
     * See @ref KDB::Field for further details on kdbcore type system
     */
    enum DataType {
        UNKNOWN = -1,
        CHAR,
        VARCHAR,
        SMALLINT,
        INT,
        BIGINT,
        FLOAT,
        DOUBLE,
        DATE,
        TIME,
        TIMESTAMP,
        SET,
        ENUM,
        ARRAY,
        BLOB
    };

    /**
     * These are all currently known capabilities of a DBMS that a
     * plugin can support. Plugins are queryable about the supported
     * capabilities through @ref KDB::Plugin::provides and implementations
     * are available through @ref KDB::Plugin::createObject
     */
    enum capability {
        TRANSACTIONS,
        REORG,
        STOPROCS,
        VIEWS,
        ADMIN,
        SEQUENCES,
        FUNCTIONS,
        CONFIG
    };
}



#endif
