/*
 *   Copyright (C) 2004 Lubos Lunak <l.lunak@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _KDE_TRAY_PROXY_H_
#define _KDE_TRAY_PROXY_H_

#include <qvaluelist.h>
#include <kmanagerselection.h>
#include <kwinmodule.h>
#include <qwidget.h>

class KDETrayProxy
    : public QWidget
    {
    Q_OBJECT
    public:
        KDETrayProxy();
    public slots:
        void windowAdded( WId );
        void newOwner( Window );
    protected:
        virtual bool x11Event( XEvent* );
    private:
        void dockWindow( Window w, Window owner );
        void withdrawWindow( Window w );
        static Atom makeSelectionAtom();
        KSelectionWatcher selection;
        KWinModule module;
        QValueList< Window > pending_windows;
        QValueList< Window > tray_windows;
        QMap< Window, unsigned long > docked_windows;
    };

#endif
