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
#ifndef EXCEPTION_H
#define EXCEPTION_H

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
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */
class Exception : public QObject {

    Q_OBJECT
    
public:
    /**
     * Builds an exception with the given description
     *     */
    Exception (const QString & description) :m_description(description)
        {
            kdDebug(20000) << k_funcinfo << description << endl;
        };
    /**
     * this is the detailed description of the exception
     */
    QString description() { return m_description; };

private:
    QString m_description;

};

class PluginException : public Exception {
    Q_OBJECT
public:
    PluginException (const QString & description) : Exception( description ) {};
};

class PluginNotFound : public PluginException {
    Q_OBJECT
public:
    PluginNotFound (const QString & description) : PluginException( description ){};
};

class LibraryError : public PluginException {
    Q_OBJECT
public:
    LibraryError (const QString & description) : PluginException( description ){};
};

class UnsupportedCapability: public PluginException {
    Q_OBJECT
public:
    UnsupportedCapability(const QString & description) : PluginException( description ){};
};

class DataException : public Exception {
    Q_OBJECT
public:
    DataException (const QString & description) : Exception( description ){};
};

class ConversionException : public DataException {
    Q_OBJECT
public:
    ConversionException (const QString & description) : DataException( description ){};
};

class ServerError : public DataException {
    Q_OBJECT
public:
    ServerError (const QString & description) : DataException( description ){};
};

class ObjectNotFound : public DataException {
    Q_OBJECT
public:
    ObjectNotFound (const QString & description) : DataException( description ){};
};

class SQLError : public DataException {
    Q_OBJECT
public:
    SQLError (const QString & description) : DataException( description ){};
};

class InvalidRequest : public DataException {
    Q_OBJECT
public:
    InvalidRequest (const QString & description) : DataException( description ){};
};
 
class ConnectionException : public Exception {
    Q_OBJECT
public:
    ConnectionException (const QString & description) : Exception( description ){};
};

class HostNotFound : public ConnectionException {
    Q_OBJECT
public:
    HostNotFound (const QString & description) : ConnectionException( description ){};
};

class InvalidLogin : public ConnectionException {
    Q_OBJECT
public:
    InvalidLogin (const QString & description) : ConnectionException( description ){};
};

}
#endif


