/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

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
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef WINOPTICALDISC_H
#define WINOPTICALDISC_H


#include <solid/ifaces/opticaldisc.h>

#include "winstoragevolume.h"


namespace Solid
{
namespace Backends
{
namespace Win
{
class WinOpticalDisc : public WinStorageVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    WinOpticalDisc(WinDevice *device);
    virtual ~WinOpticalDisc();


    virtual Solid::OpticalDisc::ContentTypes availableContent() const;

    /**
     * Retrieves the disc type (cdr, cdrw...).
     *
     * @return the disc type
     */
    virtual Solid::OpticalDisc::DiscType discType() const;

    /**
     * Indicates if it's possible to write additional data to the disc.
     *
     * @return true if the disc is appendable, false otherwise
     */
    virtual bool isAppendable() const;

    /**
     * Indicates if the disc is blank.
     *
     * @return true if the disc is blank, false otherwise
     */
    virtual bool isBlank() const;

    /**
     * Indicates if the disc is rewritable.
     *
     * A disc is rewritable if you can write on it several times.
     *
     * @return true if the disc is rewritable, false otherwise
     */
    virtual bool isRewritable() const;

    /**
     * Retrieves the disc capacity (that is the maximum size of a
     * volume could have on this disc).
     *
     * @return the capacity of the disc in bytes
     */
    virtual qulonglong capacity() const;

private:
    Solid::OpticalDisc::DiscType m_discType;
    bool m_isRewritable;
    bool m_isBlank;
    bool m_isAppendable;

};
}
}
}

#endif // WINOPTICALDISC_H
