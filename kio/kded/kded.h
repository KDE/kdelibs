/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *            (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kded_h__
#define __kded_h__ 

#include <time.h>

#include <qobject.h>
#include <qstring.h>
#include <ksycoca.h>
#include <ksycocatype.h>

class KDirWatch;

// No need for this in libkio - apps only get readonly access
class Kded : public KSycoca
{
  Q_OBJECT
public:
   Kded( );
   virtual ~Kded();

   /**
    * process DCOP message.  Only calls to "recreate" are supported at
    * this time.
    */
   bool process(const QCString &fun, const QByteArray &data, 
		QCString &replyType, QByteArray &replyData);

public slots:

   /**
    * Recreate the database file
    */
   void recreate();

   /**
    * Collect all directories to watch
    */
   void build();

protected slots:

   /**
    * @internal Triggers rebuilding
    */
   void dirDeleted(const QString& path);
 
   /**
    * @internal Triggers rebuilding
    */
   void update (const QString& dir );

protected:

   /**
    * Scans dir for new files and new subdirectories.
    */
   void readDirectory(const QString& dir );
   
   /**
    * @internal
    * @return true if building (i.e. if a Kded);
    */
   virtual bool isBuilding() { return true; }

protected:

   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    */
   KDirWatch* m_pDirWatch;
   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   QTimer* m_pTimer;
};

#endif
