/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

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
#ifndef __kio_observer_h__
#define __kio_observer_h__

#include <qobject.h>

#include "kio/global.h"

class UIServer_stub;
class KURL;
namespace KIO {

class Job;

/**
 * Observer for @ref KIO::Job progress information
 * This class, of which there is always only one instance,
 * "observes" what jobs do and forwards this information
 * to the progress-info server.
 *
 * @short Observer for @ref KIO::Job progress information
 * @author David Faure <faure@kde.org>
 */
class Observer : public QObject {

  Q_OBJECT

public:

  static Observer * self() {
      if (!s_pObserver) s_pObserver = new Observer;
      return s_pObserver;
  }

  int newJob( KIO::Job * job );

protected:

  static Observer * s_pObserver;
  Observer();
  virtual ~Observer() {}

  UIServer_stub * m_uiserver;

public slots:

  virtual void slotTotalEntries( KIO::Job*, unsigned long );
  virtual void slotEntries( KIO::Job*, const KIO::UDSEntryList& );

  virtual void slotTotalSize( KIO::Job*, unsigned long size );
  virtual void slotTotalFiles( KIO::Job*, unsigned long files );
  virtual void slotTotalDirs( KIO::Job*, unsigned long dirs );

  virtual void slotProcessedSize( KIO::Job*, unsigned long size );
  virtual void slotProcessedFiles( KIO::Job*, unsigned long files );
  virtual void slotProcessedDirs( KIO::Job*, unsigned long dirs );

  virtual void slotSpeed( KIO::Job*, unsigned long bytes_per_second );
  virtual void slotPercent( KIO::Job*, unsigned long percent );

  virtual void slotCopying( KIO::Job*, const KURL& from, const KURL& to );
  virtual void slotMoving( KIO::Job*, const KURL& from, const KURL& to );
  virtual void slotDeleting( KIO::Job*, const KURL& from );
  virtual void slotCreatingDir( KIO::Job*, const KURL& dir );

  virtual void slotCanResume( KIO::Job*, bool can_resume );
};

};

// -*- mode: c++; c-basic-offset: 2 -*-
#endif
