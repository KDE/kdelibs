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
#ifndef KDB_EXCEPTION_H
#define KDB_EXCEPTION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qobject.h>
#include <kdebug.h>

namespace KDB {

/**
 * this class represents and exceptional condition
 * of the library. The type of exceptional condition
 * depends on the subclasses of this class
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */
class Exception : public QObject {

    Q_OBJECT
    
public:
    /**
     * Builds an exception with the given description
     * Stores name and type of the originating object
     */
    Exception (const QObject *origin, const QString & description)
        : m_description(description)
        {
            m_originName = QString::fromLocal8Bit(origin->name());
            m_originType = QString::fromLocal8Bit(origin->className());
            kdDebug(20000) << k_funcinfo << description << endl;
        };
    /**
     * this is the detailed description of the exception
     */
    QString description() const { return m_description; };

    /**
     * the name of the generating object
     */
    QString originName() const { return m_originName; };

    /**
     * the type of the originating object
     */
    QString originType() const { return m_originType; };
    
private:
    QString m_description;
    QString m_originName;
    QString m_originType;
};

class PluginException : public Exception {
    Q_OBJECT
public:
    PluginException (const QObject *origin, const QString & description) : Exception( origin,  description ) {};
};

class PluginNotFound : public PluginException {
    Q_OBJECT
public:
    PluginNotFound (const QObject *origin, const QString & description) : PluginException( origin, description ){};
};

class LibraryError : public PluginException {
    Q_OBJECT
public:
    LibraryError (const QObject *origin, const QString & description) : PluginException( origin, description ){};
};

class UnsupportedCapability: public PluginException {
    Q_OBJECT
public:
    UnsupportedCapability(const QObject *origin, const QString & description) : PluginException( origin, description ){};
};

class DataException : public Exception {
    Q_OBJECT
public:
    DataException (const QObject *origin, const QString & description) : Exception( origin, description ){};
};

class ConversionException : public DataException {
    Q_OBJECT
public:
    ConversionException (const QObject *origin, const QString & description) : DataException( origin, description ){};
};

class ServerError : public DataException {
    Q_OBJECT
public:
    ServerError (const QObject *origin, const QString & description) : DataException( origin, description ){};
};

class ObjectNotFound : public DataException {
    Q_OBJECT
public:
    ObjectNotFound (const QObject *origin, const QString & description) : DataException( origin, description ){};
};

class SQLError : public DataException {
    Q_OBJECT
public:
    SQLError (const QObject *origin, const QString & description) : DataException( origin, description ){};
};

class InvalidRequest : public DataException {
    Q_OBJECT
public:
    InvalidRequest (const QObject *origin, const QString & description) : DataException( origin, description ){};
};
 
class ConnectionException : public Exception {
    Q_OBJECT
public:
    ConnectionException (const QObject *origin, const QString & description) : Exception( origin, description ){};
};

class HostNotFound : public ConnectionException {
    Q_OBJECT
public:
    HostNotFound (const QObject *origin, const QString & description) : ConnectionException( origin, description ){};
};

class InvalidLogin : public ConnectionException {
    Q_OBJECT
public:
    InvalidLogin (const QObject *origin, const QString & description) : ConnectionException( origin, description ){};
};

}
#endif


