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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KUNIQUEAPP_H
#define KUNIQUEAPP_H

#include <kapplication.h>

/**
 * KUniqueApplication is a KApplication which only uses a single process.  When
 * a KUniqueApplication is started, it attempts to contact an existing copy 
 * of the application.  If successful, the program asks the 
 * existing process to create a new instance by calling its newInstance() method
 * and then exits.  If there is no existing process then the program forks and
 * calls the newInstance() method.  When newInstance() is called, the application
 * will typically create a new window or activate an existing one.
 *
 * Instances of KUniqueApplication can be made to behave like a normal application by passing
 * the StartFlag::NonUniqueInstance flag to start().
 *
 * Please note that this supports only one process per KDE session. If
 * your application can only be opened once per user or once per host, you
 * need to ensure this independently of KUniqueApplication.
 *
 * The .desktop file for the application should state X-DBUS-StartupType=Unique,
 * see ktoolinvocation.h
 *
 * If you use command line options before start() is called, you MUST call addCmdLineOptions()
 * so that the KUniqueApplication-specific command-line options can still work.
 *
 * If your application is used to open files, it should also support the --tempfile
 * option (see KCmdLineArgs::addTempFileOption()), to delete tempfiles after use.
 * Add X-KDE-HasTempFileOption=true to the .desktop file to indicate this.
 *
 * @see KApplication
 * @author Preston Brown <pbrown@kde.org>
 */
class KDEUI_EXPORT KUniqueApplication : public KApplication
{
  Q_OBJECT
public:
  /**
   * Constructor. Takes command line arguments from KCmdLineArgs
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   * @param configUnique If true, the uniqueness of the application will
   *                 depend on the value of the "MultipleInstances"
   *                 key in the "KDE" group of the application config file.
   */
  explicit KUniqueApplication( bool GUIenabled=true,
                               bool configUnique=false);

#ifdef Q_WS_X11
  /**
   * Constructor. Takes command line arguments from KCmdLineArgs
   *
   * @param display Will be passed to Qt as the X display. The display
   * must be valid and already opened.
   *
   * @param visual Pointer to the X11 visual that should be used by the
   * application. If NULL, the default visual will be used instead.
   *
   * @param colormap The colormap that should be used by the application.
   * If 0, the default colormap will be used instead.
   *
   * @param configUnique If true, the uniqueness of the application will
   *                 depend on the value of the "MultipleInstances"
   *                 key in the "KDE" group of the application config file.
   */
  explicit KUniqueApplication( Display *display,
                               Qt::HANDLE visual=0,
                               Qt::HANDLE colormap=0,
                               bool configUnique=false);
#endif

  /**
   * Adds command line options specific for KUniqueApplication.
   *
   * Should be called before calling KUniqueApplication constructor
   * and / or start().
   */
  static void addCmdLineOptions();

  /** 
   * These flags can be used to specify how new instances of 
   * unique applications are created.
   */
  enum StartFlag
  {
	  /** 
	   * Create a new instance of the application in a new process and 
	   * do not attempt to re-use an existing process.
	   * 
	   * With this flag set, the new instance of the application will 
	   * behave as if it were a plain KApplication rather than a KUniqueApplication.
	   *
	   * This is useful if you have an application where all instances are typically run
	   * in a single process but under certain circumstances new instances may require
	   * their own process.
	   */
  	  NonUniqueInstance = 0x1
  };
  Q_DECLARE_FLAGS(StartFlags,StartFlag)

  /**
   * Forks and registers with D-Bus.
   *
   * The command line arguments are being sent via D-Bus to newInstance()
   * and will be received once the application enters the event loop.
   *
   * Typically this is used like:
   * \code
   * int main(int argc, char **argv) {
   *    KAboutData about("myappname", 0, ki18n("myAppName"), .....);
   *    KCmdLineArgs::init(argc, argv, &about);
   *    KCmdLineArgs::addCmdLineOptions( myCmdOptions );
   *    KUniqueApplication::addCmdLineOptions();
   *
   *    if (!KUniqueApplication::start()) {
   *       fprintf(stderr, "myAppName is already running!\n");
   *       return 0;
   *    }
   *    KUniqueApplication a;
   *    return a.exec();
   * }
   * \endcode
   * Note that it's not necessary to call start() explicitly. It will be
   * called automatically before creating KUniqueApplication if it hasn't
   * been called yet, without any performance impact.
   *
   * Also note that you MUST call KUniqueApplication::addCmdLineOptions(),
   * if you use command line options before start() is called.
   *
   * @param flags 	Optional flags which control how a new instance 
   * 				of the application is started.
   * @return true if registration is successful.
   *         false if another process was already running.
   */
  static bool start(StartFlags flags);
  // BIC: merge with start(StartFlags flags = StartFlags()) 
  static bool start();

  /**
   * Destructor
   */
  virtual ~KUniqueApplication();

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
   * The default implementation ensures the mainwindow of the already
   * running instance is shown and activated if necessary. If your
   * application has only one mainwindow, you should call this default
   * implementation and only add your special handling if needed.
   *
   * Note that newInstance() is called also in the first started
   * application process.
   *
   * For applications that share one process for several mainwindows,
   * the reimplementation could be:
   * \code
    int MyApp::newInstance()
    {
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    static bool first = true;
    if (args->count() > 0) {
        for (int i = 0; i < args->count(); ++i) {
            openWindow(args->url(i));
        }
    } else if( !first || !isSessionRestored()) {
        openWindow(KUrl()); // create a new window
    }
    first = false;
    args->clear();
    return 0;
    }
   * \endcode
   *
   * @return An exit value. The calling process will exit with this value.
   */
  virtual int newInstance();

  /**
   * Returns whether newInstance() is being called while session
   * restoration is in progress.
   */
  bool restoringSession();

  /**
   * @internal
   */
  static void setHandleAutoStarted();

private:
  friend class KUniqueApplicationAdaptor;
  class Private;
  Private * const d;

  Q_PRIVATE_SLOT(d, void _k_newInstanceNoFork())
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KUniqueApplication::StartFlags)

#endif
