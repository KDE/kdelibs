/* This file is part of the KDE libraries
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000-2001 Waldo Bastian <bastian@kde.org>

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

#include <kapplication.h>
#include <dcopobject.h>

class KUniqueApplicationPrivate;

/**
 * Maintains only a single
 * instance of a running application at a time.
 *
 * If another instance
 * is started, it will determine (via DCOP) whether it is the first instance
 * or a second instance.  If it is a second instance, it will forward on
 * the information to the first instance and then quit.
 *
 * @see KApplication DCOPObject
 * @author Preston Brown <pbrown@kde.org>
 */
class KUniqueApplication : public KApplication, public DCOPObject
{
  Q_OBJECT
public:
  /**
   * Constructor. Takes command line arguments from KCmdLineArgs
   * Parameters : See @ref KApplication constructor.
   * @p configUnique If true, the uniqueness of the application will
   *                 depend on the value of the "MultipleInstances"
   *                 key in the "KDE" group of the application config file.
   *
   */
  KUniqueApplication( bool allowStyles,
		      bool GUIenabled,
		      bool configUnique);
  // KDE 3.0: Remove me
  KUniqueApplication( bool allowStyles=true,
		      bool GUIenabled=true);

  /**
   * Adds command line options specific for KUniqueApplication.
   *
   * Should be called before calling KUniqueApplication constructor
   * and / or start().
   */
  static void addCmdLineOptions();

  /**
   * Forks and registers with dcop.
   *
   * The command line arguments are being sent via DCOP to @ref newInstance()
   * and will be received once the application enters the event loop.
   * @return @p true if registration is succesful.
   *         @p false if another process was already running.
   *
   * Typically this is used like:
   * <pre>
   * int main(int argc, char **argv) {
   *    KAboutData about("myappname", "myAppName", .....);
   *    KCmdLineArgs::init(argc, argv, &about);
   *    KCmdLineArgs::addCmdLineOptions( myCmdOptions );
   *    KUniqueApplication::addCmdLineOptions();
   *
   *    if (!KUniqueApplication::start()) {
   *       fprintf(stderr, "myAppName is already running!\n");
   *       exit(0);
   *    }
   *    KUniqueApplication a;
   *    a.exec();
   * }
   * </pre>
   * Note that it's not necessary to call @ref start() explicitly. It will be
   * called automatically before creating KUniqueApplication if it hasn't
   * been called yet, without any performance impact.
   */
  static bool start();

  /** Destructor */
  virtual ~KUniqueApplication();

  /**
   * Dispatches any incoming DCOP message for a new instance.
   *
   * If it is not a request for a new instance, return @p false.
   */
  bool process(const QCString &fun, const QByteArray &data,
	       QCString &replyType, QByteArray &replyData);

  /**
   * Creates a new "instance" of the application.
   *
   * Usually this will involve making some calls into the GUI portion of your
   * application asking for a new window to be created, possibly with
   * some data already loaded based on the arguments received.
   *
   * Command line arguments have been passed to KCmdLineArgs before this
   * function is called and can be checked in the usual way.
   * 
   * Note that newInstance() is called also in the first started 
   * application process.
   *
   * @return An exit value. The calling process will exit with this value.
   */
  virtual int newInstance();

private:
  /**
   * Delays the processing of a DCOP request.
   */
  void delayRequest(const QCString &fun, const QByteArray &data);

private slots:
  /**
   * Delayed processing of DCOP requests.
   */
  void processDelayed();

  void newInstanceNoFork();
  
  KInstance* initHack( bool configUnique );
 
private:
  static bool s_nofork;
  static bool s_multipleInstances;
  static bool s_uniqueTestDone;

  KUniqueApplicationPrivate *d;
};

#endif
