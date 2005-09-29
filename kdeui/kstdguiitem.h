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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kstdguiitem_h
#define __kstdguiitem_h

// Needed for the template below
#include <kguiitem.h>

#include <qstring.h>
#include <qpair.h>

/**
 * @short Provides a set of standardized KGuiItems.
 *
 * The various static methods returns standardized @ref KGuiItem's
 * conforming to the KDE UI Standards. Use them instead of creating
 * your own.
 *
 * @author Holger Freyther <freyther@kde.org>
 */
class KDEUI_EXPORT KStdGuiItem
{
public:
    /**
     * The back and forward items by default use the RTL settings for Hebrew
     * and Arab countries. If you want those actions to ignore the RTL value
     * and force 'Western' behavior instead, use the IgnoreRTL value instead.
     */
    enum BidiMode { UseRTL = 0, IgnoreRTL };

    enum StdItem
    {
        Ok=1, Cancel, Yes, No, Discard, Save, DontSave, SaveAs,
        Apply, Clear, Help, Defaults, Close, Back, Forward, Print,
        Continue, Open, Quit, AdminMode, Reset, Delete, Insert,
	Configure,    ///< @since 3.4
        Find,         ///< @since 3.4
	Stop,         ///< @since 3.4
	Add,          ///< @since 3.4
	Remove,       ///< @since 3.4
	Test,         ///< @since 3.4
	Properties,   ///< @since 3.4
	Overwrite     ///< @since 3.5
    };
    static KGuiItem guiItem ( StdItem ui_enum );
    static QString  stdItem ( StdItem ui_enum );
    static KGuiItem ok();
    static KGuiItem cancel();
    static KGuiItem yes();
    static KGuiItem no();
    static KGuiItem insert();
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
     * @since 3.4
     */
    static KGuiItem properties();
    static KGuiItem reset();
    /**
     * @since 3.5
     */
    static KGuiItem overwrite();
    /**
     * Returns a KGuiItem suiting for cases where code or functionality
     * runs under root privileges. Used in conjunction with KConfig Modules.
     */
    static KGuiItem adminMode();

    /**
     * Returns a "continue" item. The short name is due to "continue" being a
     * reserved word in the C++ language.
     */
    static KGuiItem cont();

    /**
     * Returns a "delete" item. The short name is due to "delete" being a
     * reserved word in the C++ language.
     * @since 3.3
     */
    static KGuiItem del();

    static KGuiItem open();

    /**
     * Return a GUI item for a 'back' action, like Konqueror's back button.
     * This GUI item can optionally honor the user's setting for BiDi, so the
     * icon for right-to-left languages (Hebrew and Arab) has the arrow
     * pointing in the opposite direction.
     * By default the arrow points in the Western 'back' direction (i.e.
     * to the left). This is because usually you only want the Bidi aware
     * GUI item if you also want the 'forward' item. Those two are available
     * in the separate backAndForward() method.
     */
    static KGuiItem back( BidiMode useBidi = IgnoreRTL );

    /**
     * Return a GUI item for a 'forward' action, like Konqueror's forward
     * button. This GUI item can optionally honor the user's setting for BiDi,
     * so the icon for right-to-left languages (Hebrew and Arab) has the arrow
     * pointing in the opposite direction.
     * By default the arrow points in the Western 'forward' direction (i.e.
     * to the right). This is because usually you only want the Bidi aware
     * GUI item if you also want the 'back' item. Those two are available
     * in the separate backAndForward() method.
     */
    static KGuiItem forward( BidiMode useBidi = IgnoreRTL );

    /**
     * Returns a "configure" item.
     * @since 3.4
     */
    static KGuiItem configure();

    /**
     * Return both a back and a forward item. This function always returns
     * items that are aware of the Right-to-Left setting for Arab and Hebrew
     * locales. If you have a reason for wanting the 'Western' back/forward
     * buttons, please use the back() and forward() items instead.
     */
    static QPair<KGuiItem, KGuiItem> backAndForward();

    static KGuiItem quit();

    /**
    * Returns a "find" item.
    * @since 3.4
    */
    static KGuiItem find();

    /**
    * Returns a "stop" item.
    * @since 3.4
    */
    static KGuiItem stop();

    /**
    * Returns a "add" item.
    * @since 3.4
    */
    static KGuiItem add();

    /**
    * Returns a "remove" item.
    * @since 3.4
    */
    static KGuiItem remove();

    /**
    * Returns a "test" item.
    * @since 3.4
    */
    static KGuiItem test();

};

#endif

// vim: set et ts=4 sw=4 sts=4:

