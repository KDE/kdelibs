/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
        */

// $Id$

#ifndef KCHECKACCELERATORS_H_
#define KCHECKACCELERATORS_H_

#include <qobject.h>
#include <qkeysequence.h>
#include <qmap.h>
#include <qstring.h>
#include <qtimer.h>

class QMenuData;

class KCheckAccelerators : public QObject
{
    Q_OBJECT
    public:
        KCheckAccelerators( QObject* parent );
        bool eventFilter( QObject * , QEvent * e);
    private:
        void checkAccelerators( bool automatic );
        struct AccelInfo {
            QString item;
	    QString string;
        };
        typedef QValueList<AccelInfo> AccelInfoList;
        typedef QMap<QChar, AccelInfoList> AccelMap;
        void findAccel( const QString& item, const QString &txt, AccelMap &accels );
        void checkMenuData( const QString& prefix, QMenuData* m );
        void checkMenuData( QMenuData* m );
        QKeySequence key;
        bool alwaysShow;
        bool autoCheck;
        bool block;
        QMap<QString, AccelMap> menuAccels;
        QTimer autoCheckTimer;
    private slots:
        void autoCheckSlot();
};

#endif
