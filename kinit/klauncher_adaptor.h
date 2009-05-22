/*
 * Copyright 2006, 2007 Thiago Macieira <thiago@kde.org>
 * Copyright 2006-2008 David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KLAUNCHER_ADAPTOR_H_18181148166088
#define KLAUNCHER_ADAPTOR_H_18181148166088

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;

/*
 * Adaptor class for interface org.kde.KLauncher
 */
class KLauncherAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KLauncher")
public:
    KLauncherAdaptor(QObject *parent);
    virtual ~KLauncherAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void autoStart(int phase = 1);
    void exec_blind(const QString &name, const QStringList &arg_list);
    void exec_blind(const QString &name, const QStringList &arg_list, const QStringList &envs, const QString &startup_id);
    int kdeinit_exec(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id, const QDBusMessage &msg, QString &dbusServiceName, QString &error, int &pid);
    int kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id, const QDBusMessage &msg, QString &dbusServiceName, QString &error, int &pid);
    int kdeinit_exec_with_workdir(const QString &app, const QStringList &args, const QString& workdir, const QStringList &env, const QString& startup_id, const QDBusMessage &msg, QString &dbusServiceName, QString &error, int &pid);
    void reparseConfiguration();
    int requestHoldSlave(const QString &url, const QString &app_socket);
    int requestSlave(const QString &protocol, const QString &host, const QString &app_socket, QString &error);
    void setLaunchEnv(const QString &name, const QString &value);
    int start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg, QString &dbusServiceName, QString &error, int &pid);
    int start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg, QString &dbusServiceName, QString &error, int &pid);
    // Deprecated, to be removed in KDE5.
    int start_service_by_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg, QString &dbusServiceName, QString &error, int &pid);
    void waitForSlave(int pid, const QDBusMessage &msg);
    void terminate_kdeinit();
Q_SIGNALS: // SIGNALS
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();
};

#endif
