/*  This file is part of the KDE Libraries
 *  Copyright (C) 2004 Peter Rockai (mornfall) <mornfall@danill.sk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KWHATSTHISMANAGER_P_H
#define KWHATSTHISMANAGER_P_H

#include <qapplication.h>

#include <kdialogbase.h>

class QToolButton;
class QMenu;
class Q3StyleSheet;
class Q3TextEdit;

/**
 * This class is used automatically by KMainWindow and KDialog to attach a
 * What's This templates to all new widgets. If you don't use either of those
 * classes and want this functionality, call KWhatsThisManager::init () before
 * creating any widgets.
 *
 * It works by watching for child creation events and if the new objects are
 * widgets, it attachs the whatsthis template to them (unless there already is
 * What's This defined). This template will be overriden by any custom What's
 * This info.
 *
 * @see QWhatsThis
 * @author Peter Rockai (mornfall) <mornfall@danill.sk>
 **/
class KDEUI_EXPORT KWhatsThisManager : public QObject
{
    Q_OBJECT
    public:
        static void init ();
        bool eventFilter (QObject *o, QEvent *e);
    protected:
        static KWhatsThisManager *s_instance;
    private:
        KWhatsThisManager ();
};

#endif
