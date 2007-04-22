/*  This file is part of the KDE project
    Copyright (C) 2006,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_IFACES_AUTHENTICATION_H
#define SOLID_IFACES_AUTHENTICATION_H

#include <QObject>
#include <QMap>
#include <QString>

#include <solid/solid_export.h>

namespace SolidExperimental
{
    class Authentication;

namespace Ifaces
{
    /**
     * Utility interface
     * Specifies a backend specific validator class to validate authentication
     * Can be used for example to authenticate user input as they type
     */
    class SOLIDIFACES_EXPORT AuthenticationValidator
    {
        public:
            virtual ~AuthenticationValidator();
            /**
             * Call this to check if an authentication is valid
             * (All secrets present, passphrase length correct
             */
            virtual bool validate( const Authentication * ) = 0;
    };
} // Ifaces
} // Solid

Q_DECLARE_INTERFACE( SolidExperimental::Ifaces::AuthenticationValidator, "org.kde.Solid.Ifaces.AuthenticationValidator/0.1" )

#endif /* SOLID_IFACES_AUTHENTICATION_H */
