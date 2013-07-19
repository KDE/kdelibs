/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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

#ifndef KAPP_H
#define KAPP_H

// Version macros. Never put this further down.
#include <kde4support_export.h>

class KConfig;

#ifdef KDE3_SUPPORT
#include <krandom.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <QPixmap>
#include <QIcon>
#endif

typedef unsigned long Atom;
#include <qplatformdefs.h>

#include <QApplication>
#include <kcomponentdata.h>

struct _IceConn;
class QPixmap;

#define kapp KApplication::kApplication()

class KApplicationPrivate;

/**
* Controls and provides information to all KDE applications.
*
* Only one object of this class can be instantiated in a single app.
* This instance is always accessible via the 'kapp' global variable.
*
* This class provides the following services to all KDE applications.
*
* @li It controls the event queue (see QApplication ).
* @li It provides the application with KDE resources such as
* accelerators, common menu entries, a KConfig object. session
* management events, help invocation etc.
* @li Installs an empty signal handler for the SIGPIPE signal.
* If you want to catch this signal
* yourself, you have set a new signal handler after KApplication's
* constructor has run.
* @li It can start new services
*
*
* @short Controls and provides information to all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
*/
class KDE4SUPPORT_DEPRECATED_EXPORT KApplication : public QApplication
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KApplication")
public:
  /**
   * This constructor is the one you should use.
   * It takes aboutData and command line arguments from KCmdLineArgs.
   *
   * @param GUIenabled Set to false to disable all GUI stuff.
   * Note that for a non-GUI daemon, you might want to use QCoreApplication
   * and a KComponentData instance instead. You'll save an unnecessary dependency
   * to kdeui. The main difference is that you will have to do a number of things yourself:
   * <ul>
   *  <li>Register to DBus, if necessary.</li>
   *  <li>Call KLocale::global(), if using multiple threads.</li>
   * </ul>
   */
  explicit KApplication(bool GUIenabled = true);

  virtual ~KApplication();

  /**
   * Returns the current application object.
   *
   * This is similar to the global QApplication pointer qApp. It
   * allows access to the single global KApplication object, since
   * more than one cannot be created in the same application. It
   * saves you the trouble of having to pass the pointer explicitly
   * to every function that may require it.
   * @return the current application object
   */
  static KApplication* kApplication();

  /**
   * @deprecated
   * Returns the application session config object.
   *
   * @return A pointer to the application's instance specific
   * KConfig object.
   * @see KConfigGui::sessionConfig
   */
  KConfig* sessionConfig();

#ifdef KDE3_SUPPORT
 /**
   * Is the application restored from the session manager?
   *
   * @return If true, this application was restored by the session manager.
   *    Note that this may mean the config object returned by
   * sessionConfig() contains data saved by a session closedown.
   * @see sessionConfig()
   * @deprecated use qApp->isSessionRestored()
   */
  inline KDE4SUPPORT_DEPRECATED bool isRestored() const { return QApplication::isSessionRestored(); }
#endif

  /**
   * Disables session management for this application.
   *
   * Useful in case  your application is started by the
   * initial "startkde" script.
   */
  void disableSessionManagement();

  /**
   * Enables session management for this application, formerly
   * disabled by calling disableSessionManagement(). You usually
   * shouldn't call this function, as session management is enabled
   * by default.
   */
  void enableSessionManagement();

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManager.
     * @internal
     */
  void commitData( QSessionManager& sm );

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManager.
     * @internal
     */
  void saveState( QSessionManager& sm );

  /**
   * @deprecated since 5.0, use QGuiApplication::isSavingSession()
   *
   * Returns true if the application is currently saving its session
   * data (most probably before KDE logout). This is intended for use
   * mainly in KMainWindow::queryClose() and KMainWindow::queryExit().
   *
   * @see KMainWindow::queryClose
   * @see KMainWindow::queryExit
   */
  bool sessionSaving() const;

#ifdef KDE3_SUPPORT
  /**
   * Returns a QPixmap with the application icon.
   * @return the application icon
   * @deprecated Use QApplication::windowIcon()
   */
  inline KDE4SUPPORT_DEPRECATED QPixmap icon() const {
      int size = IconSize(KIconLoader::Desktop);
      return windowIcon().pixmap(size,size);
  }

  /**
   * Returns the mini-icon for the application as a QPixmap.
   * @return the application's mini icon
   * @deprecated Use QApplication::windowIcon()
   */
  inline KDE4SUPPORT_DEPRECATED QPixmap miniIcon() const {
      int size = IconSize(KIconLoader::Small);
      return windowIcon().pixmap(size,size);
  }
#endif

  /**
   *  Sets the top widget of the application.
   *  This means basically applying the right window caption.
   *  An application may have several top widgets. You don't
   *  need to call this function manually when using KMainWindow.
   *
   *  @param topWidget A top widget of the application.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( QWidget *topWidget );

  /**
   * Get a file name in order to make a temporary copy of your document.
   *
   * @param pFilename The full path to the current file of your
   * document.
   * @return A new filename for auto-saving.
   * @deprecated use QTemporaryFile, QSaveFile or KAutoSaveFile instead
   */
#ifndef KDE_NO_DEPRECATED
  static KDE4SUPPORT_DEPRECATED QString tempSaveName( const QString& pFilename );
#endif

  /**
   * Check whether  an auto-save file exists for the document you want to
   * open.
   *
   * @param pFilename The full path to the document you want to open.
   * @param bRecover  This gets set to true if there was a recover
   * file.
   * @return The full path of the file to open.
   */
  static QString checkRecoverFile( const QString& pFilename, bool& bRecover );

  /**
   * @deprecated since 5.0, use QCoreApplication::installNativeEventFilter
   *  Installs widget filter as global X11 event filter.
   *
   * The widget
   *  filter receives XEvents in its standard QWidget::x11Event() function.
   *
   *  Warning: Only do this when absolutely necessary. An installed X11 filter
   *  can slow things down.
   */
  void installX11EventFilter( QWidget* filter );

  /**
   * @deprecated since 5.0, use QCoreApplication::removeNativeEventFilter
   * Removes global X11 event filter previously installed by
   * installX11EventFilter().
   */
  void removeX11EventFilter( const QWidget* filter );

#ifdef KDE3_SUPPORT
  /**
   * Generates a uniform random number.
   * @return A truly unpredictable number in the range [0, RAND_MAX)
   * @deprecated Use KRandom::random()
   */
  static inline KDE4SUPPORT_DEPRECATED int random() { return KRandom::random(); }

  /**
   * Generates a random string.  It operates in the range [A-Za-z0-9]
   * @param length Generate a string of this length.
   * @return the random string
   * @deprecated use KRandom::randomString() instead.
   */
  static inline KDE4SUPPORT_DEPRECATED QString randomString(int length) { return KRandom::randomString(length); }
#endif

  /**
   * @deprecated
   * Returns the app startup notification identifier for this running
   * application.
   * @return the startup notification identifier
   * @see KStartupInfo::startupId
   */
  QByteArray startupId() const;

  /**
   * @internal
   * @deprecated
   * Sets a new value for the application startup notification window property for newly
   * created toplevel windows.
   * @param startup_id the startup notification identifier
   * @see KStartupInfo::setStartupId
   * @see KStartupInfo::setNewStartupId
   */
  void setStartupId( const QByteArray& startup_id );
  /**
   * @internal
   * Used only by KStartupId.
   */
  void clearStartupId();

  /**
   * @deprecated
   * Returns the last user action timestamp or 0 if no user activity has taken place yet.
   * @see updateuserTimestamp
   * @see KUserTimestamp::userTimestamp
   */
  unsigned long userTimestamp() const;

  /**
   * Updates the last user action timestamp in the application registered to DBUS with id service
   * to the given time, or to this application's user time, if 0 is given.
   * Use before causing user interaction in the remote application, e.g. invoking a dialog
   * in the application using a DCOP call.
   * Consult focus stealing prevention section in kdebase/kwin/README.
   */
  void updateRemoteUserTimestamp( const QString& service, int time = 0 );

#ifdef KDE3_SUPPORT
    /**
    * Returns the argument to --geometry if any, so the geometry can be set
    * wherever necessary
    * @return the geometry argument, or QString() if there is none
    * @deprecated please use the following code instead:
    *
    * <code>
    * QString geometry;
    * KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    * if (args && args->isSet("geometry"))
    *     geometry = args->getOption("geometry");
    *
    * </code>
    */
  static inline KDE4SUPPORT_DEPRECATED QString geometryArgument() {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    return args->isSet("geometry") ? args->getOption("geometry") : QString();
  }
#endif

  /**
      @internal
    */
  bool notify( QObject* receiver, QEvent* event );

public Q_SLOTS:
  /**
   * @deprecated
   * Updates the last user action timestamp to the given time, or to the current time,
   * if 0 is given. Do not use unless you're really sure what you're doing.
   * Consult focus stealing prevention section in kdebase/kwin/README.
   * @see KUserTimestamp::updateUserTimestamp
   */
  Q_SCRIPTABLE void updateUserTimestamp( int time = 0 );

  // D-Bus Q_SLOTS:
  Q_SCRIPTABLE void reparseConfiguration();
  Q_SCRIPTABLE void quit();

Q_SIGNALS:
  /**
     @deprecated since 5.0, connect to saveStateRequest instead

     Session management asks you to save the state of your application.

     This signal is provided for compatibility only. For new
     applications, simply use KMainWindow. By reimplementing
     KMainWindow::queryClose(), KMainWindow::saveProperties() and
     KMainWindow::readProperties() you can simply handle session
     management for applications with multiple toplevel windows.

     For purposes without KMainWindow, create an instance of
     KSessionManager and reimplement the functions
     KSessionManager::commitData() and/or
     KSessionManager::saveState()

     If you still want to use this signal, here is what you should do:

     Connect to this signal in order to save your data. Do NOT
     manipulate the UI in that slot, it is blocked by the session
     manager.

     Use the sessionConfig() KConfig object to store all your
     instance specific data.

     Do not do any closing at this point! The user may still select
     Cancel  wanting to continue working with your
     application. Cleanups could be done after aboutToQuit().
  */
  void saveYourself();

protected:
  /**
   * @internal Used by KUniqueApplication
   */
  KApplication(bool GUIenabled, const KComponentData &cData);

  /// Current application object.
  static KApplication *KApp;

private:
  KApplication(const KApplication&);
  KApplication& operator=(const KApplication&);

private:
  // This is to catch invalid implicit conversions
  KApplication(bool, bool);

  friend class KApplicationPrivate;
  KApplicationPrivate* const d;

  Q_PRIVATE_SLOT(d, void _k_x11FilterDestroyed())
  Q_PRIVATE_SLOT(d, void _k_checkAppStartedSlot())
  Q_PRIVATE_SLOT(d, void _k_slot_KToolInvocation_hook(QStringList&, QByteArray&))
};

#endif

