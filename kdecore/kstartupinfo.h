/****************************************************************************

 $Id$

 Copyright (C) 2001 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#ifndef __KSTARTUPINFO_H
#define __KSTARTUPINFO_H

#include <sys/types.h>
#include <qobject.h>
#ifdef Q_WS_X11 // FIXME(E): Redo in a less X11-specific way
#include <qcstring.h>
#include <qstring.h>
#include <qvaluelist.h>

class KStartupInfoId;
class KStartupInfoData;

class KStartupInfoPrivate;

/**
 * Class for manipulating the application startup notification.
 *
 * This class can be used to send information about started application,
 * change the information and receive this information. For detailed
 * description, see kdelibs/kdecore/README.kstartupinfo.
 *
 * You usually don't need to use this class for sending the notification
 * information, as KDE libraries should do this when an application is
 * started ( e.g. KRun class ).
 *
 * For receiving the startup notification info, create an instance and connect
 * to its slots. It will automatically detect started applications and when
 * they are ready.
 *
 * @see KStartupInfoId
 * @see KStartupInfoData
 *
 * @author Lubos Lunak <l.lunak@kde.org>
 * @version $Id$
 */
class KStartupInfo
    : public QObject
    {
    Q_OBJECT
    public:
	/**
	 * Creates an instance that will receive the startup notifications.
	 *
	 * @param clean_on_cantdetect if true, and a new unknown window appears,
	 *  removes all notification for applications that are not compliant
	 *  with the app startup protocol
	 * @param parent the parent of this QObject (can be 0 for no parent)
	 * @param name the name of the QObject (can be 0 for no name)
	 */
        KStartupInfo( bool clean_on_cantdetect, QObject* parent = 0, const char* name = 0 );
        virtual ~KStartupInfo();
	/**
	 * Sends given notification data about started application
	 * with the given startup identification. If no notification for this identification
	 * exists yet, it is created, otherwise it's updated.
	 * 
	 * @param id the id of the application
	 * @param data the application's data
	 * @return true if successful, false otherwise
	 * @see KStartupInfoId
	 * @see KStartupInfoData
	 */
        static bool sendStartup( const KStartupInfoId& id, const KStartupInfoData& data );

	/**
	 * Like @ref sendStartup , uses dpy instead of qt_x11display() for sending the info.
	 * @param dpy the display of the application.
	 * @param id the id of the application
	 * @param data the application's data
	 * @return true if successful, false otherwise
	 */
        static bool sendStartupX( Display* dpy, const KStartupInfoId& id,
            const KStartupInfoData& data );

	/**
	 * Sends given notification data about started application
	 * with the given startup identification. This is used for updating the notification
	 * info, if no notification for this identification exists, it's ignored.
	 * @param id the id of the application
	 * @param data the application's data
	 * @return true if successful, false otherwise
	 * @see KStartupInfoId
	 * @see KStartupInfoData
	 */
        static bool sendChange( const KStartupInfoId& id, const KStartupInfoData& data );

	/**
	 * Like @ref sendChange , uses dpy instead of qt_x11display() for sending the info.
	 * @param dpy the display of the application.
	 * @param id the id of the application
	 * @param data the application's data
	 * @return true if successful, false otherwise
	 */
        static bool sendChangeX( Display* dpy, const KStartupInfoId& id,
            const KStartupInfoData& data );

	/**
	 * Ends startup notification with the given identification.
	 * @param id the id of the application
	 * @return true if successful, false otherwise
	 */
        static bool sendFinish( const KStartupInfoId& id );

	/**
	 * Like @ref sendFinish , uses dpy instead of qt_x11display() for sending the info.
	 * @param dpy the display of the application.
	 * @param id the id of the application
	 * @return true if successful, false otherwise
	 */
        static bool sendFinishX( Display* dpy, const KStartupInfoId& id );

	/**
	 * Ends startup notification with the given identification and the given data ( e.g.
	 * PIDs of processes for this startup notification that exited ).
	 * @param id the id of the application
	 * @param data the application's data
	 * @return true if successful, false otherwise
	 */
        static bool sendFinish( const KStartupInfoId& id, const KStartupInfoData& data );

	/**
	 * Like @ref sendFinish , uses dpy instead of qt_x11display() for sending the info.
	 * @param dpy the display of the application.
	 * @param id the id of the application
	 * @param data the application's data
	 * @return true if successful, false otherwise
	 */
        static bool sendFinishX( Display* dpy, const KStartupInfoId& id,
            const KStartupInfoData& data );

	/**
	 * Returns the current startup notification identification for the current
	 * startup notification environment variable. Note that KApplication constructor
	 * unsets the variable and you have to use @ref KApplication::startupId .
	 * @return the current startup notification identification
	 */
        static KStartupInfoId currentStartupIdEnv();
	/**
	 * Unsets the startup notification environment variable.
	 */
        static void resetStartupEnv();
        /**
         * Manual notification that the application has started.
         * If you do not map a (toplevel) window, then startup
         * notification will not disappear for the application
         * until a timeout. You can use this as an alternative
         * method in this case.
         */
        static void appStarted();
	/**
	 * @li NoMatch    - the window doesn't match any existing startup notification</li>
	 * @li Match      - the window matches an existing startup notification</li>
	 * @li CantDetect - unable to detect if the window matches any existing
	 *			startup notification</li>
	 */
        enum startup_t { NoMatch, Match, CantDetect };
	/**
	 * Checks if the given windows matches any existing startup notification. If yes,
	 * the startup notification is removed.
	 * @param w the window id to check
	 * @return the result of the operation
	 */
        startup_t checkStartup( WId w );
	/**
	 * Checks if the given windows matches any existing startup notification, and
	 * if yes, returns the identification in id, and removes the startup notification.
	 * @param w the window id to check
	 * @param id if found, the id of the startup notification will be written here
	 * @return the result of the operation
	 */
        startup_t checkStartup( WId w, KStartupInfoId& id );
	/**
	 * Checks if the given windows matches any existing startup notification, and
	 * if yes, returns the notification data in data, and removes the startup
	 * notification.
	 * @param w the window id to check
	 * @param data if found, the data of the startup notification will be written here
	 * @return the result of the operation
	 */
        startup_t checkStartup( WId w, KStartupInfoData& data );
	/**
	 * Checks if the given windows matches any existing startup notification, and
	 * if yes, returns the identification in id and notification data in data,
	 * and removes the startup notification.
	 * @param w the window id to check
	 * @param id if found, the id of the startup notification will be written here
	 * @param data if found, the data of the startup notification will be written here
	 * @return the result of the operation
	 */
        startup_t checkStartup( WId w, KStartupInfoId& id, KStartupInfoData& data );
	/**
	 * Sets the timeout for notifications, after this timeout a notification is removed.
	 * @param secs the new timeout in seconds
	 */
        void setTimeout( unsigned int secs );
	/**
	 * Sets the startup notification window property on the given window.
	 * @param windows the id of the window
	 * @param id the startup notification id
	 */
        static void setWindowStartupId( WId window, const QCString& id );
	/**
	 * Returns startup notification identification of the given window.
	 * @param windows the id of the window
	 * @return the startup notification id. Can be null if not found.
	 */
        static QCString windowStartupId( WId w );
	/**
	 * @internal
	 */
        class Data;
    signals:
	/**
	 * Emitted when a new startup notification is created ( i.e. a new application is
	 * being started ).
	 * @param id the notification identification
	 * @param data the notification data
	 */
        void gotNewStartup( const KStartupInfoId& id, const KStartupInfoData& data );
	/**
	 * Emitted when a startup notification changes.
	 * @param id the notification identification
	 * @param data the notification data
	 */
        void gotStartupChange( const KStartupInfoId& id, const KStartupInfoData& data );
	/**
	 * Emitted when a startup notification is removed ( either because it was detected
	 * that the application is ready or because of a timeout ).
	 * @param id the notification identification
	 * @param data the notification data
	 */
        void gotRemoveStartup( const KStartupInfoId& id, const KStartupInfoData& data );
    protected:
	virtual void customEvent( QCustomEvent* e_P );
    private slots:
        void startups_cleanup();
        void startups_cleanup_no_age();
        void got_message( const QString& msg );
        void window_added( WId w );
	void slot_window_added( WId w );
    private:
        friend class KStartupInfoPrivate;
        void got_startup_info( const QString& msg_P, bool update_only_P );
        void got_remove_startup_info( const QString& msg_P );
        void new_startup_info_internal( const KStartupInfoId& id_P,
            Data& data_P, bool update_only_P );
        void remove_startup_info_internal( const KStartupInfoId& id_P );
        void remove_startup_pids( const KStartupInfoId& id, const KStartupInfoData& data );
        void remove_startup_pids( const KStartupInfoData& data );
        startup_t check_startup_internal( WId w, KStartupInfoId* id, KStartupInfoData* data,
            bool remove );
        bool find_id( const QCString& id_P, KStartupInfoId* id_O,
            KStartupInfoData* data_O, bool remove );
        bool find_pid( pid_t pid_P, const QCString& hostname, KStartupInfoId* id_O,
            KStartupInfoData* data_O, bool remove );
        bool find_wclass( QCString res_name_P, QCString res_class_P,
            KStartupInfoId* id_O, KStartupInfoData* data_O, bool remove );
        static QCString get_window_startup_id( WId w_P );
        static QCString get_window_hostname( WId w_P );
        void startups_cleanup_internal( bool age_P );
        void clean_all_noncompliant();
        bool clean_on_cantdetect;
        unsigned int timeout;
        KStartupInfoPrivate* d;
    };

class KStartupInfoIdPrivate;

/**
 * Class representing an identification of application startup notification.
 *
 * Every existing notification about a starting application has its own unique
 * identification, that's used to identify and manipulate the notification.
 *
 * @see KStartupInfo
 * @see KStartupInfoData
 *
 * @author Lubos Lunak <l.lunak@kde.org>
 * @version $Id$
 */
class KStartupInfoId
    {
    public:
	/**
	 * Overloaded operator.
	 * @return true if the notification identifications are the same
	 */
        bool operator==( const KStartupInfoId& id ) const;
	/**
	 * Overloaded operator.
	 * @return true if the notification identifications are different
	 */
        bool operator!=( const KStartupInfoId& id ) const;
	/**
	 * Checks whether the identifier is valid.
	 * @return true if this object doesn't represent a valid notification identification
	 */
        bool none() const;
	/**
	 * Initializes this object with the given identification ( which may be also "0"
	 * for no notification ), or if "" is given, tries to read it from the startup
	 * notification environment variable, and if it's not set, creates a new one.
	 * @param id the new identification, "0" for no notification or "" to read
	 *           the environment variable
	 */
        void initId( const QCString& id = "" );
	/**
	 * Returns the notification identifier as string.
	 * @return the identification string for the notification
	 */
        const QCString& id() const;
	/**
	 * Sets the startup notification environment variable to this identification.
	 * @return true if successful, false otherwise
	 */
        bool setupStartupEnv() const;
	/**
	 * Creates an empty identification
	 */
        KStartupInfoId();
	/**
	 * Copy constructor.
	 */
        KStartupInfoId( const KStartupInfoId& data );
        ~KStartupInfoId();
        KStartupInfoId& operator=( const KStartupInfoId& data );
        bool operator<( const KStartupInfoId& id ) const;
    private:
        KStartupInfoId( const QString& txt );
        QString to_text() const;
        friend class KStartupInfo;
        KStartupInfoIdPrivate* d;
    };

class KStartupInfoDataPrivate;

/**
 * Class representing data about an application startup notification.
 *
 * Such data include the icon of the starting application, the desktop on which
 * the application should start, the binary name of the application, etc.
 *
 * @see KStartupInfo
 * @see KStartupInfoId
 *
 * @author Lubos Lunak <l.lunak@kde.org>
 * @version $Id$
 */
class KStartupInfoData
    {
    public:
	/**
	 * Sets the binary name of the application ( e.g. 'kcontrol' ).
	 * @param bin the new binary name of the application
	 */
        void setBin( const QString& bin );
	/**
	 * Returns the binary name of the starting application
	 * @return the new binary name of the application
	 */
        const QString& bin() const;
	/**
	 * Sets the name for the notification ( e.g. 'Control Center' )
	 */
        void setName( const QString& name );
	/**
	 * Returns the name of the startup notification, or if it's not available,
	 * the binary name.
	 * @return the name of the startup notification, or the binary name
	 *         if not available
	 */
        const QString& findName() const;
	/**
	 * Returns the name of the startup notification, or empty if not available.
	 * @return the name of the startup notificaiton, or an empty string
	 *         if not set.
	 */
        const QString& name() const;
	/**
	 * Sets the icon for the startup notification ( e.g. 'kcontrol' )
	 * @param icon the name of the icon
	 */
        void setIcon( const QString& icon );
	/**
	 * Returns the icon of the startup notification, and if it's not available,
	 * tries to get it from the binary name.
	 * @return the name of the startup notification's icon, or the name of
	 *         the binary if not set
	 */
        const QString& findIcon() const;
	/**
	 * Returns the icon of the startup notification, or empty if not available.
	 * @return the name of the icon, or an empty string if not set.
	 */
        const QString& icon() const;
	/**
	 * Sets the desktop for the startup notification ( i.e. the desktop on which
	 * the starting application should appear ).
	 * @param desktop the desktop for the startup notification
	 */
        void setDesktop( int desktop );
	/**
	 * Returns the desktop for the startup notification.
	 * @return the desktop for the startup notification
	 */
        int desktop() const;
	/**
	 * Sets a WM_CLASS value for the startup notification, it may be used for increasing
	 * the chance that the windows created by the starting application will be
	 * detected correctly.
	 * @param wmclass the WM_CLASS value for the startup notification
	 */
        void setWMClass( const QCString& wmclass );
	/**
	 * Returns the WM_CLASS value for the startup notification, or binary name if not
	 * available.
	 * @return the WM_CLASS value for the startup notification, or the binary name
	 *         if not set
	 */
        const QCString findWMClass() const;
	/**
	 * Returns the WM_CLASS value for the startup notification, or empty if not available.
	 * @return the WM_CLASS value for the startup notification, or empty
	 *         if not set
	 */
        const QCString& WMClass() const;
	/**
	 * Adds a PID to the list of processes that belong to the startup notification. It
	 * may be used to increase the chance that the windows created by the starting
	 * application will be detected correctly, and also for detecting if the application
	 * has quit without creating any window.
	 * @param pid the PID to add
	 */
        void addPid( pid_t pid );
	/**
	 * Returns all PIDs for the startup notification.
	 * @return the list of all PIDs
	 */
        const QValueList< pid_t >& pids() const;
	/**
	 * Checks whether the given @p pid is in the list of PIDs for starup
	 * notification.
	 * @return true if the given @p pid is in the list of PIDs for the startup notification
	 */
        bool is_pid( pid_t pid ) const;
	/**
	 * Sets the hostname on which the application is starting. It's necessary to set
	 * it if PIDs are set.
	 * @param hostname the application's hostname. If it's a null string, the current hostname is used
	 */
        void setHostname( const QCString& hostname = QCString());
	/**
	 * Returns the hostname for the startup notification.
	 * @return the hostname
	 */
        const QCString& hostname() const;
	/**
	 * Updates the notification data from the given data. Some data, such as the desktop
	 * or the name, won't be rewritten if already set.
	 * @param data the data t update
	 */
        void update( const KStartupInfoData& data );
	/**
	 * Constructor. Initializes all the data to their default empty values.
	 */
        KStartupInfoData();

	/**
	 * Copy constructor.
	 */
        KStartupInfoData( const KStartupInfoData& data );
        ~KStartupInfoData();
        KStartupInfoData& operator=( const KStartupInfoData& data );
    private:
        KStartupInfoData( const QString& txt );
        QString to_text() const;
        void remove_pid( pid_t pid );
        friend class KStartupInfo;
        friend class KStartupInfo::Data;
        KStartupInfoDataPrivate* d;
    };

#endif
#endif
