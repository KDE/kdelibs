/* This file is part of the KDE libraries
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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

#ifndef _KUNIQUEAPP_H
#define _KUNIQUEAPP_H

#include <kapp.h>
#include <dcopobject.h>

/**
 * KUniqueApplication provides a way of maintaining only a single
 * instance of a running application at a time.  If another instance
 * is started, it will determine (via DCOP) whether it is the first instance
 * or a second instance.  If it is a second instance, it will forward on
 * the information to the first instance and then quit.
 *
 * @see KApplication, DCOPObject
 * @author Preston Brown <pbrown@kde.org>
 */
class KUniqueApplication : public KApplication, DCOPObject
{
  Q_OBJECT
public:
  /**
   * Constructor. Parses command-line arguments.
   *
   */
  KUniqueApplication( int& argc, char** argv,
		      const QCString& rAppName = 0);
  
  /** Destructor */
  virtual ~KUniqueApplication();
  
  /** 
   * dispatch any incoming DCOP message for a new instance.  If
   * it is not a request for a new instance, return false.
   */
  bool process(const QCString &fun, const QByteArray &data,
	       QCString &replyType, QByteArray &reply);

  /**
   * create a new "instance" of the application.  Usually this
   * will involve making some calls into the GUI portion of your
   * application asking for a new window to be created, possibly with
   * some data already loaded based on the arguments received.
   *
   * @params is the bundled up command line parameters that were passed
   *          on the command line when the application request was initiated,
   *          _after_ being processed by Qt's QApplication.
   */
  virtual void newInstance(QValueList<QCString> params);
};

#endif
