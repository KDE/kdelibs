/*
   $Id$

   KIO DNS - asynchronous DNS lookup class
 
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown and Red Hat, Inc.
 
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

#ifndef __KIO_DNS_H
#define __KIO_DNS_H

#include <qobject.h>

class KIODNS : public QObject
{
  Q_OBJECT

 public:
  /**
   * Creates a new domain name service lookup object.  
   * 
   * @param serverCount is the number of servers to fork at startup.  If
   * unspecified or <= 0, they will be dynamically allocated.  If you
   * are concerned about virtual memory, create your KIODNS object
   * near the beginning of your program.
   *
   * Usage will usually be as follows:
   * <pre>
   *   KIODNS *dns = new KIODNS;
   *   connect (dns, SIGNAL(lookupDone(QString &)), 
   *            SLOT(processHostName(QString &)));
   *   dns->lookup("127.0.0.1"); 
   * </pre>
   */
  KIODNS(int serverCount = 0);

  /**
   * Destroys the KIODNS object, stopping any pending lookups and killing
   * all forked processes.
   */
  virtual ~KIODNS();

  /** 
   * starts a lookup.  You must connect to the signal lookupDone() in
   * order to receive lookup results.  If all current DNS servers are
   * in use, a new server may be spawned.
   *
   * @return a tag that can be used as an argument to abort().  
   */
  unsigned int lookup(QString &hostnum);

  /** 
   * abort any lookup that may be in progress (insure that the lookupDone()
   * signal will not be sent. 
   * 
   * @param tag the tag received when lookup() was called for this
   * particular query.  
   */
  void abort(unsigned int tag);

 signals:
  /** 
   * This signal is emitted when the lookup is complete.
   */
  void lookupDone(const QString &hostname);

 private:
  int sc;
};

#endif
