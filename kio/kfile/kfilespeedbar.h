/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFILESPEEDBAR_H
#define KFILESPEEDBAR_H

#include <kurlbar.h>

class KConfig;

class KFileSpeedBar : public KURLBar
{
    Q_OBJECT
public:
    KFileSpeedBar( QWidget *parent = 0, const char *name = 0 );
    ~KFileSpeedBar();

    virtual void save( KConfig *config );

private:
    bool m_initializeSpeedbar : 1;

};

#endif // KFILESPEEDBAR_H
