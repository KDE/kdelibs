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
#define KDE_DEFAULT_INSERTTEAROFFHANDLES 0
#define KDE_DEFAULT_AUTOSELECTDELAY -1
#define KDE_DEFAULT_CHANGECURSOR true
#define KDE_DEFAULT_LARGE_CURSOR false
#define KDE_DEFAULT_VISUAL_ACTIVATE true
#define KDE_DEFAULT_VISUAL_ACTIVATE_SPEED 50
#define KDE_DEFAULT_WHEEL_ZOOM false

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
     * Returns a threshold in pixels for drag & drop operations.
     * As long as the mouse movement has not exceeded this number
     * of pixels in either X or Y direction no drag operation may
     * be started. This prevents spurious drags when the user intended
     * to click on something but moved the mouse a bit while doing so.
     *
     * For this to work you must save the position of the mouse (oldPos)
     * in the @ref QWidget::mousePressEvent().
     * When the position of the mouse (newPos)
     * in a  @ref QWidget::mouseMoveEvent() exceeds this threshold
     * you may start a drag
     * which should originate from oldPos.
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
     *    if(newPos.x() > mOldPos.x()+delay || newPos.x() < mOldPos.x()-delay ||
     *       newPos.y() > mOldPos.y()+delay || newPos.y() < mOldPos.y()-delay)
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
     * @return the threshold for drag & drop in pixels
     */

    static int dndEventDelay();

    /**
     * Returns whether KDE runs in single (default) or double click
     * mode.
     * see http://developer.kde.org/documentation/standards/kde/style/mouse/index.html
     * @return true if single click mode, or false if double click mode.
     **/
    static bool singleClick();

    /**
     * This enum describes the return type for insertTearOffHandle() wether to insert
     * a handle or not. Applications who independently want to use handles in their popup menus
     * should test for Application level before calling the appropriate function in KPopupMenu.
     * @since 3,1
     **/
    enum TearOffHandle {
      Disable = 0, ///< disable tear-off handles
      ApplicationLevel, ///< enable on application level
      Enable ///< enable tear-off handles
    };

    /**
     * Returns whether tear-off handles are inserted in @ref KPopupMenus.
     * @return whether tear-off handles are inserted in KPopupMenus.
     * @since 3.1
     **/
    static TearOffHandle insertTearOffHandle();

    /**
     * Checks whether the cursor changes over icons.
     * @return the KDE setting for "change cursor over icon"
     */
    static bool changeCursorOverIcon();

    /**
     * Checks whether to show feedback when in item (specifically an
     * icon) is activated.
     * @return whether to show some feedback when an item is activated.
     */
    static bool visualActivate();

    /**
     * Returns the speed of the visual activation feedback.
     * @return the speed of the visual activation feedback, between
     *         0 for minimum and 100 for maximum speed
     */
    static unsigned int visualActivateSpeed();

    /**
     * Returns the KDE setting for the auto-select option.
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
     * This enum describes the completion mode used for by the @ref KCompletion class.
     * See <a href="http://developer.kde.org/documentation/standards/kde/style/keys/completion.html">
     * the styleguide</a>.
     **/
   enum Completion {
       /**
        * No completion is used.
        */
       CompletionNone=1,
       /**
        * Text is automatically filled in whenever possible.
        */
       CompletionAuto,
       /**
        * Same as automatic except shortest match is used for completion.
        */
       CompletionMan,
       /**
        * Complete text much in the same way as a typical *nix shell would.
        */
       CompletionShell,
       /**
        * Lists all possible matches in a popup list-box to choose from.
        */
       CompletionPopup,
       /**
        * Lists all possible matches in a popup list-box to choose from, and automatically
        * fill the result whenever possible.
        */
       CompletionPopupAuto
   };

    /**
     * Returns the preferred completion mode setting.
     *
     * @return @ref Completion.  Default is @p CompletionPopup.
     */
    static Completion completionMode();

    /**
     * Describes the mouse settings.
     */
    struct KMouseSettings
    {
        enum { RightHanded = 0, LeftHanded = 1 };
        int handed; // left or right
    };

    /**
     * This returns the current mouse settings.
     * @return the current mouse settings
     */
    static KMouseSettings & mouseSettings();

    /**
     * The path to the desktop directory of the current user.
     * @return the user's desktop directory
     */
    static QString desktopPath() { initStatic(); return *s_desktopPath; }

    /**
     * The path to the autostart directory of the current user.
     * @return the path of the autostart directory
     */
    static QString autostartPath() { initStatic(); return *s_autostartPath; }

    /**
     * The path to the trash directory of the current user.
     * @return the path of the trash directory
     */
    static QString trashPath() { initStatic(); return *s_trashPath; }

    /**
     * The path where documents are stored of the current user.
     * @return the path of the document directory
     */
    static QString documentPath() { initStatic(); return *s_documentPath; }


    /**
     * The default color to use when highlighting toolbar buttons.
     * @return the toolbar highlight color
     */
    static QColor toolBarHighlightColor();

    /**
     * The default color to use for inactive titles.
     * @return the inactive title color
     */
    static QColor inactiveTitleColor();

    /**
     * The default color to use for inactive texts.
     * @return the inactive text color
     */
    static QColor inactiveTextColor();

    /**
     * The default color to use for active titles.
     * @return the active title color
     */
    static QColor activeTitleColor();

    /**
     * The default color to use for active texts.
     * @return the active text color
     */
    static QColor activeTextColor();

    /**
     * Returns the contrast for borders.
     * @return the contrast (between 0 for minimum and 10 for maximum
     *         contrast)
     */
    static int contrast();

    /**
     * Returns the default base (background) color.
     * @return the default base (background) color
     * @see QColorGroup::base()
     */
    static QColor baseColor();

    /**
     * Returns the default text color.
     * @return the default text color
     * @see QColorGroup::text()
     */
    static QColor textColor();

    /**
     * Returns the default link color.
     * @return the default link color
     */
    static QColor linkColor();

    /**
     * Returns the default color for visited links.
     * @return the default color for visited links
     */
    static QColor visitedLinkColor();

    /**
     * Returns the default color for highlighted text.
     * @return the default color for highlighted text
     * @see QColorGroup::hightlightedText()
     */
    static QColor highlightedTextColor();

    /**
     * Returns the default color for text highlights.
     * @return the default color for text highlights
     * @see QColorGroup::hightlight()
     */
    static QColor highlightColor();

    /**
     * Returns the alternate background color used by @ref KListView with
     * @ref KListViewItem. Any other list that uses alternating background
     * colors should use this too, to obey to the user's preferences. Returns
     * an invalid color if the user doesn't want alternating backgrounds.
     * @return the alternate background color
     * @see #calculateAlternateBackgroundColor
     */
    static QColor alternateBackgroundColor();

    /**
     * Calculates a color based on @p base to be used as alternating
     * color for e.g. listviews.
     * @param base the base for the calculation
     * @return the calculated color
     * @see #alternateBackgroundColor
     */
    static QColor calculateAlternateBackgroundColor(const QColor& base);


    /**
     * Returns the default general font.
     * @return the default general font.
     */
    static QFont generalFont();

    /**
     * Returns the default fixed font.
     * @return the default fixed font.
     */
    static QFont fixedFont();

    /**
     * Returns the default toolbar font.
     * @return the default toolbar font.
     */
    static QFont toolBarFont();

    /**
     * Returns the default menu font.
     * @return the default menu font.
     */
    static QFont menuFont();

    /**
     * Returns the default window title font.
     * @return the default window title font.
     */
    static QFont windowTitleFont();

    /**
     * Returns the default taskbar font.
     * @return the default taskbar font.
     */
    static QFont taskbarFont();

    /**
     * Returns a font of approx. 48 pt. capable of showing @p text.
     * @param text the text to test
     * @return the font that is capable to show the text with 48 pt
     * @since 3.1
     */
    static QFont largeFont(const QString &text = QString::null);

    /**
     * Returns if the user specified multihead. In case the display
     * has multiple screens, the return value of this function specifies
     * if the user wants KDE to run on all of them or just on the primary
     * @return true if the user chose multi head
     */
    static bool isMultiHead();

    /**
     * Typically, QScrollView derived classes can be scrolled fast by
     * holding down the Ctrl-button during wheel-scrolling.
     * But QTextEdit and derived classes perform zooming instead of fast
     * scrolling.
     *
     * This value determines whether the user wants to zoom or scroll fast
     * with Ctrl-wheelscroll.
     * @return true if the user wishes to zoom with the mouse wheel,
     *         false for scrolling
     * @since 3.1
     */
    static bool wheelMouseZooms();

private:
    /**
     * reads in all paths from kdeglobals
     */
    static void initStatic();
    /**
     * initialise kde2Blue
     */
    static void initColors();
    /**
     * drop cached values for fonts (called by KApplication)
     */
    static void rereadFontSettings();
    /**
     * drop cached values for paths (called by KApplication)
     */
    static void rereadPathSettings();
    /**
     * drop cached values for mouse settings (called by KApplication)
     */
    static void rereadMouseSettings();


    static QString* s_desktopPath;
    static QString* s_autostartPath;
    static QString* s_trashPath;
    static QString* s_documentPath;
    static QFont *_generalFont;
    static QFont *_fixedFont;
    static QFont *_toolBarFont;
    static QFont *_menuFont;
    static QFont *_windowTitleFont;
    static QFont *_taskbarFont;
    static QFont *_largeFont;
    static QColor * kde2Gray;
    static QColor * kde2Blue;
    static QColor * kde2AlternateColor;
    static KMouseSettings *s_mouseSettings;

    friend class KApplication;
};

#endif
