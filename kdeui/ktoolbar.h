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

#include <QToolBar>
#include <QMap>
#include <QPointer>

#include <kdelibs_export.h>

class QMenu;
class QDomElement;

class KMenu;
class KXMLGUIClient;
class KConfig;
class KMainWindow;

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
   * Normal constructor.
   * This constructor is used by the XML-GUI. If you use it, you need
   * to call QMainWindow::addToolBar to specify the position of the toolbar.
   * So it's simpler to use the other constructor.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @p honorStyle is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param parent      The standard toolbar parent (usually a KMainWindow)
   * @param honorStyle  If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( QWidget *parent, bool honorStyle = false, bool readConfig = true );

  /**
   * Constructor for non-XML-GUI applications.
   *
   * The toolbar will read in various global config settings for
   * things like icon size and text position, etc.  However, some of
   * the settings will be honored only if @p honorStyle is set to
   * true.  All other toolbars will be IconOnly and use Medium icons.
   *
   * @param objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position
   * @param parentWindow The window that should be the parent of this toolbar
   * @param area        The position of the toolbar. Usually Qt::TopToolBarArea.
   * @param newLine     If true, start a new line in the dock for this toolbar.
   * @param honorStyle  If true, then global settings for IconSize and IconText will be honored
   * @param readConfig  whether to apply the configuration (global and application-specific)
   */
    KToolBar( const QString& objectName, QMainWindow* parentWindow, Qt::ToolBarArea area, bool newLine = false,
              bool honorStyle = false, bool readConfig = true );

  /**
   * Destructor
   */
    virtual ~KToolBar();

  /**
   * Returns the main window that this toolbar is docked with.
   */
    KMainWindow* mainWindow() const;

  /**
   * Convenience function to set icon size
   */
    inline void setIconDimensions(int size) { QToolBar::setIconSize(QSize(size,size)); }

  /**
   * Returns the default size for this type of toolbar.
   * @return the default size for this type of toolbar.
   */
    int iconSizeDefault() const;

  /**
   * This allows you to enable or disable the context menu.
   *
   * @param enable If false, then the context menu will be disabled
   */
    void setContextMenuEnabled(bool enable = true);

  /**
   * Returns the context menu enabled flag
   * @return true if the context menu is disabled
   */
    bool contextMenuEnabled() const;

  /**
   * Save the toolbar settings to group @p configGroup in @p config.
   */
    void saveSettings(KConfig *config, const QString &configGroup);

  /**
   * Read the toolbar settings from group @p configGroup in @p config
   * and apply them. Even default settings are re-applied if @p force is set.
   */
    void applySettings(KConfig *config, const QString &configGroup, bool force=false);

    void setXMLGUIClient( KXMLGUIClient *client );

    /**
     * Load state from an XML element, called by KXMLGUIBuilder
     */
    void loadState( const QDomElement &e );

    /**
     * Save state into an XML element, called by KXMLGUIBuilder
     */
    void saveState( QDomElement &e ) const;

    /// Reimplemented to support context menu activation on disabled tool buttons
    bool eventFilter(QObject* watched, QEvent* event);

public:
    /**
     * Returns the global setting for "Icon Text"
     * @return global setting for "Icon Text"
     */
    static Qt::ToolButtonStyle toolButtonStyleSetting();

    /**
     * Returns whether the toolbars are currently editable (drag & drop of actions).
     */
    static bool toolBarsEditable();

    /**
     * Enable or disable toolbar editing via drag & drop of actions.  This is
     * called by KEditToolbar and should generally be set to disabled whenever
     * KEditToolbar is not active.
     */
    static void setToolBarsEditable(bool editable);

    /**
     * Returns whether the toolbars are locked (i.e., moving of the toobars disallowed).
     */
    static bool toolBarsLocked();

    /**
     * Allows you to lock and unlock all toolbars (i.e., disallow/allow moving of the toobars).
     */
    static void setToolBarsLocked(bool locked);

protected Q_SLOTS:
    virtual void slotMovableChanged(bool movable);

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void actionEvent(QActionEvent* event);
    void applyAppearanceSettings(KConfig *config, const QString &_configGroup, bool forceGlobal = false);
    QString settingsGroup() const;

    // Draggable toolbar configuration
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dragLeaveEvent(QDragLeaveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private Q_SLOTS:
    void slotReadConfig();
    void slotAppearanceChanged();
    void slotContextAboutToShow();
    void slotContextAboutToHide();

    void slotContextLeft();
    void slotContextRight();
    void slotContextTop();
    void slotContextBottom();
    void slotContextIcons();
    void slotContextText();
    void slotContextTextRight();
    void slotContextTextUnder();
    void slotContextIconSize();

    void slotLockToolBars(bool lock);

private:
    void init( bool readConfig = true, bool honorStyle = false );
    void getAttributes( QString &position, Qt::ToolButtonStyle &toolButtonStyle, int &index ) const;
    int dockWindowIndex() const;
    KMenu *contextMenu();
    bool isMainToolBar() const;
    void setLocked(bool locked);
    void adjustSeparatorVisibility();

    static Qt::ToolButtonStyle toolButtonStyleFromString(const QString& style);
    static QString toolButtonStyleToString(Qt::ToolButtonStyle);

private:
    class KToolBarPrivate* const d;
};

#endif
