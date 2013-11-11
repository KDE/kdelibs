/* This file is part of libkdbusaddons

   Copyright (c) 2011 David Faure <faure@kde.org>
   Copyright (c) 2011 Kevin Ottens <ervin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDBUSSERVICE_H
#define KDBUSSERVICE_H

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <kdbusaddons_export.h>

class KDBusServicePrivate;

/**
 * KDBusService takes care of registering the current process with D-Bus.
 *
 * This registers the application at a predictable location on D-Bus, registers
 * the QCoreApplication (or subclass) object at /MainApplication, and
 * assists in implementing the application side of D-Bus activation from
 * the <a
 * href="http://standards.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html">Desktop
 * Entry Specification</a>.
 *
 * An application can either work in Multiple mode or Unique mode.
 *
 * In Multiple mode, the application can be launched many times.  The service
 * name in the D-Bus registration will contain the PID to distinguish the
 * various instances; for example: <tt>org.kde.konqueror-12345</tt>.
 *
 * In Unique mode, only one instance of this application can ever run.
 * The first instance of the application registers with D-Bus without the PID,
 * and any attempt to run the application again will call the D-Bus method
 * "Activate" in the running instance, and then quit.
 *
 * In order to allow for activation over D-Bus, applications should connect to
 * the signals emitted by this class.  In particular, the activateRequested()
 * signal should normally be connected to a slot that raises the main window of
 * the application.  This is particularly important for Unique-mode
 * applications, but the activation interface will also be exported to D-Bus for
 * Multiple-mode applications.
 *
 * @note In order to avoid a race, the application should export its objects to
 * D-Bus before allowing the event loop to run (for example, by calling
 * QCoreApplication::exec()).  Otherwise, the application will appear on the bus
 * before its objects are accessible via D-Bus, which could be a problem for
 * other applications or scripts which start the application in order to talk
 * D-Bus to it immediately.
 *
 * Example usage:
 *
 * <code>
     QApplication app(argc, argv);
     app.setApplicationName("kuiserver");
     app.setOrganizationDomain("kde.org");
     // Create your dbus objects here
     // ...
     KDBusService service(KDBusService::Unique);
     return app.exec();
 * </code>
 *
 * @since 5.0
 */
class KDBUSADDONS_EXPORT KDBusService : public QObject
{
    Q_OBJECT
    Q_ENUMS(StartupOption)
    Q_FLAGS(StartupOptions)

public:
    /** Options to control the behaviour of KDBusService */
    enum StartupOption {
        /** Indicates that only one instance of this application should ever
         * exist.
         *
         * Cannot be combined with @c Multiple.
         */
        Unique = 1,
        /** Indicates that multiple instances of the application may exist.
         *
         * Cannot be combined with @c Unique.  This is the default.
         */
        Multiple = 2,
        /** Indicates that the application should not exit if it failed to
         * register with D-Bus.
         *
         * By default, KDBusService will quit the application if it failed to
         * register the service with D-Bus (with error code @c 1).
         */
        NoExitOnFailure = 4
    };

    Q_DECLARE_FLAGS(StartupOptions, StartupOption)

    /**
     * Registers the current process to D-Bus at an address based on the
     * application name and organization domain.
     *
     * The DBus service name is the reversed organization domain, followed by
     * the application name.  If @p options includes the @c Multiple flag, the
     * application PID will be appended.  For example,
     * @code
     * app.setApplicationName("kuiserver");
     * app.setOrganizationDomain("kde.org");
     * @endcode
     * will make KDBusService register as @c org.kde.kuiserver in @c Unique
     * mode, and @c org.kde.kuiserver-1234 (if the process has PID @c 1234) in
     * @c Multiple mode.
     */
    explicit KDBusService(StartupOptions options = Multiple, QObject *parent = 0);

    /**
     * Destroys this object (but does not unregister the application).
     *
     * Deleting this object before unregister() is called (either manually or
     * because QCoreApplication::aboutToQuit() was emitted) could confuse
     * clients, who will see the service on the bus but will be unable to use
     * the activation methods.
     */
    ~KDBusService();

    /**
     * Returns true if the D-Bus registration succeeded.
     *
     * Note that this is only useful when specifying the option NoExitOnFailure.
     * Otherwise, the simple fact that this process is still running indicates
     * that the registration succeeded.
     */
    bool isRegistered() const;

    /**
     * Returns the error message from the D-Bus registration if it failed.
     *
     * Note that this is only useful when specifying the option NoExitOnFailure.
     * Otherwise the process has quit by the time you can get a chance to call this.
     */
    QString errorMessage() const;

Q_SIGNALS:
    /**
     * Signals that the application is to be activated.
     *
     * In a @c Unique application, this signal will typically be emitted in the
     * first instance of the application to be run when a second instance is
     * run.
     *
     * In single-window applications, the connected signal should typically
     * raise the window.
     */
    void activateRequested();

    /**
     * Signals that one or more files should be opened in the application.
     *
     * @param uris  The URLs of the files to open.
     */
    void openRequested(const QList<QUrl> &uris);

    /**
     * Signals that an application action should be triggered.
     *
     * See the desktop entry specification for more information.
     */
    void activateActionRequested(const QString &actionName, const QVariant& parameter);

public Q_SLOTS:
    /**
     * Unregister from D-Bus.
     *
     * This is called automatically when the application is about to quit, to
     * make sure it doesn't keep receiving calls to its D-Bus interface while it
     * is doing final cleanups.
     */
    void unregister();

private:
    // fdo.Application spec
    void Activate(const QVariantMap &platform_data);
    void Open(const QStringList &uris, const QVariantMap &platform_data);
    void ActivateAction(const QString &action_name, const QVariantList &maybeParameter, const QVariantMap &platform_data);
    friend class KDBusServiceAdaptor;

private:
    KDBusServicePrivate * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDBusService::StartupOptions)

#endif /* KDBUSSERVICE_H */
