/* This file is part of the KDE libraries
   Copyright (C) 2001 Holger Freyther <freyther@kde.org>

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

#ifndef __kstdguiitem_h
#define __kstdguiitem_h

// Needed for the template below
#include <kguiitem.h>

#include <qstring.h>
#include <qpair.h>

class KStdGuiItem
{
public:
    /**
     * The back and forward items by default use the RTL settings for Hebrew
     * and Arab countries. If you want those actions to ignore the RTL value
     * and force 'Western' behaviour instead, use the IgnoreRTL value instead.
     */
    enum BidiMode { UseRTL = 0, IgnoreRTL };
    
    enum StdItem
    {
        Ok=1, Cancel, Yes, No, Discard, Save, DontSave, SaveAs,
        Apply, Clear, Help, Defaults, Close, Back, Forward, Print,
        Continue
    };
    static KGuiItem guiItem ( StdItem ui_enum );
    static QString  stdItem ( StdItem ui_enum );
    static KGuiItem ok();
    static KGuiItem cancel();
    static KGuiItem yes();
    static KGuiItem no();
    static KGuiItem discard();
    static KGuiItem save();
    static KGuiItem help();
    static KGuiItem dontSave();
    static KGuiItem saveAs();
    static KGuiItem apply();
    static KGuiItem clear();
    static KGuiItem defaults();
    static KGuiItem close();
    static KGuiItem print();
    /**
     * It should be "continue", but that's a C/C++ keyword :-o
     */
    static KGuiItem cont();

    /**
     * Return a GUI item for a 'back' action, like Konqueror's back button.
     * This GUI item can optionally honour the user's setting for BiDi, so the
     * icon for right-to-left languages (Hebrew and Arab) has the arrow
     * pointing in the opposite direction.
     * By default the arrow points in the Western 'back' direction (i.e.
     * to the left). This is because usually you only want the Bidi aware
     * GUI item if you also want the 'forward' item. Those two are available
     * in the separate @ref backAndForward() method.
     */
    static KGuiItem back( BidiMode useBidi = IgnoreRTL );
    
    /**
     * Return a GUI item for a 'forward' action, like Konqueror's forward
     * button. This GUI item can optionally honour the user's setting for BiDi,
     * so the icon for right-to-left languages (Hebrew and Arab) has the arrow
     * pointing in the opposite direction.
     * By default the arrow points in the Western 'forward' direction (i.e.
     * to the right). This is because usually you only want the Bidi aware
     * GUI item if you also want the 'back' item. Those two are available
     * in the separate @ref backAndForward() method.
     */
    static KGuiItem forward( BidiMode useBidi = IgnoreRTL );

    /**
     * Return both a back and a forward item. This function always returns
     * items that are aware of the Right-to-Left setting for Arab and Hebrew
     * locales. If you have a reason for wanting the 'Western' back/forward
     * buttons, please use the @ref back() and @ref forward() items instead.
     */
    static QPair<KGuiItem, KGuiItem> backAndForward();
};

#endif

// vim: set et ts=4 sw=4 sts=4:

