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

#ifndef KDEHW_IFACES_BUTTON_H
#define KDEHW_IFACES_BUTTON_H

#include <kdelibs_export.h>

#include <kdehw/ifaces/capability.h>

namespace KDEHW
{
namespace Ifaces
{
    /**
     * This class specifies Button capability interface and represent button
     * devices attached to the system.
     * A button is a device that can be pressed by user.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class KDE_EXPORT Button : virtual public Capability, public Enums::Button
    {
//         Q_PROPERTY( ButtonType type READ type )
//         Q_PROPERTY( bool hasState READ hasState )
//         Q_PROPERTY( bool stateValue READ stateValue )
//         Q_ENUMS( ButtonType )

    public:
        virtual ~Button();

        virtual ButtonType type() const = 0;
        virtual bool hasState() const = 0;
        virtual bool stateValue() const = 0;

    protected:
    //signals:
        virtual void pressed( int type ) = 0;
    };
}
}

#endif
