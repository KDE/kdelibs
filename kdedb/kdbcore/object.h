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
#ifndef OBJECT_H
#define OBJECT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class QObject;

#include <qobject.h>
#include <qstring.h>
#include <qptrdict.h>
#include <qqueue.h>
#include "exception.h"

namespace KDB {

/**
 * Base class for all KDB objects.
 *
 * It inherits QObject and provide base
 * error reporting
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */
class Object: public QObject {

    Q_OBJECT

 public:

    Object( QObject *parent = 0L, const char *name = 0L);
    virtual ~Object();
    
    void pushError(Exception *) const;
    Exception *popError() const;
    Exception *getError() const;
    void clearErrors() const;
    
    bool error() const;

    QObject * parent() const { return m_parent; };

#if 0
    void printPendingObjects();
#endif
    
 private:

    QObject * m_parent;
    
    static QQueue<Exception> m_errors;

#if 0
    static QPtrDict<char> d;
    static int allocs;
#endif
};

}

#endif








