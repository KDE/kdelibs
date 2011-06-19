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

class OrgFreedesktopSecretCollectionInterface;
class OrgFreedesktopSecretServiceInterface;

namespace KSecretsService {
    

/**
 * Current implementation of the client API uses the public DBus interface exposed by the
 * daemon. As such, it'll allow this library to interface with gnome-keyring if needed
 * 
 * NOTE: this class is not intended to be part of the client public API
 */
class DBusSession {
public:
    
    static OrgFreedesktopSecretServiceInterface *service();
    
protected:
    static bool startDaemon();
    
private:
    
};

}

#endif // DBUSBACKEND_H
