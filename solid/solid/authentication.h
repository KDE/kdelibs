/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_AUTHENTICATION_H
#define SOLID_AUTHENTICATION_H

#include <solid/ifaces/authentication.h>

namespace Solid
{
    using Ifaces::Authentication;
    using Ifaces::AuthenticationNone;
    using Ifaces::AuthenticationWep;
    using Ifaces::AuthenticationWpa;
    using Ifaces::AuthenticationWpaPersonal;
    using Ifaces::AuthenticationWpaEnterprise;

    class AuthenticationValidator
    {
        public:
            AuthenticationValidator();
            virtual ~AuthenticationValidator();
            bool validate( const Ifaces::Authentication * );
        private:
            class Private;
            Private * d;
    };
}

#endif
