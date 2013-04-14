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
 * KDBusService takes care of registering the current process with DBus.
 *
 * This is typically done in the main() function.
 * An application can work in one of two modes:
 * <ul>
 *   <li>Multiple: the application can be launched many times. To allow this,
 *    the service name in the DBus registration contains the PID. Example:
 *      org.kde.konqueror-12345.
 *   <li>Unique: only one instance of this application can ever run. The application
 *   registers to DBus without PID, and any attempt to run the application again will
 *   call a DBus method called "Activate" in the running instance, and then quit.
 *   To implement this method, inherit from KDBusService and reimplement
 *       virtual int Activate();
 *   This slot usually raises the main window of the application.
 * </ul>
 *
 * Important: in order to avoid a race, the application should try to export its
 * objects to DBus before instanciating KDBusService.
 * Otherwise the application appears on the bus before its objects are accessible
 * via DBus, which could be a problem for other apps or scripts which start the
 * application in order to talk DBus to it immediately.
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
    //Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Application")

public:
    enum StartupOption {
        Unique = 1,
        Multiple = 2,
        NoExitOnFailure = 4
    };

    Q_DECLARE_FLAGS(StartupOptions, StartupOption)

    /**
     * Constructor. Registers the current process to DBus, using the given options.
     * The DBus service name is determined from the organization domain (reversed)
     * and the application name of the QApplication instance. For instance:
     * app.setApplicationName("kuiserver");
     * app.setOrganizationDomain("kde.org");
     * will make KDBusService register as org.kde.kuiserver.
     */
    explicit KDBusService(StartupOptions options = Multiple, QObject *parent = 0);

    /**
     * Destructor. Does not de-register from DBus. Could be added, though.
     *
     * KDBusService de-registers when the application is about to quit, though, to ensure
     * it doesn't receive calls anymore after that point.
     */
    ~KDBusService();

    /**
     * Returns true if the DBus registration succeeded.
     *
     * Note that this is only useful when specifying the option NoExitOnFailure.
     * Otherwise the simple fact that this process is still running, indicates
     * that the registration succeeded, since KDBusService quits on failure, by default.
     */
    bool isRegistered() const;

    /**
     * Returns the error message from the DBus registration, in case it failed.
     * Note that this is only useful when specifying the option NoExitOnFailure.
     * Otherwise the process has quit by the time you can get a chance to call this.
     */
    QString errorMessage() const;

Q_SIGNALS:
    /**
     * This method is called when a Unique application is launched while it is
     * already running. In single-windows applications, this method would typically
     * raise the window.
     * The default implementation does nothing.
     */
    void activateRequested();

    /**
     * Opens one or more files in the application.
     * @param uris The URLs of the files to open.
     */
    void openRequested(const QList<QUrl> &uris);

    /**
     * This method is called when a Unique application is launched while it is
     * already running, and the user selected a specific application action to trigger.
     * The default implementation does nothing.
     */
    void activateActionRequested(const QString &actionName, const QVariant& parameter);

public Q_SLOTS:
    /**
     * Unregister from DBus.
     * This is called automatically when the application is about to quit,
     * to make sure it doesn't keep receiving calls to Activate while it's
     * doing final cleanups.
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
