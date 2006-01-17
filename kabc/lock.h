/*
    This file is part of libkabc.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KABC_LOCK_H
#define KABC_LOCK_H

#include <qstring.h>
#include <qobject.h>

#include <kdelibs_export.h>

namespace KABC {

/**
  This class provides locking functionality for a file, directory or an
  arbitrary string-represented resource.
*/
class KABC_EXPORT Lock : public QObject
{
    Q_OBJECT
  public:
    /**
      Constructor.

      @param identifier An identifier for the resource to be locked, e.g. a file
                        name.
     */
    Lock( const QString &identifier );

    /**
      Destruct lock object. This also removes the lock on the resource.
    */
    ~Lock();

    /**
      Lock resource.
    */
    virtual bool lock();
    
    /**
      Unlock resource.
    */
    virtual bool unlock();

    virtual QString error() const;

    QString lockFileName() const;

    static bool readLockFile( const QString &filename, int &pid, QString &app );
    static bool writeLockFile( const QString &filename );

    static QString locksDir();

  Q_SIGNALS:
    void locked();
    void unlocked();

  private:
    QString mIdentifier;
    
    QString mLockUniqueName;

    QString mError;

    class Private;
    Private *d;
};

}

#endif
