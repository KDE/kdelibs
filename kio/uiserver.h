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

class KURL;

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
   * @return the job id
   */
  int newJob( /* param: type of the job ? class name ? */ );

  // ListJob
  void totalEntries( int id, unsigned long total );
  void entries( int id, unsigned long count );

private:
  static int s_jobId;
};

// -*- mode: c++; c-basic-offset: 2 -*-
#endif
