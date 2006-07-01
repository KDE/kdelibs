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

#ifndef kstdguiitem_h
#define kstdguiitem_h

#include <QtCore/QPair>

#include <kdelibs_export.h>

class KGuiItem;

class QString;

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
        Configure, Find, Stop, Add, Remove, Test, Properties, Overwrite
    };

    /**
     * Returns the gui item for the given identifier @param id.
     */
    static KGuiItem guiItem ( StdItem id );

    /**
     * Returns the name of the gui item for the given identifier @param id.
     */
    static QString stdItem ( StdItem id );

    /**
     * Returns the 'Ok' gui item.
     */
    static KGuiItem ok();

    /**
     * Returns the 'Cancel' gui item.
     */
    static KGuiItem cancel();

    /**
     * Returns the 'Yes' gui item.
     */
    static KGuiItem yes();

    /**
     * Returns the 'No' gui item.
     */
    static KGuiItem no();

    /**
     * Returns the 'Insert' gui item.
     */
    static KGuiItem insert();

    /**
     * Returns the 'Discard' gui item.
     */
    static KGuiItem discard();

    /**
     * Returns the 'Save' gui item.
     */
    static KGuiItem save();

    /**
     * Returns the 'Help' gui item.
     */
    static KGuiItem help();

    /**
     * Returns the 'DontSave' gui item.
     */
    static KGuiItem dontSave();

    /**
     * Returns the 'SaveAs' gui item.
     */
    static KGuiItem saveAs();

    /**
     * Returns the 'Apply' gui item.
     */
    static KGuiItem apply();

    /**
     * Returns the 'Clear' gui item.
     */
    static KGuiItem clear();

    /**
     * Returns the 'Defaults' gui item.
     */
    static KGuiItem defaults();

    /**
     * Returns the 'Close' gui item.
     */
    static KGuiItem close();

    /**
     * Returns the 'Print' gui item.
     */
    static KGuiItem print();

    /**
     * Returns the 'Properties' gui item.
     */
    static KGuiItem properties();

    /**
     * Returns the 'Reset' gui item.
     */
    static KGuiItem reset();

    /**
     * Returns the 'Overwrite' gui item.
     */
    static KGuiItem overwrite();

    /**
     * Returns a KGuiItem suiting for cases where code or functionality
     * runs under root privileges. Used in conjunction with KConfig Modules.
     */
    static KGuiItem adminMode();

    /**
     * Returns the 'Continue' gui item. The short name is due to 'continue' being a
     * reserved word in the C++ language.
     */
    static KGuiItem cont();

    /**
     * Returns the 'Delete' gui item. The short name is due to 'delete' being a
     * reserved word in the C++ language.
     */
    static KGuiItem del();

    /**
     * Returns the 'Open' gui item.
     */
    static KGuiItem open();

    /**
     * Returns the 'Back' gui item, like Konqueror's back button.
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
     * Returns the 'Forward' gui item, like Konqueror's forward
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
     * Returns the 'Configure' gui item.
     */
    static KGuiItem configure();

    /**
     * Return both a back and a forward gui item. This function always returns
     * items that are aware of the Right-to-Left setting for Arab and Hebrew
     * locales. If you have a reason for wanting the 'Western' back/forward
     * buttons, please use the back() and forward() items instead.
     */
    static QPair<KGuiItem, KGuiItem> backAndForward();

    /**
     * Returns the 'Quit' gui item.
     */
    static KGuiItem quit();

    /**
     * Returns the 'Find' gui item.
     */
    static KGuiItem find();

    /**
     * Returns the 'Stop' gui item.
     */
    static KGuiItem stop();

    /**
     * Returns the 'Add' gui item.
     */
    static KGuiItem add();

    /**
     * Returns the 'Remove' gui item.
     */
    static KGuiItem remove();

    /**
     * Returns the 'Test' gui item.
     */
    static KGuiItem test();

};

#endif
