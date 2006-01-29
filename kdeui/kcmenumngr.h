/* This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCMENUMNGR_H
#define KCMENUMNGR_H

#include <QHash>
#include <QKeySequence>
#include <QObject>

#include <kdelibs_export.h>

class QWidget;
class QMenu;

/**
@short Convenience class to mangage context menus
@author Matthias Ettrich <ettrich@kde.org>

KContextMenuManager manages configurable context popup menus.  Instead
of reimplementing QWidget::mousePressEvent() or
 QWidget::mouseReleaseEvent() and/or QWidget::keyPressEvent(), simply
create the popup menu and insert it into the context menu manager with
the static function insert().

Example:
\code
   KContextMenuManager::insert( myWidget, myPopupMenu );
\endcode


Context menus are invoked with either a special shortcut key (usually
the menu key) or the right mouse button.

Menus are configurable in the [ContextMenus] group of the application's
configuration file, usually in kdeglobals:
\code
    [ContextMenus]
    ShowOnPress=true|false
\endcode

@p ShowOnPress defines whether the menu shall be shown on mouse
press or on mouse release.

The shortcut key to invoke the context menu is defined in the standard
[Keys] section of the application configuration:
\code
   [Keys]
   ...
   PopupContextMenu=Menu
   ...
\endcode
The key can be configured with the standard keys module in the KDE control center.

If the popup menu is invoked with the keyboard shortcut, it's shown at
the position of the micro focus hint of the widget ( QWidget::microFocusHint() ).
*/

class KDEUI_EXPORT KContextMenuManager : public QObject
{
  Q_OBJECT

  public:

    /**
       Makes @p popup a context popup menu for widget @p widget.

       Ownership of the popup menu is not transferred to the context
       menu manager.
    */
    static void insert( QWidget* widget, QMenu* popup );

    /**
     * Use this method to get information about when a popup menu
     * should be activated. This can be useful if the popup menu is
     * to be activated from a listview.
     *
     * @return true if the menu should be made visible on a button press
     *         or false after a button press-release sequence.
     */
     static bool showOnButtonPress( void );

  private Q_SLOTS:
    void widgetDestroyed();

  private:
    KContextMenuManager( QObject* parent = 0 );
    ~KContextMenuManager();

    bool eventFilter( QObject*, QEvent* );

    QHash<QObject*, QMenu*> mMenus;
    bool mShowOnPress;
    QKeySequence mMenuKey;

    static KContextMenuManager* mSelf;

    friend class I_really_like_this_class; // avoid warning

    class KContextMenuManagerPrivate;
    KContextMenuManagerPrivate *d;
};

#endif
