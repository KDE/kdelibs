/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DBUSBACKEND_H
#define DBUSBACKEND_H
#include <kjob.h>
#include <QDBusObjectPath>

class OrgFreedesktopSecretCollectionInterface;
class OrgFreedesktopSecretServiceInterface;
class OrgFreedesktopSecretSessionInterface;
class OrgFreedesktopSecretPromptInterface;
class QDBusPendingCallWatcher;

class OpenSessionJob : public KJob {
    Q_OBJECT
public:
    explicit OpenSessionJob( QObject *parent =0 );
    
    virtual void start();
    
    OrgFreedesktopSecretServiceInterface *serviceInterface() const;
    OrgFreedesktopSecretSessionInterface *sessionInterface() const;
    
private Q_SLOTS:
    void openSessionFinished(QDBusPendingCallWatcher*);
    
private:
    OrgFreedesktopSecretSessionInterface *sessionIf;
    OrgFreedesktopSecretServiceInterface *serviceIf;
};

/**
 * Current implementation of the client API uses the public DBus interface exposed by the
 * daemon. As such, it'll allow this library to interface with gnome-keyring if needed
 * 
 * NOTE: this class is not intended to be part of the client public API
 */
class DBusSession {
public:

    /**
     * This @return a job allowing connection to the KSecretsService deamon via dbus
     */
    static OpenSessionJob * openSession();
    
    static OrgFreedesktopSecretPromptInterface * createPrompt( const QDBusObjectPath &path );
    static OrgFreedesktopSecretCollectionInterface * createCollection( const QDBusObjectPath &path );
    
private:
    friend class OpenSessionJob;

    static const QString    encryptionAlgorithm;
    static OpenSessionJob   openSessionJob;
};

#endif // DBUSBACKEND_H
