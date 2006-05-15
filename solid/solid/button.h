/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#ifndef KDEHW_BUTTON_H
#define KDEHW_BUTTON_H

#include <kdelibs_export.h>

#include <kdehw/capability.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class Button;
    }

    class KDE_EXPORT Button : public Capability, public Ifaces::Enums::Button
    {
        Q_OBJECT
    public:
        Button( Ifaces::Button *iface, QObject *parent = 0 );

        virtual ~Button();

        static Type capabilityType() { return Capability::Button; }
        ButtonType type() const;
        bool hasState() const;
        bool stateValue() const;

    signals:
        void pressed( int type );

    private slots:
        void slotPressed( int type );

    private:
        class Private;
        Private *d;
    };
}

#endif
