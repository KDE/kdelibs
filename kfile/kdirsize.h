/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef __KDIRSIZE_H
#define __KDIRSIZE_H

#include <kio/job.h>

/**
 * Compute directory size (similar to "du", but doesn't give the same results
 * since we simply sum up the dir and file sizes, whereas du speaks disk blocks)
 */
class KDirSize : public KIO::Job
{
  Q_OBJECT
protected:
  KDirSize( const KURL & directory );
  ~KDirSize() {}

public:
  /**
   * @return the size we found
   */
  unsigned long totalSize() { return m_totalSize; }

  /**
   * Asynchronous method. Connect to the result signal.
   */
  static KDirSize * dirSizeJob( const KURL & directory );

  /**
   * Synchronous method - you get the result as soon as
   * the call returns.
   */
  static unsigned long dirSize( const KURL & directory );

protected:
  /**
   * @internal
   */
  void setSync() { m_bAsync = false; }

protected slots:

 virtual void slotResult( KIO::Job *job );
 void slotEntries( KIO::Job * , const KIO::UDSEntryList & );

private:
  bool m_bAsync;
  unsigned long m_totalSize;
};

#endif
