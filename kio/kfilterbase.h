/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kfilterbase__h
#define __kfilterbase__h

#include <qobject.h>
class QIODevice;

class KFilterBase : public QObject
{
    Q_OBJECT
public:
    KFilterBase();
    virtual ~KFilterBase() {}

    void setDevice( QIODevice * dev ) { m_dev = dev; }
    QIODevice * device() { return m_dev; }
    virtual void init() {}
    virtual void terminate() {}
    virtual void reset() {}
    virtual bool readHeader() = 0;
    virtual void setOutBuffer( char * data, uint maxlen ) = 0;
    virtual void setInBuffer( char * data, uint maxlen ) = 0;
    virtual bool inBufferEmpty() const { return inBufferAvailable() == 0; }
    virtual int  inBufferAvailable() const = 0;
    virtual bool outBufferFull() const { return outBufferAvailable() == 0; }
    virtual int  outBufferAvailable() const = 0;

    enum Result { OK, END, ERROR };
    virtual Result uncompress() = 0;
protected:
    QIODevice * m_dev;
};

#endif
