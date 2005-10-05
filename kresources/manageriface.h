/*
    This file is part of libkresources.

    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KRES_MANAGERIFACE_H
#define KRES_MANAGERIFACE_H

#include <dcopobject.h>

namespace KRES {

class KRESOURCES_EXPORT ManagerIface : virtual public DCOPObject
{
  K_DCOP

  k_dcop_signals:
    void signalKResourceAdded( QString managerId, QString resourceId );
    void signalKResourceModified( QString managerId, QString resourceId );
    void signalKResourceDeleted( QString managerId, QString resourceId );

  k_dcop:
    virtual ASYNC dcopKResourceAdded( QString managerId, QString resourceId ) = 0;
    virtual ASYNC dcopKResourceModified( QString managerId, QString resourceId ) = 0;
    virtual ASYNC dcopKResourceDeleted( QString managerId, QString resourceId ) = 0;
};

}

#endif
