/* This file is part of the KDE libraries
   Copyright (C) 2000,2006 David Faure <faure@kde.org>

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
#ifndef _KGLOBALSETTINGS_H
#define _KGLOBALSETTINGS_H

#include <kdeui_export.h>
#include <ksharedconfig.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QPalette>

#define KDE_DEFAULT_SINGLECLICK true
#define KDE_DEFAULT_SMOOTHSCROLL true
#define KDE_DEFAULT_INSERTTEAROFFHANDLES 0
#define KDE_DEFAULT_AUTOSELECTDELAY -1
#define KDE_DEFAULT_CHANGECURSOR true
#define KDE_DEFAULT_LARGE_CURSOR false
#define KDE_DEFAULT_WHEEL_ZOOM false
#define KDE_DEFAULT_ICON_ON_PUSHBUTTON true
#define KDE_DEFAULT_OPAQUE_RESIZE true
#define KDE_DEFAULT_BUTTON_LAYOUT 0
#define KDE_DEFAULT_SHADE_SORT_COLUMN true
#define KDE_DEFAULT_ALLOW_DEFAULT_BACKGROUND_IMAGES true
#define KDE_DEFAULT_NATURAL_SORTING true

class KUrl;

class QColor;
class QFont;
class QPoint;
class QRect;
class QWidget;

/**
 * Access the KDE global configuration.
 *
 * @author David Faure \<faure@kde.org\>
 */
class KDEUI_EXPORT KGlobalSettings : public QObject
{
    Q_OBJECT

public:
    ~KGlobalSettings();

    /**
     * Returns a threshold in pixels for drag & drop operations.
     * As long as the mouse movement has not exceeded this number
     * of pixels in either X or Y direction no drag operation may
     * be started. This prevents spurious drags when the user intended
     * to click on something but moved the mouse a bit while doing so.
     *
     * For this to work you must save the position of the mouse (oldPos)
     * in the QWidget::mousePressEvent().
     * When the position of the mouse (newPos)
     * in a QWidget::mouseMoveEvent() exceeds this threshold
     * you may start a drag
     * which should originate from oldPos.
     *
     * Example code:
     * \code
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
     * \endcode
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
     * Returns if item views should force smooth scrolling.
     * @return true if smooth scrolling is enabled for item view, false otherwise.
     * @since 4.2
     */
    static bool smoothScroll();

    /**
     * This enum describes the return type for insertTearOffHandle() whether to insert
     * a handle or not. Applications who independently want to use handles in their popup menus
     * should test for Application level before calling the appropriate function in KMenu.
     **/
    enum TearOffHandle {
        Disable = 0, ///< disable tear-off handles
        ApplicationLevel, ///< enable on application level
        Enable ///< enable tear-off handles
    };

    /**
     * Returns whether tear-off handles are inserted in KMenus.
     * @return whether tear-off handles are inserted in KMenus.
     **/
    static TearOffHandle insertTearOffHandle();

    /**
     * Checks whether the cursor changes over icons.
     * @return the KDE setting for "change cursor over icon"
     */
    static bool changeCursorOverIcon();

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
     * @deprecated Simply reimplement QWidget::contextMenuEvent() instead.
     */
    static KDE_DEPRECATED int contextMenuKey ();

    /**
     * Returns the KDE setting for context menus.
     *
     * @return whether context menus should be shown on button press
     * or button release (click).
     */
    static bool showContextMenusOnPress ();

    /**
     * This enum describes the completion mode used for by the KCompletion class.
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
     * @return Completion.  Default is @p CompletionPopup.
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
     * On Windows, settings are retrieved from the system.
     * @return the current mouse settings
     */
    static KMouseSettings & mouseSettings();

    /**
     * The path to the desktop directory of the current user.
     * @return the user's desktop directory
     */
    static QString desktopPath();

    /**
     * The path to the autostart directory of the current user.
     * @return the path of the autostart directory
     */
    static QString autostartPath();

    /**
     * The path where documents are stored of the current user.
     * @return the path of the document directory
     */
    static QString documentPath();

    /**
     * The path where music are stored of the current user.
     * @return the path of the music directory
     */
    static QString musicPath();

    /**
     * The path where videos are stored of the current user.
     * @return the path of the video directory
     */
    static QString videosPath();

    /**
     * The path where download are stored of the current user.
     * @return the path of the download directory
     */
    static QString downloadPath();

    /**
     * The path where pictures are stored of the current user.
     * @return the path of the pictures directory
     */
    static QString picturesPath();

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
     * Returns the contrast for borders as a floating point value.
     * @param config pointer to the config from which to read the contrast
     * setting (the default is to use KGlobal::config())
     * @return the contrast (between 0.0 for minimum and 1.0 for maximum
     *         contrast)
     */
    static qreal contrastF(const KSharedConfigPtr &config = KSharedConfigPtr());

    /**
     * Returns if the sorted column in a K3ListView shall be drawn with a
     * shaded background color.
     * @return true if the sorted column shall be shaded
     */
    static bool shadeSortColumn();

    /**
     * Returns if default background images are allowed by the color scheme.
     * A "default" background image is just that, i.e. the user has not
     * actively selected a background image to use.
     * @return true if default background images may be used
     */
    static bool allowDefaultBackgroundImages();

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
     */
    static QFont largeFont(const QString &text = QString());

    /**
     * Returns the smallest readable font. This can be used in dockers,
     * rulers and other places where space is at a premium.
     */
    static QFont smallestReadableFont();

    /**
     * Returns if the user specified multihead. In case the display
     * has multiple screens, the return value of this function specifies
     * if the user wants KDE to run on all of them or just on the primary
     * On Windows, settings are retrieved from the system.
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
     */
    static bool wheelMouseZooms();

    /**
     * This function returns the desktop geometry for an application's splash
     * screen.  It takes into account the user's display settings (number of
     * screens, Xinerama, etc), and the user's preferences (if KDE should be
     * Xinerama aware).
     *
     * @return the geometry to use for the desktop.  Note that it might not
     *         start at (0,0).
     */
    static QRect splashScreenDesktopGeometry();

    /**
     * This function returns the desktop geometry for an application that needs
     * to set the geometry of a widget on the screen manually.  It takes into
     * account the user's display settings (number of screens, Xinerama, etc),
     * and the user's preferences (if KDE should be Xinerama aware).
     *
     * Note that this can break in multi-head (not Xinerama) mode because this
     * point could be on multiple screens.  Use with care.
     *
     * @param point a reference point for the widget, for instance one that the
     *              widget should be adjacent or on top of.
     *
     * @return the geometry to use for the desktop.  Note that it might not
     *         start at (0,0).
     */
    static QRect desktopGeometry(const QPoint& point);

    /**
     * This function returns the desktop geometry for an application that needs
     * to set the geometry of a widget on the screen manually.  It takes into
     * account the user's display settings (number of screens, Xinerama, etc),
     * and the user's preferences (if KDE should be Xinerama aware).
     *
     * @param w the widget in question.  This is used to determine which screen
     *          to use in Xinerama or multi-head mode.
     *
     * @return the geometry to use for the desktop.  Note that it might not
     *         start at (0,0).
     */
    static QRect desktopGeometry(const QWidget* w);

    /**
     * This function determines if the user wishes to see icons on the
     * push buttons.
     *
     * @return Returns true if user wants to show icons.
     */
    static bool showIconsOnPushButtons();
    
    /**
     * Returns true, if user visible strings should be sorted in a natural way:
     * image 1.jpg
     * image 2.jpg
     * image 10.jpg
     * image 11.jpg
     * If false is returned, the strings are sorted by their unicode values:
     * image 1.jpg
     * image 10.jpg
     * image 11.jpg
     * image 2.jpg
     *
     * @since 4.4
     */     
    static bool naturalSorting();

    enum GraphicEffect {
        NoEffects               = 0x0000, ///< GUI with no effects at all.
        GradientEffects         = 0x0001, ///< GUI with only gradients enabled.
        SimpleAnimationEffects  = 0x0002, ///< GUI with simple animations enabled.
        ComplexAnimationEffects = 0x0006  ///< GUI with complex animations enabled.
                                          ///< Note that ComplexAnimationsEffects implies SimpleAnimationEffects.
    };

    Q_DECLARE_FLAGS(GraphicEffects, GraphicEffect)

    /**
     * This function determines the desired level of effects on the GUI.
     *
     * @since 4.1
     */
    static GraphicEffects graphicEffectsLevel();

    /**
     * This function determines the default level of effects on the GUI
     * depending on the system capabilities.
     *
     * @since 4.1
     */
    static GraphicEffects graphicEffectsLevelDefault();

    /**
     * This function determines if the user wishes to see previews
     * for the selected url
     *
     * @return Returns true if user wants to show previews.
     */
    static bool showFilePreview(const KUrl &);

    /**
     * Whether the user wishes to use opaque resizing. Primarily
     * intended for QSplitter::setOpaqueResize()
     *
     * @return Returns true if user wants to use opaque resizing.
     */
    static bool opaqueResize();

    /**
     * The layout scheme to use for dialog buttons
     *
     * @return Returns the number of the scheme to use.
     */
    static int buttonLayout();

    /**
     * Used to obtain the QPalette that will be used to set the application palette.
     *
     * This is only useful for configuration modules such as krdb and should not be
     * used in normal circumstances.
     * @param config KConfig from which to load the colors (passed as-is to
     * ::KColorScheme).
     *
     * @return the QPalette
     */
    static QPalette createApplicationPalette(const KSharedConfigPtr &config = KSharedConfigPtr());

    /**
     * An identifier for change signals.
     * \see emitChange
     */
    enum ChangeType { PaletteChanged = 0, FontChanged, StyleChanged,
                      SettingsChanged, IconChanged, CursorChanged,
                      ToolbarStyleChanged, ClipboardConfigChanged,
                      BlockShortcuts, NaturalSortingChanged };

    /**
     * Notifies all KDE applications on the current display of a change.
     *
     * This is typically called by kcontrol modules after changing the corresponding
     * config file. Do not call this from a normal KDE application.
     */
    static void emitChange(ChangeType changeType, int arg = 0);

    /**
     * Return the KGlobalSettings singleton.
     * This is used to connect to its signals, to be notified of changes.
     */
    static KGlobalSettings* self();

    /**
     * Makes all globally applicable settings take effect
     * and starts listening for changes to these settings.
     *
     * This is usually called only by the KApplication constructor.
     *
     * @since 4.3.3
     */
    void activate();

    /**
     * Valid values for the settingsChanged signal
     */
    enum SettingsCategory { SETTINGS_MOUSE, SETTINGS_COMPLETION, SETTINGS_PATHS,
                            SETTINGS_POPUPMENU, SETTINGS_QT, SETTINGS_SHORTCUTS };

Q_SIGNALS:
    /**
     * Emitted when the application has changed its palette due to a KControl request.
     *
     * Normally, widgets will update their palette automatically, but you
     * should connect to this to program special behavior.
     *
     * Note: If you derive from a QWidget-based class, a faster method is to
     *       reimplement QWidget::changeEvent() and catch QEvent::PaletteChange.
     *       This is the preferred way to get informed about palette updates.
     */
    void kdisplayPaletteChanged();

    /**
     * Emitted when the application has changed its GUI style in response to a KControl request.
     *
     * Normally, widgets will update their styles automatically (as they would
     * respond to an explicit setGUIStyle() call), but you should connect to
     * this to program special behavior.
     *
     * Note: If you derive from a QWidget-based class, a faster method is to
     *       reimplement QWidget::changeEvent() and catch QEvent::StyleChange.
     *       This is the preferred way to get informed about style updates.
     */
    void kdisplayStyleChanged();

    /**
     * Emitted when the application has changed its font in response to a KControl request.
     *
     * Normally widgets will update their fonts automatically, but you should
     * connect to this to monitor global font changes, especially if you are
     * using explicit fonts.
     *
     * Note: If you derive from a QWidget-based class, a faster method is to
     *       reimplement QWidget::changeEvent() and catch QEvent::FontChange.
     *       This is the preferred way to get informed about font updates.
     */
    void kdisplayFontChanged();

    /**
     * Emitted when the application has changed either its GUI style, its font or its palette
     * in response to a kdisplay request. Normally, widgets will update their styles
     * automatically, but you should connect to this to program special
     * behavior.
     */
    void appearanceChanged();

    /**
     * Emitted when the settings for toolbars have been changed. KToolBar will know what to do.
     */
    void toolbarAppearanceChanged(int);

    /**
     * Emitted when the global settings have been changed.
     * KGlobalSettings takes care of calling reparseConfiguration on KGlobal::config()
     * so that applications/classes using this only have to re-read the configuration
     * @param category the category among the SettingsCategory enum.
     */
    void settingsChanged(int category);

    /**
     * Emitted when the global icon settings have been changed.
     * @param group the new group
     */
    void iconChanged(int group);

    /**
     * Emitted when the cursor theme has been changed.
     */
    void cursorChanged();

    /**
     * Emitted by BlockShortcuts
     */
    void blockShortcuts(int data);
    
    /**
     * Emitted when the natural sorting has been changed.
     * @since 4.4
     */
    void naturalSortingChanged();

private:
    friend class KApplication;

    KGlobalSettings();

    class Private;
    Private* const d;

    Q_PRIVATE_SLOT(d, void _k_slotNotifyChange(int, int))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KGlobalSettings::GraphicEffects)

#endif
