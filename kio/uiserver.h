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
#ifndef __kio_uiserver_h__
#define __kio_uiserver_h__

#include <dcopobject.h>
#include <kio/global.h>
#include <kurl.h>

/**
 * @short Graphical server for progress information
 * @author David Faure <faure@kde.org>
 */
class UIServer : public DCOPObject {

  K_DCOP

public:

  UIServer();
  virtual ~UIServer() {}

k_dcop:
  /**
   * Signal a new job
   * @param the DCOP application id of the job's parent application
   *   (@see KIO::Observer::newJob)
   * @return the job id
   */
  int newJob( QCString appId );

  void jobFinished( int id );

  void totalSize( int id, unsigned long size );
  void totalFiles( int id, unsigned long files );
  void totalDirs( int id, unsigned long dirs );

  void processedSize( int id, unsigned long bytes );
  void processedFiles( int id, unsigned long files );
  void processedDirs( int id, unsigned long dirs );

  void percent( int id, unsigned long ipercent );
  void speed( int id, unsigned long bytes_per_second );

  void canResume( int id, unsigned int can_resume );

  void copying( int id, KURL from, KURL to );
  void moving( int id, KURL from, KURL to );
  void deleting( int id, KURL from );
  void renaming( int id, KURL old_name, KURL new_name );
  void creatingDir( int id, KURL dir );

private:

  // Matt: call this when the user presses cancel
  // I suggest that the dialog stores both the appid and the
  // job's progress ID. If that's not possible, then I'll
  // put some dicts here... Well we need a dict id<->dialog box probably, anyway.
  void killJob( QCString observerAppId, int progressId );

  static int s_jobId;
};

// -*- mode: c++; c-basic-offset: 2 -*-
#endif
