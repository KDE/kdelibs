/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __ko_trader_h__
#define __ko_trader_h__

#include <qstring.h>
#include <qobject.h>
#include <kservice.h>

/**
 */
class KTrader : public QObject
{
    Q_OBJECT
public:
    // A list of services
    typedef QValueList<KService::Ptr> OfferList;

    virtual ~KTrader();

    virtual OfferList query( const QString& servicetype,
			     const QString& constraint = QString::null,
			     const QString& preferences = QString::null) const;

    static KTrader* self();

protected:
    KTrader();

private:
    static KTrader* s_self;
};

#endif
