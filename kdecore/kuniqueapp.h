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
   * Parameters : See KApplication constructor.
   */
  KUniqueApplication( int& argc, char** argv,
		      const QCString& rAppName = 0, 
		      bool allowStyles=true, 
		      bool GUIenabled=true);

  /**
   * Fork and register with dcop.
   * The command line arguments are being sent via DCOP to newInstance()
   * and will be received once the application enters the event loop.
   * @return True if registration is succesfull
   *         False if another process was already running.
   *
   * Typically this is used like:
   *
   * int main(int argc, char **argv) {
   *    if (!KUniqueApplication::start(argc, argv, "myAppName")) {
   *       fprintf(stderr, "myAppName is already running!\n");
   *       exit(0);
   *    }
   *    KUniqueApplication a(argc, argv, "myAppName");
   *    a.exec();
   * }
   *
   * or
   *
   * int main(int argc, char **argv) {
   *    if (!KUniqueApplication::start(argc, argv, "myAppName"))
   *       exit(0);
   *    KUniqueApplication a(argc, argv, "myAppName");
   *    a.exec();
   * }
   * 
   * Although it is not necassery to call start() before creating a
   * KUniqueApplication it is adviced to so because it is about
   * 40% faster if the application was already running: 
   * If you use start() the KApplication constructor will not be 
   * called if this isn't necassery.
   */
  static bool start(int& argc, char** argv, const QCString &rAppName);
   
  
  /** Destructor */
  virtual ~KUniqueApplication();

  virtual DCOPClient *dcopClient();
  
  /** 
   * dispatch any incoming DCOP message for a new instance.  If
   * it is not a request for a new instance, return false.
   */
  bool process(const QCString &fun, const QByteArray &data,
	       QCString &replyType, QByteArray &replyData);

  /**
   * create a new "instance" of the application.  Usually this
   * will involve making some calls into the GUI portion of your
   * application asking for a new window to be created, possibly with
   * some data already loaded based on the arguments received.
   *
   * @param params is the bundled up command line parameters that were passed
   *               on the command line when the application request was initiated,
   *               _after_ being processed by Qt's QApplication.
   *
   * @return An exit value. The calling process will exit with this value.
   */
  virtual int newInstance(QValueList<QCString> params);
private:
  static DCOPClient *s_DCOPClient;
};

#endif
