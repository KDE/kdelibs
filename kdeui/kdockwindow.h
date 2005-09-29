/* This file is part of the KDE libraries
   Copyright (C) 1999 Matthias Ettrich <ettrich@kde.org>

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
#ifndef KDOCKWINDOW_H
#define KDOCKWINDOW_H

#include <qlabel.h>
#include <ksystemtray.h>

#ifndef KDE_NO_COMPAT
/**
 * \brief Obsolete system tray support
 *
 * @deprecated
 * This class is obsolete, it is provided for compatibility only.
 * Use KSystemTray instead.
 */
class KDE_DEPRECATED KDockWindow : public KSystemTray
{
    Q_OBJECT
public:
    KDockWindow( QWidget* parent = 0, const char* name  = 0 )
	: KSystemTray( parent, name ) {}
    ~KDockWindow() {}
};
#endif
#endif
