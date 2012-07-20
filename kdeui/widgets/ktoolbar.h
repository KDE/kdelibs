/* This file is part of the KDE libraries
    Copyright (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999, 2000 Kurt Granroth (granroth@kde.org)
    (C) 2005-2006 Hamish Rodda (rodda@kde.org)

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

#ifndef KTOOLBAR_H
#define KTOOLBAR_H

#include <kdeui_export.h>

#include <QToolBar>

class QDomElement;

class KConfigGroup;
class KConfig;
class KMainWindow;
class KXMLGUIClient;

 /**
  * @short Floatable toolbar with auto resize.
  *
  * A KDE-style toolbar.
  *
  * KToolBar can be used as a standalone widget, but KMainWindow
  * provides easy factories and management of one or more toolbars.
  *
  * KToolBar uses a global config group to load toolbar settings on
  * construction. It will reread this config group on a
  * KApplication::appearanceChanged() signal.
  *
  * @author Reginald Stadlbauer <reggie@kde.org>, Stephan Kulow <coolo@kde.org>, Sven Radej <radej@kde.org>, Hamish Rodda <rodda@kde.org>.
  */
class KDEUI_EXPORT KToolBar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if @p parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(name), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * @param parent      The standard toolbar parent (usually a KMainWindow)
     * @param isMainToolBar  True for the "main toolbar", false for other toolbars. Different settings apply.
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    explicit KToolBar(QWidget *parent, bool isMainToolBar = false, bool readConfig = true);
    // KDE5: remove. The one below is preferred so that all debug output from init() shows the right objectName already,
    // and so that isMainToolBar() and iconSizeDefault() return correct values during loading too.

    /**
     * Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if @p parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(name), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * @param objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position,
     *                    and so that KToolBar can find out if it's the main toolbar.
     * @param parent      The standard toolbar parent (usually a KMainWindow)
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    explicit KToolBar(const QString& objectName, QWidget* parent, bool readConfig = true);

    /**
     * Alternate constructor with additional arguments, e.g. to choose in which area
     * the toolbar should be auto-added. This is rarely used in KDE. When using XMLGUI
     * you can specify this as an xml attribute instead.
     *
     * @param objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position
     * @param parentWindow The window that should be the parent of this toolbar
     * @param area        The position of the toolbar. Usually Qt::TopToolBarArea.
     * @param newLine     If true, start a new line in the dock for this toolbar.
     * @param isMainToolBar  True for the "main toolbar", false for other toolbars. Different settings apply.
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    KToolBar(const QString& objectName, QMainWindow* parentWindow, Qt::ToolBarArea area, bool newLine = false,
             bool isMainToolBar = false, bool readConfig = true); // KDE5: remove, I don't think anyone is using this.

    /**
     * Destroys the toolbar.
     */
    virtual ~KToolBar();

    /**
     * Returns the main window that this toolbar is docked with.
     */
    KMainWindow* mainWindow() const;

    /**
     * Convenience function to set icon size
     */
    void setIconDimensions( int size );

    /**
     * Returns the default size for this type of toolbar.
     *
     * @return the default size for this type of toolbar.
     */
    int iconSizeDefault() const; // KDE5: hide from public API. Doesn't make sense to export this, and it isn't used.

    /**
     * This allows you to enable or disable the context menu.
     *
     * @param enable If false, then the context menu will be disabled
     * @deprecated use setContextMenuPolicy
     */
#ifndef KDE_NO_DEPRECATED
    KDEUI_DEPRECATED void setContextMenuEnabled( bool enable = true );
#endif

    /**
     * Returns the context menu enabled flag
     * @return true if the context menu is disabled
     * @deprecated use contextMenuPolicy
     */
#ifndef KDE_NO_DEPRECATED
    KDEUI_DEPRECATED bool contextMenuEnabled() const;
#endif

    /**
     * Save the toolbar settings to group @p configGroup in @p config.
     */
    void saveSettings( KConfigGroup &cg );

    /**
     * Read the toolbar settings from group @p configGroup in @p config
     * and apply them.
     *
     * @param forceGlobal is deprecated. In kde3 it used to mean
     * "force global settings, i.e. ignore @p cg", but only for visibility/position/index,
     * not for icon size etc. Only visibility is still controlled by this.
     */
    void applySettings( const KConfigGroup &cg, bool forceGlobal = false );

    /**
     * Sets the XML gui client.
     * @deprecated use addXMLGUIClient.
     */
#ifndef KDE_NO_DEPRECATED
    KDEUI_DEPRECATED void setXMLGUIClient( KXMLGUIClient *client );
#endif

    /**
     * Adds an XML gui client that uses this toolbar
     * @since 4.8.1
     */
    void addXMLGUIClient( KXMLGUIClient *client );

    /**
     * Removes an XML gui client that uses this toolbar
     * @since 4.8.5
     */
    void removeXMLGUIClient( KXMLGUIClient *client );

    /**
     * Load state from an XML @param element, called by KXMLGUIBuilder.
     */
    void loadState( const QDomElement &element );

    /**
     * Save state into an XML @param element, called by KXMLGUIBuilder.
     */
    void saveState( QDomElement &element ) const;

    /**
     * Reimplemented to support context menu activation on disabled tool buttons.
     */
    bool eventFilter( QObject* watched, QEvent* event );

    /**
     * Returns the global setting for "Icon Text" for the main toolbar
     * @return global setting for "Icon Text" for the main toolbar
     */
    static Qt::ToolButtonStyle toolButtonStyleSetting(); // KDE5: make private and file-static, nobody is calling this

    /**
     * Returns whether the toolbars are currently editable (drag & drop of actions).
     */
    static bool toolBarsEditable();

    /**
     * Enable or disable toolbar editing via drag & drop of actions.  This is
     * called by KEditToolbar and should generally be set to disabled whenever
     * KEditToolbar is not active.
     */
    static void setToolBarsEditable( bool editable );

    /**
     * Returns whether the toolbars are locked (i.e., moving of the toobars disallowed).
     */
    static bool toolBarsLocked();

    /**
     * Allows you to lock and unlock all toolbars (i.e., disallow/allow moving of the toobars).
     */
    static void setToolBarsLocked( bool locked );

  protected Q_SLOTS:
    virtual void slotMovableChanged(bool movable);

  protected:
    virtual void contextMenuEvent( QContextMenuEvent* );
    virtual void actionEvent( QActionEvent* );

    // Draggable toolbar configuration
    virtual void dragEnterEvent( QDragEnterEvent* );
    virtual void dragMoveEvent( QDragMoveEvent* );
    virtual void dragLeaveEvent( QDragLeaveEvent* );
    virtual void dropEvent( QDropEvent* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotAppearanceChanged() )
    Q_PRIVATE_SLOT( d, void slotContextAboutToShow() )
    Q_PRIVATE_SLOT( d, void slotContextAboutToHide() )
    Q_PRIVATE_SLOT( d, void slotContextLeft() )
    Q_PRIVATE_SLOT( d, void slotContextRight() )
    Q_PRIVATE_SLOT( d, void slotContextShowText() )
    Q_PRIVATE_SLOT( d, void slotContextTop() )
    Q_PRIVATE_SLOT( d, void slotContextBottom() )
    Q_PRIVATE_SLOT( d, void slotContextIcons() )
    Q_PRIVATE_SLOT( d, void slotContextText() )
    Q_PRIVATE_SLOT( d, void slotContextTextRight() )
    Q_PRIVATE_SLOT( d, void slotContextTextUnder() )
    Q_PRIVATE_SLOT( d, void slotContextIconSize() )
    Q_PRIVATE_SLOT( d, void slotLockToolBars( bool ) )
};

#endif
