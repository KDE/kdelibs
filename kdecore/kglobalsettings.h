/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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
#ifndef _KGLOBALSETTINGS_H
#define _KGLOBALSETTINGS_H

#include <qstring.h>
#include <qcolor.h>
#include <qfont.h>

#define KDE_DEFAULT_SINGLECLICK true
#define KDE_DEFAULT_INSERTTEAROFFHANDLES true
#define KDE_DEFAULT_AUTOSELECTDELAY -1
#define KDE_DEFAULT_CHANGECURSOR true

/**
 * Access the KDE global configuration.
 *
 * @author David Faure <faure@kde.org>
 * @version $Id$
 */
class KGlobalSettings
{
  public:

    /**
     * Returns a treshold in pixels for drag & drop operations.
     * As long as the mouse movement has not exceeded this number
     * of pixels in either X or Y direction no drag operation may
     * be started. This prevents spurious drags when the user intended
     * to click on something but moved the mouse a bit while doing so.
     *
     * For this to work you must save the position of the mouse (@p oldPos)
     * in the @ref QWidget::mousePressEvent().
     * When the position of the mouse (@p newPos)
     * in a  @ref QWidget::mouseMoveEvent() exceeds this treshold
     * you may start a drag
     * which should originate from @ref oldPos.
     *
     * Example code:
     * <pre>
     * void KColorCells::mousePressEvent( QMouseEvent *e )
     * {
     *    mOldPos = e->pos();
     * }
     *
     * void KColorCells::mouseMoveEvent( QMouseEvent *e )
     * {
     *    if( !(e->state() && LeftButton)) return;
     *
     *    int delay = KGlobalSettings::dndEventDelay();
     *    QPoint newPos = e->pos();
     *    if(newPos->x() > mOldPos.x()+delay || newPos->x() < mOldPos.x()-delay ||
     *       newPos->y() > mOldPos.y()+delay || newPos->y() < mOldPos.y()-delay)
     *    {
     *       // Drag color object
     *       int cell = posToCell(mOldPos); // Find color at mOldPos
     *       if ((cell != -1) && colors[cell].isValid())
     *       {
     *          KColorDrag *d = KColorDrag::makeDrag( colors[cell], this);
     *          d->dragCopy();
     *       }
     *    }
     * }
     * </pre>
     *
     */

    static int dndEventDelay();

    /**
     * Returns whether KDE runs in single (default) or double click
     * mode.
     *
     * @return @p true if single click mode, or @p false if double click mode.
     *
     * see @ref http://developer.kde.org/documentation/standards/kde/style/mouse/index.html
     **/
    static bool singleClick();

    /**
     * Returns whether tear-off handles are inserted in KPopupMenus.
     **/
    static bool insertTearOffHandle();

    /**
     * @return the KDE setting for "change cursor over icon"
     */
    static bool changeCursorOverIcon();

    /**
     * Returns the KDE setting for the auto-select option
     *
     * @return the auto-select delay or -1 if auto-select is disabled.
     */
    static int autoSelectDelay();

    /**
     * Returns the KDE setting for the shortcut key to open
     * context menus.
     *
     * @return the key that pops up context menus.
     */
    static int contextMenuKey ();

    /**
     * Returns the KDE setting for context menus.
     *
     * @return whether context menus should be shown on button press
     * or button release (click).
     */
    static bool showContextMenusOnPress ();

    /**
     * Retrieve the configured completion mode.
     *
     * see @ref http://developer.kde.org/documentation/standards/kde/style/keys/completion.html
     *
     * @return CompletionNone:  Completion should be disabled
     *         CompletionAuto:  Automatic completion
     *         CompletionMan:   Like automatic completion except the user initiates
     *                          the completion using the completion key as in CompletionEOL
     *         CompletionShell: Attempts to mimic the completion feature found in
     *                          typical *nix shell enviornments.
     **/

    enum Completion { CompletionNone=1, CompletionAuto, CompletionMan, CompletionShell };

    static Completion completionMode();

    /**
     * This returns whether or not KDE should use certain GNOME
     * resources.  The resources, in this case, are things like pixmap
     * directories, applnk paths, etc.
     *
     * @return Whether or not KDE should use certain GNOME resources
     */
    static bool honorGnome();

    /**
     * The path to the desktop directory of the current user.
     */
    static QString desktopPath() { initStatic(); return *s_desktopPath; }

    /**
     * The path to the autostart directory of the current user.
     */
    static QString autostartPath() { initStatic(); return *s_autostartPath; }

    /**
     * The path to the trash directory of the current user.
     */
    static QString trashPath() { initStatic(); return *s_trashPath; }


    /**
     * The default color to use when highlighting toolbar buttons
     */
    static QColor toolBarHighlightColor();
    static QColor inactiveTitleColor();
    static QColor inactiveTextColor();
    static QColor activeTitleColor();
    static QColor activeTextColor();
    static int contrast();

    static QFont generalFont();
    static QFont fixedFont();
    static QFont toolBarFont();
    static QFont menuFont();

private:
    /**
     * reads in all paths from kdeglobals
     */
    static void initStatic();
    static void rereadFontSettings();

    static QString* s_desktopPath;
    static QString* s_autostartPath;
    static QString* s_trashPath;
    static QFont *_generalFont;
    static QFont *_fixedFont;
    static QFont *_toolBarFont;
    static QFont *_menuFont;

    friend class KApplication;
};

#endif
